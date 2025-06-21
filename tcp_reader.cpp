#include "tcp_reader.h"

extern std::atomic_bool quit;

/*──────────────────────── TcpReader class ─────────────────────────────*/
    TcpReader::TcpReader(boost::asio::io_context& io,
              std::string              host,
              uint16_t                 port,
              EventQueue&                   q)
        : io_(io)
        , resolver_(io)
        , socket_(io)
        , reconnect_timer_(io)
        , host_(std::move(host))
        , port_(port)
        , out_(q)
    {}

    void TcpReader::start() { 
        resolve(); 
    }                  // entry point
    void TcpReader::stop () { 
        boost::system::error_code ec; socket_.close(ec); 
    }

    /*──────────────────── async steps ────────────────────*/
    void TcpReader::resolve() {
        resolver_.async_resolve(
            host_, std::to_string(port_),
            [self = shared_from_this()]
            (const boost::system::error_code& ec,
             const tcp::resolver::results_type& endpoints)
            {
                if (ec) {
                    std::cerr << "resolve: " << ec.message() << '\n';
                    self->schedule_reconnect();
                } else {
                    self->connect(endpoints);
                }
            });
    }

    void TcpReader::connect(const tcp::resolver::results_type& eps) {
        boost::asio::async_connect(
            socket_, eps,
            [self = shared_from_this()]
            (const boost::system::error_code& ec,
             const tcp::endpoint&)
            {
                if (ec) {
                    std::cerr << "connect: " << ec.message() << '\n';
                    self->schedule_reconnect();
                } else {
                    self->read_header();
                }
            });
    }

    void TcpReader::read_header() {
        boost::asio::async_read(
            socket_, boost::asio::buffer(hdr_buf_),
            [self = shared_from_this()]
            (const boost::system::error_code& ec, std::size_t)
            {
                if (ec) { self->handle_error(ec); return; }

                uint16_t be_len = (self->hdr_buf_[0] << 8) | self->hdr_buf_[1];
                self->body_buf_.resize(be_len);
                self->read_body(be_len);
            });
    }

    void TcpReader::read_body(std::size_t len) {
        boost::asio::async_read(
            socket_, boost::asio::buffer(body_buf_.data(), len),
            [self = shared_from_this()]
            (const boost::system::error_code& ec, std::size_t)
            {
                if (ec) { self->handle_error(ec); return; }

                Event ev;
                if (self->parseBodyToEvent(self->body_buf_.data(), self->body_buf_.size(), ev)) {
                        if (!self->out_.push(ev))
                std::cerr << "ring overflow — consumer too slow!\n";
                    }
                self->body_buf_.clear();
                self->read_header();
    });
}
    bool TcpReader::parseBodyToEvent(const void* p, std::size_t n, Event& ev){
        if (n < sizeof(WireHeader)) return false;

        auto* hdr = static_cast<const WireHeader*>(p);
        switch (hdr->type)
        {
            case FeedType::ADD: // ADD --------------------------------------------------
            {
                if (n != sizeof(WireAdd)) return false;
                auto* m  = static_cast<const WireAdd*>(p);

                ev.type            = FeedType::ADD;
                ev.order.id        = m->orderId;
                double price = static_cast<double>(m->price) / 1e4;
                if (price <= 0.0) std::cerr << "bad price " << price << '\n';
                ev.order.price = price;
                ev.order.quantity  = m->qty;           // value 0 or 1 from Python
                Side    side     = static_cast<Side>(m->side);
                ev.order.side = side;
                OrderType orderType = static_cast<OrderType>(m->orderType);
                ev.order.type = orderType;
                return true;
            }
            case FeedType::CANCEL:  // CANCEL -----------------------------------------------
            {
                if (n != sizeof(WireCancel)) return false;
                auto* m  = static_cast<const WireCancel*>(p);

                ev.type            = FeedType::CANCEL;
                ev.order.id        = m->orderId;
                return true;
            }
            case FeedType::AMEND:  // AMEND ------------------------------------------------
            {
                if (n != sizeof(WireAmend)) return false;
                auto* m  = static_cast<const WireAmend*>(p);

                ev.type            = FeedType::AMEND;
                ev.order.id        = m->orderId;
                ev.order.quantity  = m->newQty;
                return true;
            }
            default:
                return false;                   // unknown message type
    }
}

    /*──────────── error + reconnect helpers ────────────*/
    void TcpReader::handle_error(const boost::system::error_code& ec) {
        if (ec == boost::asio::error::eof) {
            std::cerr << "EOF — shutting down\n";
        } else {
            std::cerr << "read: " << ec.message() << '\n';
        }

        quit = true;                 // signal book thread to exit its loop
        socket_.close();             // closes gracefully
        io_.stop();
    }
    void TcpReader::schedule_reconnect() {
        constexpr int BACKOFF_MS = 500;
        reconnect_timer_.expires_after(std::chrono::milliseconds(BACKOFF_MS));
        reconnect_timer_.async_wait(
            [self = shared_from_this()]
            (const boost::system::error_code& /*ec*/) { self->resolve(); });
    }
/*
void book_thread_fn(Queue& q, std::atomic_bool& stop_flag) {
    while (!stop_flag) {
        Message m;
        while (q.pop(m)) {

            std::cout << "msg len = " << m.data.size() << '\n';
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <host> <port>\n";
        return 1;
    }

    boost::asio::io_context io;
    Queue ring;
    auto reader = std::make_shared<TcpReader>(io, argv[1],
                                              static_cast<uint16_t>(std::stoi(argv[2])),
                                              ring);
    reader->start();

    std::thread net_thread([&]{ io.run(); });

    std::atomic_bool quit{false};
    std::thread book_thread(book_thread_fn, std::ref(ring), std::ref(quit));

    boost::asio::signal_set sig(io, SIGINT, SIGTERM);
    sig.async_wait([&](auto, int){ quit = true; reader->stop(); io.stop(); });

    net_thread.join();
    book_thread.join();
    return 0;
}*/