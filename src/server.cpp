#include "server.h"
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
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
        handle_disconnect(socket);
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
            handle_disconnect(socket);
            break;

        case MessageType::SUBSCRIBE:
            BOOST_LOG_TRIVIAL(info) << "[server] Client subscribed to topic: " << msg.topic;
            topic_manager_.subscribe(msg.topic, socket);
            client_subscriptions_[socket].insert(msg.topic);
            break;

        case MessageType::UNSUBSCRIBE:
            BOOST_LOG_TRIVIAL(info) << "[server] Client unsubscribed from topic: " << msg.topic;
            topic_manager_.unsubscribe(msg.topic, socket);
            client_subscriptions_[socket].erase(msg.topic);
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

void Server::handle_disconnect(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    // Unsubscribe the client from all topics
    auto it = client_subscriptions_.find(socket);
    if (it != client_subscriptions_.end()) {
        for (const auto &topic : it->second) {
            topic_manager_.unsubscribe(topic, socket);
        }
        client_subscriptions_.erase(it);
    }

    // Close the socket
    if (socket->is_open()) {
        boost::system::error_code ec;
        socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        if (ec) {
            BOOST_LOG_TRIVIAL(error) << "[server] Error shutting down socket: " << ec.message();
        }
        socket->close(ec);
        if (ec) {
            BOOST_LOG_TRIVIAL(error) << "[server] Error closing socket: " << ec.message();
        }
    }

    BOOST_LOG_TRIVIAL(info) << "[server] Client disconnected and unsubscribed from all topics";
}

void Server::cleanup() {
    for (auto &[socket, topics] : client_subscriptions_) {
        for (const auto &topic : topics) {
            topic_manager_.unsubscribe(topic, socket);
        }
        if (socket->is_open()) {
            boost::system::error_code ec;
            socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            if (ec) {
                BOOST_LOG_TRIVIAL(error) << "[server] Error shutting down socket: " << ec.message();
            }
            socket->close(ec);
            if (ec) {
                BOOST_LOG_TRIVIAL(error) << "[server] Error closing socket: " << ec.message();
            }
        }
    }
    client_subscriptions_.clear();
    BOOST_LOG_TRIVIAL(info) << "[server] Cleaned up all client subscriptions";
}
