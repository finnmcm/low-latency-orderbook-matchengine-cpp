/*
Reference:
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
*/
#include "orderbook.h"

OrderBook::OrderBook() {

}
void OrderBook::addOrder(Order& Order){

}


