
#include <deque>
#include <map>
#include <unordered_map>
#pragma once

//contains exact price, side, and position in the order book of a given order
struct OrderRef {
    Price price;
    Side side;
    OrderIterator iterator;
};

struct Level {
    std::deque<Order> orders; 
};

enum class Side { Buy, Sell };
struct Order {
    OrderID id;
    double price;
    int quantity;
    Side side;

};
using OrderIterator = std::deque<Order>::iterator;

//easier to read
using Price = double;
using OrderID = uint64_t;