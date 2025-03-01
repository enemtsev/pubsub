#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>
#include <string>
#include <functional>
#include "message.h"

class Client {
public:
    Client(boost::asio::io_context &io_context);
    virtual ~Client() = default;

    void connect(const std::string& host, const std::string& port, const std::string& client_name);
    void disconnect();
    void start_reading();

    virtual void on_message_received(const std::string& message) = 0; // Pure virtual function
    virtual void write(const Message &message) = 0; // Pure virtual function

protected:
    boost::asio::executor exec_;
    boost::asio::ip::tcp::socket socket_;
    std::array<char, 1000> buffer_;
    std::string client_name_;
};

#endif // CLIENT_H
