#ifndef MOCK_SERVER_H
#define MOCK_SERVER_H

#include "pubsub_server.h"
#include <gmock/gmock.h>
#include <vector>
#include <string>

class MockServer : public pubsub::server::PubSubServer {
public:
    MockServer(boost::asio::io_context& io_context, short port)
        : pubsub::server::PubSubServer(io_context, port) {}

    // Mock the handle_disconnect method
    MOCK_METHOD(void, handle_disconnect, (std::shared_ptr<boost::asio::ip::tcp::socket> socket), (override));

    // Add method to retrieve received messages
    const std::vector<std::string>& get_received_messages() const {
        return received_messages;
    }

private:
    std::vector<std::string> received_messages;

    // Override the process_message to store received messages
    void process_message(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string &message) override {
        received_messages.push_back(message);
        pubsub::server::PubSubServer::process_message(socket, message);
    }
};

#endif // MOCK_SERVER_H
