#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "message.h"
#include "mock_client.h"
#include "mock_server.h"

using ::testing::_;
using ::testing::Eq;

class ServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_server = std::make_unique<MockServer>(io_context, 12345);
    }

    void TearDown() override {
        io_context.stop();
    }

    boost::asio::io_context io_context;
    std::unique_ptr<MockServer> mock_server;
};

TEST_F(ServerTest, AcceptsClientsConnection) {
    // Simulate a client connecting to the server
    boost::asio::io_context client1_io_context;
    boost::asio::io_context client2_io_context;

    // Run the server in a separate thread
    std::thread server_thread([this]() {
        io_context.run();
    });

    MockClient client1(client1_io_context);
    MockClient client2(client2_io_context);

    // Threads for running client IO contexts
    std::thread client1_thread([&client1_io_context]() {
        boost::asio::io_context::work work(client1_io_context);
        client1_io_context.run();
    });

    std::thread client2_thread([&client2_io_context]() {
        boost::asio::io_context::work work(client2_io_context);
        client2_io_context.run();
    });

    // Connect client1 and subscribe to a topic
    ASSERT_NO_THROW(client1.connect_socket("127.0.0.1", "12345"));
    ASSERT_NO_THROW(client1.connect("client1"));

    ASSERT_NO_THROW(client1.subscribe("topic"));

    EXPECT_CALL(client1, on_message_received(_, _))
        .Times(3)
        .WillOnce([](const std::string &topic, const std::string &message) {
            EXPECT_EQ(message, "data1");
            EXPECT_EQ(topic, "topic");
        })
        .WillOnce([](const std::string &topic, const std::string &message) {
            EXPECT_EQ(message, "data2");
            EXPECT_EQ(topic, "topic");
        })
        .WillOnce([](const std::string &topic, const std::string &message) {
            EXPECT_EQ(message, "data3");
            EXPECT_EQ(topic, "topic");
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Connect client2
    ASSERT_NO_THROW(client2.connect_socket("127.0.0.1", "12345"));
    ASSERT_NO_THROW(client2.connect("client2"));

    // Publish multiple messages from client2
    ASSERT_NO_THROW(client2.publish("topic", "data1"));
    ASSERT_NO_THROW(client2.publish("topic", "data2"));
    ASSERT_NO_THROW(client2.publish("topic", "data3"));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Check if server received all messages
    auto received_messages = mock_server->get_received_messages();
    ASSERT_EQ(received_messages.size(), 6);
    EXPECT_EQ(received_messages[0], "CONNECT client1");
    EXPECT_EQ(received_messages[1], "SUBSCRIBE topic");
    EXPECT_EQ(received_messages[2], "CONNECT client2");
    EXPECT_EQ(received_messages[3], "PUBLISH topic data1");
    EXPECT_EQ(received_messages[4], "PUBLISH topic data2");
    EXPECT_EQ(received_messages[5], "PUBLISH topic data3");

    EXPECT_CALL(*mock_server, handle_disconnect(_)).Times(2);

    // Disconnect clients gracefully
    ASSERT_NO_THROW(client1.disconnect());
    ASSERT_NO_THROW(client2.disconnect());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Stop the server and client IO contexts
    io_context.stop();
    client1_io_context.stop();
    client2_io_context.stop();

    // Join threads to ensure they finish
    if (server_thread.joinable()) {
        server_thread.join();
    }
    if (client1_thread.joinable()) {
        client1_thread.join();
    }
    if (client2_thread.joinable()) {
        client2_thread.join();
    }
}

TEST_F(ServerTest, NoCrashWhenClientDisconnectsAndOtherClientPublishes) {
    // Simulate two clients connecting to the server
    boost::asio::io_context client1_io_context;
    boost::asio::io_context client2_io_context;

    std::thread server_thread([this]() {
        io_context.run();
    });

    PubSubClient client1(client1_io_context);
    PubSubClient client2(client2_io_context);

    std::thread client1_thread([&client1_io_context]() {
        boost::asio::io_context::work work(client1_io_context);
        client1_io_context.run();
    });

    std::thread client2_thread([&client2_io_context]() {
        boost::asio::io_context::work work(client2_io_context);
        client2_io_context.run();
    });

    // Step 1: Connect client1 and subscribe to a topic
    ASSERT_NO_THROW(client1.connect_socket("127.0.0.1", "12345"));
    ASSERT_NO_THROW(client1.connect("client1"));
    ASSERT_NO_THROW(client1.subscribe("topic"));

    // Small delay to ensure subscription is processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_CALL(*mock_server, handle_disconnect(_)).Times(1);

    // Step 2: (simulating disconnection)
    client1.disconnect();

    // Small delay to ensure client1 is fully disconnected
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Step 3: Connect client2 and publish data to the same topic
    ASSERT_NO_THROW(client2.connect_socket("127.0.0.1", "12345"));
    ASSERT_NO_THROW(client2.connect("client2"));

    ASSERT_NO_THROW(client2.publish("topic", "data1"));

    // Small delay to ensure the message is processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if server received the message
    auto received_messages = mock_server->get_received_messages();
    ASSERT_EQ(received_messages.size(), 5);
    EXPECT_EQ(received_messages[0], "CONNECT client1");
    EXPECT_EQ(received_messages[1], "SUBSCRIBE topic");
    EXPECT_EQ(received_messages[2], "DISCONNECT");
    EXPECT_EQ(received_messages[3], "CONNECT client2");
    EXPECT_EQ(received_messages[4], "PUBLISH topic data1");

    EXPECT_CALL(*mock_server, handle_disconnect(_)).Times(1);

    // Clean up
    client2.disconnect();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    client2_io_context.stop();
    if (client2_thread.joinable()) {
        client2_thread.join();
    }

    io_context.stop();
    if (server_thread.joinable()) {
        server_thread.join();
    }

    client1_io_context.stop();
    if (client1_thread.joinable()) {
        client1_thread.join();  // Wait for client1 thread to finish
    }
}
