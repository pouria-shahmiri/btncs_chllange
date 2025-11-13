#include "OrderBookManager.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>

using namespace liquibook;

OrderBookManager::OrderBookManager() : current_sequence_(0) {
    // Create Liquibook depth order book
    orderbook_ = new book::DepthOrderBook<Order*>();
}

OrderBookManager::~OrderBookManager() {
    // Clean up all orders
    for (auto& pair : order_map_) {
        delete pair.second;
    }
    order_map_.clear();
    
    // Close JSON file if open
    if (json_file_.is_open()) {
        json_file_.close();
    }
    
    delete orderbook_;
}

void OrderBookManager::processMessage(const MBOParsed& msg) {
    current_symbol_ = msg.symbol;
    current_sequence_ = msg.sequence;
    
    switch (msg.action) {
        case 'A':
            handleAdd(msg);
            break;
        case 'C':
            handleCancel(msg);
            break;
        case 'M':
        case 'R':
            handleModify(msg);
            break;
        case 'T':
            handleTrade(msg);
            break;
        default:
            break;
    }
    
    // Print JSON after every message
    printBookStateJSON();
}

void OrderBookManager::handleAdd(const MBOParsed& msg) {
    // Check for duplicate
    if (order_map_.find(msg.order_id) != order_map_.end()) {
        return;
    }
    
    // Create Liquibook order with metadata
    bool is_buy = convertSideToBool(msg.side);
    uint64_t price = convertPrice(msg.price);
    uint32_t qty = msg.size;
    
    Order* order = new Order(is_buy, price, qty, msg.order_id, msg.datetime);
    
    // Add to Liquibook with no conditions
    orderbook_->add(order, book::oc_no_conditions);
    
    // Store in map
    order_map_[msg.order_id] = order;
    
    // Store metadata
    order_metadata_[msg.order_id] = {
        msg.order_id,
        msg.datetime,
        msg.price,
        msg.size
    };
    
    // Check if there was a pending cancel
    auto pending = pending_cancels_.find(msg.order_id);
    if (pending != pending_cancels_.end()) {
        handleCancel(pending->second);
        pending_cancels_.erase(pending);
    }
}

void OrderBookManager::handleCancel(const MBOParsed& msg) {
    Order* order = findOrder(msg.order_id);
    
    if (!order) {
        // Order not found - might be out of order message
        pending_cancels_[msg.order_id] = msg;
        return;
    }
    
    // Cancel in Liquibook
    orderbook_->cancel(order);
    
    // Remove from maps
    order_map_.erase(msg.order_id);
    order_metadata_.erase(msg.order_id);
    
    // Clean up memory
    delete order;
}

void OrderBookManager::handleModify(const MBOParsed& msg) {
    Order* order = findOrder(msg.order_id);
    
    if (!order) {
        return;
    }
    
    // Liquibook doesn't have native modify - do cancel + add
    bool is_buy = order->is_buy();
    
    // Cancel old order
    orderbook_->cancel(order);
    order_map_.erase(msg.order_id);
    delete order;
    
    // Add new order with modified parameters
    uint64_t new_price = convertPrice(msg.price);
    uint32_t new_qty = msg.size;
    
    Order* new_order = new Order(is_buy, new_price, new_qty, msg.order_id, msg.datetime);
    orderbook_->add(new_order, book::oc_no_conditions);
    order_map_[msg.order_id] = new_order;
    
    // Update metadata
    order_metadata_[msg.order_id] = {
        msg.order_id,
        msg.datetime,
        msg.price,
        msg.size
    };
}

void OrderBookManager::handleTrade(const MBOParsed& msg) {
    Order* order = findOrder(msg.order_id);
    
    if (!order) {
        return;
    }
}

Order* OrderBookManager::findOrder(uint64_t order_id) {
    auto it = order_map_.find(order_id);
    if (it != order_map_.end()) {
        return it->second;
    }
    return nullptr;
}

bool OrderBookManager::convertSideToBool(char side) {
    return (side == 'B' || side == 'b');
}

uint64_t OrderBookManager::convertPrice(double price) {
    // Scale price to avoid floating point (e.g., $123.45 -> 12345)
    // Assuming 2 decimal places for stocks
    return static_cast<uint64_t>(price * 100.0);
}

void OrderBookManager::initializeJSONFile(const std::string& filename) {
    if (json_file_.is_open()) {
        json_file_.close();
    }
    json_file_.open(filename, std::ios::out | std::ios::trunc);
    if (!json_file_.is_open()) {
        std::cerr << "Error opening JSON file: " << filename << std::endl;
    }
}

