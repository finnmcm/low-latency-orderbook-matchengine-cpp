
#include <deque>
#include <map>
#include <unordered_map>
#pragma once



//easier to read
using Price = double;
using OrderID = uint64_t;
enum class Side: uint8_t { BUY = 0, SELL };
enum class OrderType: uint8_t { MARKET = 0, LIMIT};
struct Order {
    OrderID id;
    double price;
    int quantity;
    Side side;
    OrderType type;

};
struct Level {
    std::deque<Order> orders; 
};

using OrderIterator = std::deque<Order>::iterator;
//contains exact price, side, and position in the order book of a given order
struct OrderRef {
    Price price;
    Side side;
    OrderIterator iterator;
};
