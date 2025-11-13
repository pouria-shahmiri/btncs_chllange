#include "Order.hpp"

Order::Order(bool is_buy, book::Price price, book::Quantity qty)
    : is_buy_(is_buy), price_(price), qty_(qty) {
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
