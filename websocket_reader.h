#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ssl/context.hpp>
#include <json.hpp>
#include <iostream>
#include <string>
#include <boost/beast/ssl.hpp>
#pragma once
class WebSocketReader : public std::enable_shared_from_this<WebSocketReader> {
public:
    WebSocketReader(boost::asio::io_context&, boost::asio::ssl::context&);  
    void run();
    void stop();
private:
    std::string port = "443"; //coinbase port
    std::string host = "ws-feed.exchange.coinbase.com";
    std::string target = "/"; //root path

    //weird websocket stuff
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<
    boost::beast::ssl_stream<boost::asio::ip::tcp::socket>
> ws_;

    void on_resolve(...);
    void on_connect(...);
    void on_ssl_handshake(...);
    void on_handshake(...);
    void on_read(...);
    void send_subscribe();
};