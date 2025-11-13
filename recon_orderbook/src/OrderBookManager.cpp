#include "OrderBookManager.hpp"
#include <iostream>
#include <iomanip>

using namespace liquibook;

OrderBookManager::OrderBookManager() {
    // Create Liquibook depth order book
    orderbook_ = new book::DepthOrderBook<Order*>();
    std::cout << "OrderBookManager initialized" << std::endl;
}

OrderBookManager::~OrderBookManager() {
    // Clean up all orders
    for (auto& pair : order_map_) {
        delete pair.second;
    }
    order_map_.clear();
    
    delete orderbook_;
}

void OrderBookManager::processMessage(const MBOParsed& msg) {
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
            std::cerr << "Unknown action: " << msg.action << std::endl;
    }
}

void OrderBookManager::handleAdd(const MBOParsed& msg) {
    // Check for duplicate
    if (order_map_.find(msg.order_id) != order_map_.end()) {
        std::cerr << "Duplicate add for order_id: " << msg.order_id << std::endl;
        return;
    }
    
    // Create Liquibook order
    bool is_buy = convertSideToBool(msg.side);
    uint64_t price = convertPrice(msg.price);
    uint32_t qty = msg.size;
    
    Order* order = new Order(is_buy, price, qty);
    
    // Add to Liquibook with no conditions
    orderbook_->add(order, book::oc_no_conditions);
    
    // Store in map
    order_map_[msg.order_id] = order;
    
    std::cout << "ADD: order_id=" << msg.order_id 
              << " side=" << msg.side 
              << " price=" << msg.price 
              << " size=" << msg.size << std::endl;
    
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
    
    std::cout << "CANCEL: order_id=" << msg.order_id << std::endl;
    
    // Remove from map
    order_map_.erase(msg.order_id);
    
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
    
    Order* new_order = new Order(is_buy, new_price, new_qty);
    orderbook_->add(new_order, book::oc_no_conditions);
    order_map_[msg.order_id] = new_order;
    
    std::cout << "MODIFY: order_id=" << msg.order_id 
              << " new_price=" << msg.price 
              << " new_size=" << msg.size << std::endl;
}

void OrderBookManager::handleTrade(const MBOParsed& msg) {
    Order* order = findOrder(msg.order_id);
    
    if (!order) {
        return;
    }
    
    std::cout << "TRADE: order_id=" << msg.order_id 
              << " qty=" << msg.size 
              << " price=" << msg.price << std::endl;
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

void OrderBookManager::printBookState() {
    const book::DepthOrderBook<Order*>* depth_book = 
        dynamic_cast<const book::DepthOrderBook<Order*>*>(orderbook_);
    
    if (!depth_book) {
        return;
    }

    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║         ORDER BOOK (Top 5 Levels)      ║\n";
    std::cout << "╠═════════════════╦═════════════════════╣\n";
    std::cout << "║      ASKS       ║       BIDS          ║\n";
    std::cout << "║   Price   Qty   ║   Price   Qty       ║\n";
    std::cout << "╠═════════════════╬═════════════════════╣\n";

    const auto& depth = depth_book->depth();
    const book::DepthLevel* asks = depth.asks();
    const book::DepthLevel* bids = depth.bids();

    // Print 5 levels or until price is 0 (empty level)
    for (int i = 0; i < 5; ++i) {
        std::cout << "║ ";
        
        // Print ASK side (reversed, highest price first)
        const book::DepthLevel& ask = asks[4 - i];
        if (ask.price() > 0) {
            std::cout << std::fixed << std::setprecision(2)
                     << std::setw(7) << (ask.price() / 100.0)
                     << " " << std::setw(5) << ask.aggregate_qty();
        } else {
            std::cout << "             ";
        }
        
        std::cout << " ║ ";
        
        // Print BID side (highest price first)
        const book::DepthLevel& bid = bids[i];
        if (bid.price() > 0) {
            std::cout << std::fixed << std::setprecision(2)
                     << std::setw(7) << (bid.price() / 100.0)
                     << " " << std::setw(5) << bid.aggregate_qty();
        } else {
            std::cout << "             ";
        }
        
        std::cout << " ║\n";
    }

    std::cout << "╚═════════════════╩═════════════════════╝\n";
    std::cout << "  Total Orders: " << order_map_.size() << "\n\n";
}