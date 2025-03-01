#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>
#include <string>
#include <functional>

#include "message.h"

class Client {
public:
    Client(boost::asio::io_context &io_context);
    void connect(const std::string& host, const std::string& port, const std::string& client_name);
    void disconnect();
    void publish(const std::string& topic, const std::string& data);
    void subscribe(const std::string& topic);
    void unsubscribe(const std::string& topic);
    void start_reading();

private:
    void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred);

    void write(const Message &message);

    boost::asio::executor exec_;
    boost::asio::ip::tcp::socket socket_;
    std::array<char, 1000> buffer_;
    std::string client_name_;
};

#endif // CLIENT_H
