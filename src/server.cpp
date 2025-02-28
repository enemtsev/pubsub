#include "server.h"
#include <iostream>
#include <memory>
#include "message.h"
#include "topic_manager.h"

Server::Server(boost::asio::io_context &io_context, short port)
    : acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      topic_manager_(TopicManager::get_instance()) {
    std::cout << "Server started on port " << port << "\n";
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
        std::cout << "New client connected\n";

        // Start reading from the client
        auto client_buffer = std::make_shared<std::array<char, 1024>>();
        socket->async_read_some(
            boost::asio::buffer(*client_buffer),
            [this, socket, client_buffer](const boost::system::error_code &error, std::size_t bytes_transferred) {
                handle_read(socket, client_buffer, error, bytes_transferred);
            });

        // Accept the next client
        start_accept();
    } else {
        std::cerr << "Error accepting connection: " << error.message() << "\n";
    }
}

void Server::handle_read(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                         std::shared_ptr<std::array<char, 1024>> buffer, const boost::system::error_code &error,
                         std::size_t bytes_transferred) {
    if (!error) {
        // Parse the received message
        std::string message(buffer->data(), bytes_transferred);
        std::cout << "Received message: " << message << std::endl;

        Message msg = Message::deserialize(message);

        // Handle the message based on its type
        switch (msg.type) {
            case MessageType::CONNECT:
                std::cout << "Client connected: " << msg.client_name << "\n";
                break;

            case MessageType::DISCONNECT:
                std::cout << "Client disconnected\n";
                socket->close();
                break;

            case MessageType::SUBSCRIBE:
                std::cout << "Client subscribed to topic: " << msg.topic << "\n";
                topic_manager_.subscribe(msg.topic, socket);
                break;

            case MessageType::UNSUBSCRIBE:
                std::cout << "Client unsubscribed from topic: " << msg.topic << "\n";
                topic_manager_.unsubscribe(msg.topic, socket);
                break;

            case MessageType::PUBLISH:
                std::cout << "Publishing message to topic: " << msg.topic << "\n";
                topic_manager_.publish(msg.topic, msg.data);
                break;

            default:
                std::cerr << "Unknown message type received\n";
                break;
        }

        // Continue reading from the client
        socket->async_read_some(
            boost::asio::buffer(*buffer),
            [this, socket, buffer](const boost::system::error_code &error, std::size_t bytes_transferred) {
                handle_read(socket, buffer, error, bytes_transferred);
            });
    } else {
        std::cerr << "Error reading from client: " << error.message() << "\n";
        socket->close();
    }
}
