#ifndef MOCK_CLIENT_H
#define MOCK_CLIENT_H

#include "pubsub_client.h"
#include <gmock/gmock.h>
#include <vector>

class MockClient : public PubSubClient {
public:
    MockClient(boost::asio::io_context &io_context)
        : PubSubClient(io_context) {}

    MOCK_METHOD(void, on_message_received, (const std::string& topic, const std::string& message), (override));

    void write(const Message &message) override {
        captured_messages_.push_back(message);
        PubSubClient::write(message);
    }

    const std::vector<Message>& get_captured_messages() const {
        return captured_messages_;
    }

private:
    std::vector<Message> captured_messages_;
};

#endif // MOCK_CLIENT_H
