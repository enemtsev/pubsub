#ifndef MOCK_CLIENT_H
#define MOCK_CLIENT_H

#include "pubsub_client.h"
#include <gmock/gmock.h>

class MockClient : public PubSubClient {
public:
    MockClient(boost::asio::io_context &io_context)
        : PubSubClient(io_context) {}

    // Mock the on_message_received method
    MOCK_METHOD(void, on_message_received, (const std::string& message), (override));

    // Mock the write method
    MOCK_METHOD(void, write, (const Message &message), (override));
};

#endif // MOCK_CLIENT_H
