#include "websocket_reader.h"

void WebSocketReader::run() {

    // Start DNS resolution
    resolver_.async_resolve(
        host,
        port,
        boost::beast::bind_front_handler(&WebSocketReader::on_resolve, shared_from_this())
    );
}
WebSocketReader::WebSocketReader(boost::asio::io_context& io, boost::asio::ssl::context& ssl_ctx)
    : resolver_(io),
      ws_(io, ssl_ctx) {}