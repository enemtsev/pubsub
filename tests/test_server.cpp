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
    boost::asio::io_context client_io_context;
    Client client(client_io_context);

    // Run the server in a separate thread
    std::thread server_thread([this]() {
        io_context->run();
    });

    // Connect the client to the server
    EXPECT_NO_THROW(client.connect("127.0.0.1", "12345", "name"));

    // Clean up
    server_thread.join();
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
