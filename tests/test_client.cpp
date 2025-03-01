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
        io_context = std::make_shared<boost::asio::io_context>();
        server = std::make_unique<PubSubServer>(*io_context, 12345);
    }

    void TearDown() override {
        io_context->stop();
    }

    std::shared_ptr<boost::asio::io_context> io_context;
    std::unique_ptr<PubSubServer> server;
};

// Test that the client can connect to the server
TEST_F(ClientTest, ConnectsToServer) {
    boost::asio::io_context client_io_context;
    MockClient client(client_io_context);

    std::thread server_thread([this]() {
        io_context->run();
    });

    std::thread client_thread([&client_io_context]() {
        boost::asio::io_context::work work(client_io_context);
        client_io_context.run();
    });

    EXPECT_CALL(client, write(_)).Times(1);  // Expect CONNECT message to be written
    ASSERT_NO_THROW(client.connect("127.0.0.1", "12345", "client1"));

    // Wait for the connection to be established
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    client.disconnect();
    io_context->stop();
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
        io_context->run();
    });

    std::thread client_thread([&client_io_context]() {
        boost::asio::io_context::work work(client_io_context);
        client_io_context.run();
    });

    EXPECT_CALL(client, write(_)).Times(2);  // Expect CONNECT and SUBSCRIBE messages to be written
    ASSERT_NO_THROW(client.connect("127.0.0.1", "12345", "client1"));

    // Wait for the connection to be established
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Subscribe to a topic
    client.subscribe("topic");

    // Wait for the subscription to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    client.disconnect();
    io_context->stop();
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
        io_context->run();
    });

    std::thread client_thread([&client_io_context]() {
        boost::asio::io_context::work work(client_io_context);
        client_io_context.run();
    });

    EXPECT_CALL(client, write(_)).Times(2);  // Expect CONNECT and PUBLISH messages to be written
    ASSERT_NO_THROW(client.connect("127.0.0.1", "12345", "client1"));

    // Wait for the connection to be established
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Publish a message
    client.publish("topic", "data");

    // Wait for the message to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    client.disconnect();
    io_context->stop();
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
        io_context->run();
    });

    std::thread client_thread([&client_io_context]() {
        boost::asio::io_context::work work(client_io_context);
        client_io_context.run();
    });

    // Expect the client to send a CONNECT message
    EXPECT_CALL(client, write(_)).Times(1);  // Expect CONNECT message to be written
    ASSERT_NO_THROW(client.connect("127.0.0.1", "12345", "client1"));

    // Wait for the connection to be established
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Simulate an incoming message from the server
    std::string test_message = "PUBLISH topic data";

    // Expect the on_message_received method to be called with the test message
    // and validate the content of the message
    EXPECT_CALL(client, on_message_received(test_message))
        .Times(1)
        .WillOnce([&test_message](const std::string &message) {
            // Validate the content of the message
            EXPECT_EQ(message, test_message);  // Ensure the message content matches
        });

    // Trigger the on_message_received method
    client.on_message_received(test_message);

    // Disconnect the client
    client.disconnect();

    // Stop the server and client IO contexts
    io_context->stop();
    client_io_context.stop();

    // Join threads to ensure they finish
    if (server_thread.joinable()) {
        server_thread.join();
    }
    if (client_thread.joinable()) {
        client_thread.join();
    }
}
