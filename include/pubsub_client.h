#ifndef PUBSUB_CLIENT_H
#define PUBSUB_CLIENT_H

#include <boost/asio.hpp>
#include <string>
#include <functional>
#include "message.h"
#include "client.h"

namespace pubsub::client {

class PubSubClient : public Client {
public:
    explicit PubSubClient(boost::asio::io_context &io_context);

    void connect(const std::string& client_name);
    void disconnect();
    void publish(const std::string& topic, const std::string& data);
    void subscribe(const std::string& topic);
    void unsubscribe(const std::string& topic);

    void connect_socket(const std::string& host, const std::string& port);
    void disconnect_socket();

protected:
    void on_message_received(const std::string &topic, const std::string& message) override;
    void write(const Message &message) override;

private:
    using SocketSPtr = std::shared_ptr<boost::asio::ip::tcp::socket>;
    using StringSPtr = std::shared_ptr<std::string>;
    using BufferSPtr = std::shared_ptr<std::array<char, 1024>>;

    boost::asio::executor exec_;
    SocketSPtr socket_;
    std::string client_name_;

    void handle_read(SocketSPtr socket, StringSPtr client_buffer, BufferSPtr read_buffer,
                     const boost::system::error_code &error,
                     std::size_t bytes_transferred);
    void start_reading();
};

}  // namespace pubsub::client

#endif // PUBSUB_CLIENT_H
