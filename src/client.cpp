#include "client.h"
#include <iostream>
#include "message.h"

Client::Client(boost::asio::io_context &io_context)
    : socket_(io_context) {}

void Client::connect(const std::string &host, const std::string &port, const std::string &client_name) {
    boost::asio::ip::tcp::resolver resolver(socket_.get_executor());
    auto endpoints = resolver.resolve(host, port);
    boost::asio::connect(socket_, endpoints);

    // Send CONNECT message after connection
    Message msg;
    msg.type = MessageType::CONNECT;
    msg.client_name = client_name;
    std::string connect_msg = msg.serialize();
    boost::asio::write(socket_, boost::asio::buffer(connect_msg));

    // Start reading messages asynchronously
    start_reading();
}

void Client::disconnect() {
    socket_.close();
}

void Client::publish(const std::string &topic, const std::string &data) {
    Message msg;
    msg.type = MessageType::PUBLISH;
    msg.topic = topic;
    msg.data = data;
    std::string publish_msg = msg.serialize();
    boost::asio::write(socket_, boost::asio::buffer(publish_msg));
}

void Client::subscribe(const std::string &topic) {
    Message msg;
    msg.type = MessageType::SUBSCRIBE;
    msg.topic = topic;
    std::string subscribe_msg = msg.serialize();
    boost::asio::write(socket_, boost::asio::buffer(subscribe_msg));
}

void Client::unsubscribe(const std::string &topic) {
    Message msg;
    msg.type = MessageType::UNSUBSCRIBE;
    msg.topic = topic;
    std::string unsubscribe_msg = msg.serialize();
    boost::asio::write(socket_, boost::asio::buffer(unsubscribe_msg));
}

void Client::start_reading() {
    socket_.async_read_some(boost::asio::buffer(buffer_),
                            [this](const boost::system::error_code &error, std::size_t bytes_transferred) {
                                handle_read(error, bytes_transferred);
                            });
}

void Client::handle_read(const boost::system::error_code &error, std::size_t bytes_transferred) {
    if (!error) {
        // Print the received message
        std::string message(buffer_.data(), bytes_transferred);
        std::cout << message;

        // Continue reading asynchronously
        start_reading();
    } else if (error == boost::asio::error::eof) {
        std::cout << "Server disconnected\n";
    } else {
        std::cerr << "Error reading from server: " << error.message() << "\n";
    }
}

boost::asio::ip::tcp::socket &Client::get_socket() {
    return socket_;
}
