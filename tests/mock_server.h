#ifndef MOCK_SERVER_H
#define MOCK_SERVER_H

#include "pubsub_server.h"
#include <gmock/gmock.h>

class MockServer : public PubSubServer {
public:
    MockServer(boost::asio::io_context& io_context, short port)
        : PubSubServer(io_context, port) {}

    // Mock the process_message method
    MOCK_METHOD(void, process_message, (std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string &message), (override));

    // Mock the handle_disconnect method
    MOCK_METHOD(void, handle_disconnect, (std::shared_ptr<boost::asio::ip::tcp::socket> socket), (override));
};

#endif // MOCK_SERVER_H
