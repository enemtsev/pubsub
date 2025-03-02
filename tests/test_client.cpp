#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "message.h"
#include "mock_client.h"
#include "pubsub_server.h"

using ::testing::_;
using ::testing::Return;

class ClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = std::make_unique<PubSubServer>(io_context, 12345);
    }

    void TearDown() override {
        io_context.stop();
    }

    boost::asio::io_context io_context;
    std::unique_ptr<PubSubServer> server;
};

// Test that the client sends propper connect message
TEST_F(ClientTest, ConnectsToServer) {
    boost::asio::io_context client_io_context;
    MockClient client(client_io_context);

    std::thread server_thread([this]() {
        io_context.run();
    });

    std::thread client_thread([&client_io_context]() {
        boost::asio::io_context::work work(client_io_context);
        client_io_context.run();
    });

    ASSERT_NO_THROW(client.connect_socket("127.0.0.1", "12345"));
    ASSERT_NO_THROW(client.connect("client1"));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Validate the captured messages
    const auto &messages = client.getCapturedMessages();
    ASSERT_EQ(messages.size(), 1);
    EXPECT_EQ(messages[0].type, MessageType::CONNECT);
    EXPECT_EQ(messages[0].client_name, "client1");

    client.disconnect();
    io_context.stop();
    client_io_context.stop();

    if (server_thread.joinable()) {
        server_thread.join();
    }
    if (client_thread.joinable()) {
        client_thread.join();
    }
}

// Test that the client can send a subscribe message
TEST_F(ClientTest, SendsSubscribeMessage) {
    boost::asio::io_context client_io_context;
    MockClient client(client_io_context);

    std::thread server_thread([this]() {
        io_context.run();
    });

    std::thread client_thread([&client_io_context]() {
        boost::asio::io_context::work work(client_io_context);
        client_io_context.run();
    });

    ASSERT_NO_THROW(client.connect_socket("127.0.0.1", "12345"));
    ASSERT_NO_THROW(client.connect("client1"));

    // Subscribe to a topic
    ASSERT_NO_THROW(client.subscribe("topic"));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Validate the captured messages
    const auto &messages = client.getCapturedMessages();
    ASSERT_EQ(messages.size(), 2);
    EXPECT_EQ(messages[0].type, MessageType::CONNECT);
    EXPECT_EQ(messages[0].client_name, "client1");
    EXPECT_EQ(messages[1].type, MessageType::SUBSCRIBE);
    EXPECT_EQ(messages[1].topic, "topic");

    client.disconnect();
    io_context.stop();
    client_io_context.stop();

    if (server_thread.joinable()) {
        server_thread.join();
    }
    if (client_thread.joinable()) {
        client_thread.join();
    }
}

// Test that the client can send a publish message
TEST_F(ClientTest, SendsPublishMessage) {
    boost::asio::io_context client_io_context;
    MockClient client(client_io_context);

    std::thread server_thread([this]() {
        io_context.run();
    });

    std::thread client_thread([&client_io_context]() {
        boost::asio::io_context::work work(client_io_context);
        client_io_context.run();
    });

    ASSERT_NO_THROW(client.connect_socket("127.0.0.1", "12345"));
    ASSERT_NO_THROW(client.connect("client1"));
    ASSERT_NO_THROW(client.publish("topic", "data"));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Validate the captured messages
    const auto &messages = client.getCapturedMessages();
    ASSERT_EQ(messages.size(), 2);
    EXPECT_EQ(messages[0].type, MessageType::CONNECT);
    EXPECT_EQ(messages[0].client_name, "client1");
    EXPECT_EQ(messages[1].type, MessageType::PUBLISH);
    EXPECT_EQ(messages[1].topic, "topic");
    EXPECT_EQ(messages[1].data, "data");

    client.disconnect();
    io_context.stop();
    client_io_context.stop();

    if (server_thread.joinable()) {
        server_thread.join();
    }
    if (client_thread.joinable()) {
        client_thread.join();
    }
}

// Test that the client can handle incoming messages
TEST_F(ClientTest, HandlesIncomingMessagesWithContentValidation) {
    boost::asio::io_context client_io_context;
    MockClient client(client_io_context);

    std::thread server_thread([this]() {
        io_context.run();
    });

    std::thread client_thread([&client_io_context]() {
        boost::asio::io_context::work work(client_io_context);
        client_io_context.run();
    });

    // Expect the client to send a CONNECT message
    ASSERT_NO_THROW(client.connect_socket("127.0.0.1", "12345"));
    ASSERT_NO_THROW(client.connect("client1"));
    ASSERT_NO_THROW(client.subscribe("topic"));

    // Expect the on_message_received method to be called with the test message
    // and validate the content of the message
    EXPECT_CALL(client, on_message_received(_, _))
        .Times(1)
        .WillOnce([](const std::string &topic, const std::string &message) {
            EXPECT_EQ(message, "data");
            EXPECT_EQ(topic, "topic");
        });

    ASSERT_NO_THROW(client.publish("topic", "data"));

    // wait for io to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    client.disconnect();

    // Stop the server and client IO contexts
    io_context.stop();
    client_io_context.stop();

    // Join threads to ensure they finish
    if (server_thread.joinable()) {
        server_thread.join();
    }
    if (client_thread.joinable()) {
        client_thread.join();
    }
}
