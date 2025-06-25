#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ssl/context.hpp>
#include <json.hpp>
class WebSocketReader : public std::enable_shared_from_this<WebSocketReader> {
public:
    WebSocketReader(boost::asio::io_context&, boost::asio::ssl::context&);  
    void run(const std::string& host, const std::string& port, const std::string& target);
    void stop();
private:
    void on_resolve(...);
    void on_connect(...);
    void on_ssl_handshake(...);
    void on_handshake(...);
    void on_read(...);
    void send_subscribe();
};