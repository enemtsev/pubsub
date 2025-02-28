#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <memory>
#include <unordered_map>
#include <set>
#include <string>
#include "topic_manager.h"

class Server {
public:
    Server(boost::asio::io_context& io_context, short port);
    void start_accept();
    void handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code& error);
    void handle_read(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                     std::shared_ptr<std::array<char, 1024>> buffer,
                     const boost::system::error_code& error,
                     std::size_t bytes_transferred);

private:
    boost::asio::ip::tcp::acceptor acceptor_;
    TopicManager& topic_manager_;
};

#endif // SERVER_H
