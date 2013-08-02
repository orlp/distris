#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include <cstdio>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace asio = boost::asio;

// we keep the Connection object alive by passing smart pointers into async calls
// if there's no async calls left, or got cancelled because the socket closed then the Connection automatically cleans up
class Connection
: public boost::enable_shared_from_this<Connection>, private boost::noncopyable {
public:
    Connection(asio::io_service& io_service)
    : socket(io_service), timeout_timer(io_service), strand(io_service) {}

    void start() {
        // reduce latency
        socket.set_option(asio::ip::tcp::no_delay(true));

        set_timeout(5);

        std::cout << "Timeout set\n";

        std::string msg = "hello world\r\n";
        asio::async_write(socket, asio::buffer(msg, msg.length()), strand.wrap(boost::bind(&Connection::handle_hello_write, shared_from_this(), asio::placeholders::error)));
    }

    asio::ip::tcp::socket& get_socket() {
        return socket;
    }

private:
    void close() {
        if (!socket.is_open()) return;

        timeout_timer.cancel();
        socket.shutdown(asio::ip::tcp::socket::shutdown_both);
        socket.close();

        std::cout << "Socket closed\n";
    }

    void handle_hello_write(const boost::system::error_code& error) {
        if (error) {
            close();
        } else {
            socket.async_read_some(asio::buffer(read_buffer, 1024), strand.wrap(boost::bind(&Connection::handle_read, shared_from_this(), asio::placeholders::error, asio::placeholders::bytes_transferred)));
        }
    }

    void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred) {
        if (error) {
            close();
        } else {
            set_timeout(5);

            for (int i = 0; i < bytes_transferred; i++) {
                printf("%02X", read_buffer[i]);
            }

            std::cout << bytes_transferred << "\n";

            socket.async_read_some(asio::buffer(read_buffer, 1024), strand.wrap(boost::bind(&Connection::handle_read, shared_from_this(), asio::placeholders::error, asio::placeholders::bytes_transferred)));
        }
    }

    void set_timeout(int seconds) {
        timeout_timer.expires_from_now(boost::posix_time::seconds(seconds));
        timeout_timer.async_wait(strand.wrap(boost::bind(&Connection::handle_timeout, shared_from_this(), asio::placeholders::error)));
    }

    void handle_timeout(const boost::system::error_code& error) {
        // do not close connection if the timeout was cancelled
        if (error != asio::error::operation_aborted) {
            std::cout << "Connection timed out\n";
            close();
        }
    }

    char read_buffer[1024];

    asio::ip::tcp::socket socket;
    asio::deadline_timer timeout_timer;
    asio::io_service::strand strand;
};



class Server
: private boost::noncopyable {
public:
    Server(uint16_t port)
    : signal_monitor(io_service),
      acceptor(io_service) {
        // set up signal monitoring
        signal_monitor.add(SIGINT);
        signal_monitor.add(SIGTERM);

        #if defined(SIGQUIT)
            signal_monitor.add(SIGQUIT);
        #endif // defined(SIGQUIT)

        signal_monitor.async_wait(boost::bind(&Server::handle_signal_caught, this));

        // open acceptor and start listening for connections
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v6(), port);

        acceptor.open(endpoint.protocol());
        acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        acceptor.set_option(asio::ip::v6_only(false));
        acceptor.bind(endpoint);
        acceptor.listen();

        start_accept();
    }

    void run() {
        std::vector<boost::shared_ptr<boost::thread>> threads;

        // create 4 threads per core (just a random estimate for high throughput)
        for (int i = 0; i < (boost::thread::hardware_concurrency() * 4); ++i) {
            threads.push_back(boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&asio::io_service::run, &io_service))));
        }

        // wait for all threads in the pool to exit.
        for (int i = 0; i < threads.size(); ++i) {
            threads[i]->join();
        }
    }

private:
    void handle_signal_caught() {
        io_service.stop();
    }

    void start_accept() {
        new_connection.reset(new Connection(io_service));
        acceptor.async_accept(new_connection->get_socket(), boost::bind(&Server::handle_accept, this, boost::asio::placeholders::error));
    }

    void handle_accept(const boost::system::error_code &error) {
        if (!error) {
            new_connection->start();
        }

        start_accept();
    }

    asio::io_service io_service;
    asio::signal_set signal_monitor;
    asio::ip::tcp::acceptor acceptor;

    // this must use a shared_ptr, to allow connection objects to further keep themselves alive through boost::enable_shared_from_this
    boost::shared_ptr<Connection> new_connection;
};


int main(int argc, char **argv) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        Server server(std::atoi(argv[1]));
        server.run();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}