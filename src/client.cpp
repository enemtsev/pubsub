#include "client.h"
#include <iostream>

Client::Client(boost::asio::io_context &io_context)
    : exec_{io_context.get_executor()},
      socket_(io_context) {}

void Client::connect(const std::string &host, const std::string &port, const std::string &client_name) {
    std::cout << "post connect\n";
    boost::asio::post(exec_, [this, host, port, client_name]() {
        std::cout << "do connect\n";
        boost::asio::ip::tcp::resolver resolver(socket_.get_executor());
        auto endpoints = resolver.resolve(host, port);
        boost::system::error_code error;
        boost::asio::connect(socket_, endpoints, error);

        // Send CONNECT message after connection
        Message msg;
        msg.type = MessageType::CONNECT;
        msg.client_name = client_name;
        write(msg);

        // Start reading messages asynchronously
        start_reading();
    });
}

void Client::write(const Message &message) {
    boost::system::error_code error;
    boost::asio::write(socket_, boost::asio::buffer(message.serialize()), error);

    if (!error) {
        // ok
    } else if (error == boost::asio::error::eof) {
        std::cout << "write failed disconnected\n";
    } else {
        std::cerr << "Error writing to server: " << error.message() << "\n";
    }
}

void Client::disconnect() {
    socket_.close();
}

void Client::publish(const std::string &topic, const std::string &data) {
    boost::asio::post(exec_, [this, topic, data]() {
        Message msg;
        msg.type = MessageType::PUBLISH;
        msg.topic = topic;
        msg.data = data;
        write(msg);
    });
}

void Client::subscribe(const std::string &topic) {
    boost::asio::post(exec_, [this, topic]() {
        std::cout << "subscribe topic: " << topic << std::endl;
        Message msg;
        msg.type = MessageType::SUBSCRIBE;
        msg.topic = topic;
        write(msg);
    });
}

void Client::unsubscribe(const std::string &topic) {
    boost::asio::post(exec_, [this, topic]() {
        Message msg;
        msg.type = MessageType::UNSUBSCRIBE;
        msg.topic = topic;
        write(msg);
    });
}

void Client::start_reading() {
    socket_.async_read_some(boost::asio::buffer(buffer_),
                            [this](const boost::system::error_code &error, std::size_t bytes_transferred) {
                                handle_read(error, bytes_transferred);
                            });
}

void Client::handle_read(const boost::system::error_code &error, std::size_t bytes_transferred) {
    if (!error) {
        std::cout << "got data!" << bytes_transferred << std::endl;
        // Print the received message
        std::string message(buffer_.data(), bytes_transferred);
        std::cout << ">>>>>>>>>>>>>>>>>>>>>> " << message << std::endl;

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
