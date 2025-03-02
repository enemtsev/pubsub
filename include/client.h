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

    void connect_socket(const std::string& host, const std::string& port);
    void disconnect_socket();
    void start_reading();

    virtual void on_message_received(const std::string& topic, const std::string& message) = 0;
    virtual void write(const Message &message) = 0;

protected:
    boost::asio::executor exec_;
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    std::string client_name_;

private:
    void handle_read(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                     std::shared_ptr<std::string> client_buffer,
                     std::shared_ptr<std::array<char, 1024>> read_buffer, const boost::system::error_code &error,
                     std::size_t bytes_transferred);
};

#endif // CLIENT_H
