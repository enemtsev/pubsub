#ifndef PUBSUB_SERVER_H
#define PUBSUB_SERVER_H

#include <boost/asio.hpp>
#include <memory>
#include <unordered_map>
#include <set>
#include <string>
#include "server.h"

namespace pubsub::server {

class PubSubServer : public Server {
public:
    explicit PubSubServer(boost::asio::io_context& io_context, short port);

protected:
    using SocketSPtr = std::shared_ptr<boost::asio::ip::tcp::socket>;
    using StringSPtr = std::shared_ptr<std::string>;
    using BufferSPtr = std::shared_ptr<std::array<char, 1024>>;

    void handle_disconnect(SocketSPtr socket) override;
    void process_message(SocketSPtr socket, const std::string &message) override;

private:
    boost::asio::ip::tcp::acceptor acceptor_;
    TopicManager topic_manager_;

    void start_accept();
    void handle_accept(SocketSPtr socket, const boost::system::error_code& error);
    void handle_read(SocketSPtr socket, StringSPtr client_buffer, BufferSPtr read_buffer,
                     const boost::system::error_code &error,
                     std::size_t bytes_transferred);
};

}  // namespace pubsub::server

#endif // PUBSUB_SERVER_H
