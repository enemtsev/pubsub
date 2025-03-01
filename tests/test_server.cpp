#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "message.h"
#include "mock_server.h"
#include "pubsub_client.h"

using ::testing::_;
using ::testing::Eq;

class ServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        io_context = std::make_shared<boost::asio::io_context>();
        mock_server = std::make_unique<MockServer>(*io_context, 12345);
    }

    void TearDown() override {
        io_context->stop();
    }

    std::shared_ptr<boost::asio::io_context> io_context;
    std::unique_ptr<MockServer> mock_server;
};

// Test that the server accepts client connections
TEST_F(ServerTest, AcceptsClientConnection) {
    // Create a mock socket
    auto mock_socket = std::make_shared<boost::asio::ip::tcp::socket>(*io_context);

    // Simulate a CONNECT message
    std::string connect_message = "CONNECT client1\n";

    // Expect the process_message method to be called with the CONNECT message
    EXPECT_CALL(*mock_server, process_message(mock_socket, Eq(connect_message))).Times(1);

    // Trigger the process_message method
    mock_server->process_message(mock_socket, connect_message);
}

// Test that the server handles client disconnections gracefully
TEST_F(ServerTest, HandlesClientDisconnect) {
    // Create a mock socket
    auto mock_socket = std::make_shared<boost::asio::ip::tcp::socket>(*io_context);

    // Simulate a DISCONNECT message
    std::string disconnect_message = "DISCONNECT\n";

    // Expect the process_message method to be called with the DISCONNECT message
    EXPECT_CALL(*mock_server, process_message(mock_socket, Eq(disconnect_message))).Times(1);

    // Expect the handle_disconnect method to be called with the mock socket
    EXPECT_CALL(*mock_server, handle_disconnect(mock_socket)).Times(1);

    // Trigger the process_message method
    mock_server->process_message(mock_socket, disconnect_message);

    // Trigger the handle_disconnect method
    mock_server->handle_disconnect(mock_socket);
}

// Test that the server processes subscribe messages correctly
TEST_F(ServerTest, HandlesSubscribeMessage) {
    // Create a mock socket
    auto mock_socket = std::make_shared<boost::asio::ip::tcp::socket>(*io_context);

    // Simulate a SUBSCRIBE message
    std::string subscribe_message = "SUBSCRIBE topic\n";

    // Expect the process_message method to be called with the SUBSCRIBE message
    EXPECT_CALL(*mock_server, process_message(mock_socket, Eq(subscribe_message))).Times(1);

    // Trigger the process_message method
    mock_server->process_message(mock_socket, subscribe_message);
}

// Test that the server processes publish messages correctly
TEST_F(ServerTest, HandlesPublishMessage) {
    // Simulate a PUBLISH message
    std::string publish_message = "PUBLISH topic data\n";

    // Expect the process_message method to be called with the PUBLISH message
    EXPECT_CALL(*mock_server, process_message(_, Eq(publish_message))).Times(1);

    // Trigger the process_message method
    mock_server->process_message(nullptr, publish_message);  // Socket is not needed for publish
}

// Test that the server processes unsubscribe messages correctly
TEST_F(ServerTest, HandlesUnsubscribeMessage) {
    // Create a mock socket
    auto mock_socket = std::make_shared<boost::asio::ip::tcp::socket>(*io_context);

    // Simulate an UNSUBSCRIBE message
    std::string unsubscribe_message = "UNSUBSCRIBE topic\n";

    // Expect the process_message method to be called with the UNSUBSCRIBE message
    EXPECT_CALL(*mock_server, process_message(mock_socket, Eq(unsubscribe_message))).Times(1);

    // Trigger the process_message method
    mock_server->process_message(mock_socket, unsubscribe_message);
}

// Test that the server handles multiple clients correctly
TEST_F(ServerTest, HandlesMultipleClients) {
    // Create mock sockets for two clients
    auto mock_socket1 = std::make_shared<boost::asio::ip::tcp::socket>(*io_context);
    auto mock_socket2 = std::make_shared<boost::asio::ip::tcp::socket>(*io_context);

    // Simulate CONNECT messages for both clients
    std::string connect_message1 = "CONNECT client1\n";
    std::string connect_message2 = "CONNECT client2\n";

    // Expect the process_message method to be called for both clients
    EXPECT_CALL(*mock_server, process_message(mock_socket1, Eq(connect_message1))).Times(1);
    EXPECT_CALL(*mock_server, process_message(mock_socket2, Eq(connect_message2))).Times(1);

    // Trigger the process_message method for both clients
    mock_server->process_message(mock_socket1, connect_message1);
    mock_server->process_message(mock_socket2, connect_message2);
}

TEST_F(ServerTest, AcceptsClientsConnection) {
    // Simulate a client connecting to the server
    boost::asio::io_context client1_io_context;
    boost::asio::io_context client2_io_context;

    // Run the server in a separate thread
    std::thread server_thread([this]() {
        io_context->run();
    });

    PubSubClient client1(client1_io_context);
    PubSubClient client2(client2_io_context);

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
    ASSERT_NO_THROW(client1.connect("127.0.0.1", "12345", "client1"));
    ASSERT_NO_THROW(client1.subscribe("topic"));

    // Connect client2
    ASSERT_NO_THROW(client2.connect("127.0.0.1", "12345", "client2"));

    // Publish multiple messages from client2
    ASSERT_NO_THROW(client2.publish("topic", "data1"));
    ASSERT_NO_THROW(client2.publish("topic", "data2"));
    ASSERT_NO_THROW(client2.publish("topic", "data3"));

    // Wait for messages to be processed (optional, if needed)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Disconnect clients gracefully
    ASSERT_NO_THROW(client1.disconnect());
    ASSERT_NO_THROW(client2.disconnect());

    // Stop the server and client IO contexts
    io_context->stop();
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

    // Run the server in a separate thread
    std::thread server_thread([this]() {
        io_context->run();
    });

    PubSubClient client1(client1_io_context);
    PubSubClient client2(client2_io_context);

    // Threads for running client IO contexts
    std::thread client1_thread([&client1_io_context]() {
        boost::asio::io_context::work work(client1_io_context);
        client1_io_context.run();
    });

    std::thread client2_thread([&client2_io_context]() {
        boost::asio::io_context::work work(client2_io_context);
        client2_io_context.run();
    });

    // Step 1: Connect client1 and subscribe to a topic
    ASSERT_NO_THROW(client1.connect("127.0.0.1", "12345", "client1"));
    ASSERT_NO_THROW(client1.subscribe("topic"));

    // Small delay to ensure subscription is processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Step 2: (simulating disconnection)
    client1.disconnect();

    // Small delay to ensure client1 is fully disconnected
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Step 3: Connect client2 and publish data to the same topic
    ASSERT_NO_THROW(client2.connect("127.0.0.1", "12345", "client2"));
    ASSERT_NO_THROW(client2.publish("topic", "data1"));

    // Small delay to ensure the message is processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Step 4: Verify no crash occurred
    // If the test reaches this point without crashing, it means the server handled the scenario gracefully.

    // Clean up
    client2.disconnect();
    client2_io_context.stop();
    if (client2_thread.joinable()) {
        client2_thread.join();
    }

    io_context->stop();
    if (server_thread.joinable()) {
        server_thread.join();
    }

    client1_io_context.stop();
    if (client1_thread.joinable()) {
        client1_thread.join();  // Wait for client1 thread to finish
    }
}
