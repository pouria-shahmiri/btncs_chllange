#pragma once

#include <book/order.h>
#include <string>
#include <cstdint>

using namespace liquibook;

class Order : public book::Order {
public:
    Order(bool is_buy, book::Price price, book::Quantity qty, 
          uint64_t order_id = 0, const std::string& timestamp = "");
    virtual ~Order() = default;
    
    virtual bool is_buy() const override;
    virtual book::Price price() const override;
    virtual book::Quantity order_qty() const override;
    
    // Getters for order metadata
    uint64_t get_order_id() const { return order_id_; }
    const std::string& get_timestamp() const { return timestamp_; }

private:
    bool is_buy_;
    book::Price price_;
    book::Quantity qty_;
    uint64_t order_id_;
    std::string timestamp_;
};
