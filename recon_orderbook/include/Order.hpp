#pragma once

#include <book/order.h>

using namespace liquibook;

class Order : public book::Order {
public:
    Order(bool is_buy, book::Price price, book::Quantity qty);
    virtual ~Order() = default;
    
    virtual bool is_buy() const override;
    virtual book::Price price() const override;
    virtual book::Quantity order_qty() const override;

private:
    bool is_buy_;
    book::Price price_;
    book::Quantity qty_;
};
