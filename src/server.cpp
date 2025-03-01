#include "server.h"
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <iostream>
#include <memory>
#include "message.h"
#include "topic_manager.h"

Server::Server(boost::asio::io_context &io_context, short port)
    : acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      topic_manager_(TopicManager::get_instance()) {
    BOOST_LOG_TRIVIAL(info) << "[server] Server started on port " << port;
    start_accept();
}

void Server::start_accept() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(acceptor_.get_executor());
    acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code &error) {
        handle_accept(socket, error);
    });
}

void Server::handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                           const boost::system::error_code &error) {
    if (!error) {
        BOOST_LOG_TRIVIAL(info) << "[server] New client connected";

        // Create a buffer for the client
        auto client_buffer = std::make_shared<std::string>();
        auto read_buffer = std::make_shared<std::array<char, 1024>>();

        // Start reading from the client
        socket->async_read_some(boost::asio::buffer(*read_buffer),
                                [this, socket, client_buffer, read_buffer](const boost::system::error_code &error,
                                                                           std::size_t bytes_transferred) {
                                    handle_read(socket, client_buffer, read_buffer, error, bytes_transferred);
                                });

        // Accept the next client
        start_accept();
    } else {
        BOOST_LOG_TRIVIAL(error) << "[server] Error accepting connection: " << error.message();
    }
}

void Server::handle_read(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                         std::shared_ptr<std::string> client_buffer,
                         std::shared_ptr<std::array<char, 1024>> read_buffer, const boost::system::error_code &error,
                         std::size_t bytes_transferred) {
    if (!error) {
        // Append the received data to the client buffer
        client_buffer->append(read_buffer->data(), bytes_transferred);

        // Check if the buffer contains a complete message (delimited by std::endl)
        std::size_t delimiter_pos = client_buffer->find('\n');
        while (delimiter_pos != std::string::npos) {
            // Extract the complete message
            std::string message = client_buffer->substr(0, delimiter_pos);
            client_buffer->erase(0, delimiter_pos + 1);  // Remove the processed message from the buffer

            // Log and process the message
            BOOST_LOG_TRIVIAL(info) << "[server] Received message: " << message;
            process_message(socket, message);

            // Look for the next message in the buffer
            delimiter_pos = client_buffer->find('\n');
        }

        // Continue reading from the client
        socket->async_read_some(boost::asio::buffer(*read_buffer),
                                [this, socket, client_buffer, read_buffer](const boost::system::error_code &error,
                                                                           std::size_t bytes_transferred) {
                                    handle_read(socket, client_buffer, read_buffer, error, bytes_transferred);
                                });
    } else {
        BOOST_LOG_TRIVIAL(error) << "[server] Error reading from client: " << error.message();
        socket->close();
    }
}

void Server::process_message(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string &message) {
    Message msg = Message::deserialize(message);

    // Handle the message based on its type
    switch (msg.type) {
        case MessageType::CONNECT:
            BOOST_LOG_TRIVIAL(info) << "[server] Client connected: " << msg.client_name;
            break;

        case MessageType::DISCONNECT:
            BOOST_LOG_TRIVIAL(info) << "[server] Client disconnected";
            socket->close();
            break;

        case MessageType::SUBSCRIBE:
            BOOST_LOG_TRIVIAL(info) << "[server] Client subscribed to topic: " << msg.topic;
            topic_manager_.subscribe(msg.topic, socket);
            break;

        case MessageType::UNSUBSCRIBE:
            BOOST_LOG_TRIVIAL(info) << "[server] Client unsubscribed from topic: " << msg.topic;
            topic_manager_.unsubscribe(msg.topic, socket);
            break;

        case MessageType::PUBLISH:
            BOOST_LOG_TRIVIAL(info) << "[server] Publishing message to topic: " << msg.topic;
            topic_manager_.publish(msg.topic, msg.data);
            break;

        default:
            BOOST_LOG_TRIVIAL(error) << "[server] Unknown message type received";
            break;
    }
}
