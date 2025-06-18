#include "types.h"
#pragma once
class OrderBook {
    public:
    OrderBook();
    void addOrder(const Order& order);

    void cancelOrder(OrderID id);

    private:
    //O(1) mapping from orderID to OrderRef, which contains the position of the ACTUAL order in the order book
    std::unordered_map<OrderID, OrderRef> orderMap;

    // Buy side: highest price first
    std::map<Price, Level, std::greater<Price>> bids;

    // Sell side: lowest price first
    std::map<Price, Level> asks;

};