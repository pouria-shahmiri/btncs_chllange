#include "Order.hpp"

Order::Order(bool is_buy, book::Price price, book::Quantity qty,
             uint64_t order_id, const std::string& timestamp)
    : is_buy_(is_buy), price_(price), qty_(qty), 
      order_id_(order_id), timestamp_(timestamp) {
}

bool Order::is_buy() const {
    return is_buy_;
}

book::Price Order::price() const {
    return price_;
}

book::Quantity Order::order_qty() const {
    return qty_;
}