void OrderBookManager::printBookStateJSON() {
    const book::DepthOrderBook<Order*>* depth_book = 
        dynamic_cast<const book::DepthOrderBook<Order*>*>(orderbook_);
    
    if (!depth_book) {
        return;
    }

    std::stringstream json_output;
    json_output << "{\n";
    json_output << "  \"symbol\": \"" << current_symbol_ << "\",\n";
    json_output << "  \"sequence\": " << current_sequence_ << ",\n";
    json_output << "  \"bids\": [\n";
    
    // Collect all buy orders with their full metadata
    // Structure: price -> vector of (order_id, timestamp, quantity)
    std::map<double, std::vector<std::tuple<uint64_t, std::string, uint32_t>>, std::greater<double>> buy_by_price;
    
    for (const auto& pair : order_map_) {
        Order* order = pair.second;
        if (order && order->is_buy()) {
            double price = order->price() / 100.0;
            uint64_t order_id = order->get_order_id();
            const std::string& timestamp = order->get_timestamp();
            uint32_t qty = order->order_qty();
            
            // Validate price
            if (order->price() > 0 && price < 1000000.0) {
                buy_by_price[price].push_back({order_id, timestamp, qty});
            }
        }
    }
    
    // Print buy orders, grouped by price (descending - highest first)
    size_t price_idx = 0;
    for (const auto& price_group : buy_by_price) {
        double price = price_group.first;
        const auto& orders_at_price = price_group.second;
        
        // Print each individual order at this price level
        for (size_t order_idx = 0; order_idx < orders_at_price.size(); ++order_idx) {
            uint64_t order_id = std::get<0>(orders_at_price[order_idx]);
            const std::string& timestamp = std::get<1>(orders_at_price[order_idx]);
            uint32_t qty = std::get<2>(orders_at_price[order_idx]);
            
            json_output << "    {\n";
            json_output << "      \"order_id\": " << order_id << ",\n";
            json_output << "      \"timestamp\": \"" << timestamp << "\",\n";
            json_output << "      \"price\": " << std::fixed << std::setprecision(2) << price << ",\n";
            json_output << "      \"quantity\": " << qty << "\n";
            json_output << "    }";
            
            // Add comma if not the last order overall
            bool is_last_order = (price_idx == buy_by_price.size() - 1) && 
                                 (order_idx == orders_at_price.size() - 1);
            if (!is_last_order) {
                json_output << ",";
            }
            json_output << "\n";
        }
        price_idx++;
    }
    
    json_output << "  ],\n";
    json_output << "  \"asks\": [\n";
    
    // Collect all sell orders with their full metadata
    // Structure: price -> vector of (order_id, timestamp, quantity)
    std::map<double, std::vector<std::tuple<uint64_t, std::string, uint32_t>>, std::less<double>> sell_by_price;
    
    for (const auto& pair : order_map_) {
        Order* order = pair.second;
        if (order && !order->is_buy()) {
            double price = order->price() / 100.0;
            uint64_t order_id = order->get_order_id();
            const std::string& timestamp = order->get_timestamp();
            uint32_t qty = order->order_qty();
            
            // Validate price
            if (order->price() > 0 && price < 1000000.0) {
                sell_by_price[price].push_back({order_id, timestamp, qty});
            }
        }
    }
    
    // Print sell orders, grouped by price (ascending - lowest first)
    price_idx = 0;
    for (const auto& price_group : sell_by_price) {
        double price = price_group.first;
        const auto& orders_at_price = price_group.second;
        
        // Print each individual order at this price level
        for (size_t order_idx = 0; order_idx < orders_at_price.size(); ++order_idx) {
            uint64_t order_id = std::get<0>(orders_at_price[order_idx]);
            const std::string& timestamp = std::get<1>(orders_at_price[order_idx]);
            uint32_t qty = std::get<2>(orders_at_price[order_idx]);
            
            json_output << "    {\n";
            json_output << "      \"order_id\": " << order_id << ",\n";
            json_output << "      \"timestamp\": \"" << timestamp << "\",\n";
            json_output << "      \"price\": " << std::fixed << std::setprecision(2) << price << ",\n";
            json_output << "      \"quantity\": " << qty << "\n";
            json_output << "    }";
            
            // Add comma if not the last order overall
            bool is_last_order = (price_idx == sell_by_price.size() - 1) && 
                                 (order_idx == orders_at_price.size() - 1);
            if (!is_last_order) {
                json_output << ",";
            }
            json_output << "\n";
        }
        price_idx++;
    }
    
    json_output << "  ]\n";
    json_output << "}\n";
    
    // Print to console
    std::cout << json_output.str();
    
    // Write to file if open
    if (json_file_.is_open()) {
        json_file_ << json_output.str();
        json_file_.flush();
    }
}