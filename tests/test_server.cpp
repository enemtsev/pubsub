#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "client.h"
#include "message.h"
#include "server.h"

class ServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        io_context = std::make_shared<boost::asio::io_context>();
        server = std::make_unique<Server>(*io_context, 12345);
    }

    void TearDown() override {
        io_context->stop();
    }

    std::shared_ptr<boost::asio::io_context> io_context;
    std::unique_ptr<Server> server;
};

TEST_F(ServerTest, AcceptsClientConnection) {
    // Simulate a client connecting to the server
    boost::asio::io_context client1_io_context;
    boost::asio::io_context client2_io_context;

    // Run the server in a separate thread
    std::thread server_thread([this]() {
        io_context->run();
    });

    Client client1(client1_io_context);
    Client client2(client2_io_context);

    std::thread client1_thread([&client1_io_context]() {
        boost::asio::io_context::work work(client1_io_context);

        client1_io_context.run();
    });

    std::thread client2_thread([&client2_io_context]() {
        boost::asio::io_context::work work(client2_io_context);
        client2_io_context.run();
    });

    client1.connect("127.0.0.1", "12345", "client1");
    // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    client1.subscribe("topic");

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // Connect the client to the server
    client2.connect("127.0.0.1", "12345", "client2");
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client2.publish("topic", "data");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client2.publish("topic", "data");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client2.publish("topic", "data");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client2.publish("topic", "data1");

    client2.publish("topic", "data1");

    // Clean up
    server_thread.join();
    client1_thread.join();
    client2_thread.join();
}

TEST_F(ServerTest, HandlesPublishMessage) {
    // // Simulate a client connecting to the server
    // boost::asio::io_context client_io_context;
    // Client client(client_io_context);

    // // Run the server in a separate thread
    // std::thread server_thread([this]() { io_context->run(); });

    // // Connect the client to the server
    // client.connect("127.0.0.1", "12345");

    // // Publish a message
    // Message msg;
    // msg.type = MessageType::PUBLISH;
    // msg.topic = "test_topic";
    // msg.data = "test_data";
    // std::string serialized_msg = msg.serialize();
    // boost::asio::write(client.get_socket(), boost::asio::buffer(serialized_msg));

    // // Clean up
    // server_thread.join();
}
