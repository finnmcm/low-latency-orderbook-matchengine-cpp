
#include <iostream>
#include <deque>
#include <map>
#include <unordered_map>
#include "orderbook.h"
#include "types.h"
#include "tcp_reader.h"
#include <memory>
#include <thread>
#include <functional>

inline void book_thread_fn(EventQueue& q, OrderBook& b, std::atomic_bool& stop) {
    //FOR NOW, assume ALL orders are LIMIT
    while (!stop.load(std::memory_order_relaxed)) {
        Event e;
        while (q.pop(e)) { 
            switch(e.type){
                case FeedType::ADD:{
                    b.addOrder(e.order);
                    break;
                }
                case FeedType::AMEND:{
                    b.modifyOrder(e.order.id, e.order.quantity);
                    break;
                }
                case FeedType::CANCEL:{
                    b.cancelOrder(e.order.id);
                    break;
                }
            }
            b.print();
        }
       _mm_pause();  
    }

}
std::atomic<bool> quit{false};

int main(){

    boost::asio::io_context io;
    EventQueue ring = EventQueue();
    OrderBook b;
    std::string HOST = "127.0.0.1";
    u_int16_t PORT = 9000;
    auto reader = std::make_shared<TcpReader>(io, HOST, PORT, ring);
    reader->start();
    std::thread net_thread([&]{ io.run(); });

    std::thread bookThr(book_thread_fn, std::ref(ring), std::ref(b), std::ref(quit));

    boost::asio::signal_set sig(io, SIGINT, SIGTERM);
    sig.async_wait([&](const boost::system::error_code&, int) {
        quit.store(true, std::memory_order_relaxed);
        reader->stop();     // close socket
        io.stop();          // unblock io.run()
    });

    net_thread.join();
    bookThr.join();

    /*
    OrderBook b = OrderBook();
    Order newOrder1 = {123, 20, 9, Side::BUY, OrderType::LIMIT};
    Order newOrder2 = {234, 30, 10, Side::BUY, OrderType::LIMIT};
    Order newOrder3 = {345, 25, 15, Side::SELL, OrderType::LIMIT};
    Order newOrder4 = {456, 23, 12, Side::SELL, OrderType::LIMIT};
    b.addOrder(newOrder1);
    b.addOrder(newOrder2);
    //b.addOrder(newOrder3);
    //b.addOrder(newOrder4);

    b.print();
    b.addOrder(newOrder3);
    b.print();
    */
    return 0;
}