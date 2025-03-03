#ifndef MOCK_CLIENT_H
#define MOCK_CLIENT_H

#include "pubsub_client.h"
#include <gmock/gmock.h>
#include <vector>

class MockClient : public pubsub::client::PubSubClient {
public:
    MockClient(boost::asio::io_context &io_context)
        : pubsub::client::PubSubClient(io_context) {}

    MOCK_METHOD(void, on_message_received, (const std::string& topic, const std::string& message), (override));

    void write(const pubsub::Message &message) override {
        captured_messages_.push_back(message);
        pubsub::client::PubSubClient::write(message);
    }

    const std::vector<pubsub::Message>& get_captured_messages() const {
        return captured_messages_;
    }

private:
    std::vector<pubsub::Message> captured_messages_;
};

#endif // MOCK_CLIENT_H
