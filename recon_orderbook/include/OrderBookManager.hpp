#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <fstream>
#include "MBOParsed.hpp"
#include "Order.hpp"
#include <book/depth_order_book.h>

using namespace liquibook;

// Structure to store order metadata
struct OrderMetadata {
    uint64_t order_id;
    std::string timestamp;
    double price;
    uint32_t quantity;
};

class OrderBookManager {
private:
    // Liquibook orderbook instance
    book::DepthOrderBook<Order*>* orderbook_;
    
    // Map external order_id to Liquibook Order pointer
    std::unordered_map<uint64_t, Order*> order_map_;
    
    // Map to store order metadata (order_id -> metadata)
    std::unordered_map<uint64_t, OrderMetadata> order_metadata_;
    
    // Track pending cancels for out-of-order messages
    std::unordered_map<uint64_t, MBOParsed> pending_cancels_;
    
    // Track symbol for JSON output
    std::string current_symbol_;
    uint32_t current_sequence_;
    
    // File stream for JSON output
    std::ofstream json_file_;
    
public:
    OrderBookManager();
    ~OrderBookManager();
    
    // Main entry point for processing MBO messages
    void processMessage(const MBOParsed& msg);
    
    // Print current book state as JSON to terminal and file
    void printBookStateJSON();
    
    // Initialize JSON file output
    void initializeJSONFile(const std::string& filename);
    
private:
    // Action handlers
    void handleAdd(const MBOParsed& msg);
    void handleCancel(const MBOParsed& msg);
    void handleModify(const MBOParsed& msg);
    void handleTrade(const MBOParsed& msg);
    
    // Helper to find order
    Order* findOrder(uint64_t order_id);
    
    // Convert side character to boolean (true = buy, false = sell)
    bool convertSideToBool(char side);
    
    // Convert price to Liquibook format (scale if needed)
    uint64_t convertPrice(double price);
};