#include "client.h"
#include <boost/log/trivial.hpp>                          // Include Boost.Log headers
#include <boost/log/utility/setup/common_attributes.hpp>  // For common attributes like timestamps
#include <boost/log/utility/setup/console.hpp>            // For console logging

Client::Client(boost::asio::io_context &io_context)
    : exec_{io_context.get_executor()},
      socket_(io_context) {}

void Client::connect(const std::string &host, const std::string &port, const std::string &client_name) {
    BOOST_LOG_TRIVIAL(debug) << "[" << client_name << "] Post connect";
    client_name_ = client_name;
    boost::asio::post(exec_, [this, host, port, client_name]() {
        BOOST_LOG_TRIVIAL(debug) << "[" << client_name << "] Attempting to connect";
        boost::asio::ip::tcp::resolver resolver(socket_.get_executor());
        auto endpoints = resolver.resolve(host, port);
        boost::system::error_code error;
        boost::asio::connect(socket_, endpoints, error);

        if (!error) {
            BOOST_LOG_TRIVIAL(info) << "[" << client_name << "] Connected to server";

            // Send CONNECT message after connection
            Message msg;
            msg.type = MessageType::CONNECT;
            msg.client_name = client_name;
            write(msg);

            // Start reading messages asynchronously
            start_reading();
        } else {
            BOOST_LOG_TRIVIAL(error) << "[" << client_name << "] Connection failed: " << error.message();
        }
    });
}

void Client::write(const Message &message) {
    boost::system::error_code error;
    boost::asio::write(socket_, boost::asio::buffer(message.serialize()), error);

    if (!error) {
        BOOST_LOG_TRIVIAL(debug) << "[" << message.client_name << "] Message written successfully";
    } else if (error == boost::asio::error::eof) {
        BOOST_LOG_TRIVIAL(error) << "[" << message.client_name << "] Write failed: disconnected";
    } else {
        BOOST_LOG_TRIVIAL(error) << "[" << message.client_name << "] Error writing to server: " << error.message();
    }
}

void Client::disconnect() {
    client_name_.clear();
    socket_.close();
    BOOST_LOG_TRIVIAL(info) << "[" << client_name_ << "] Client disconnected";
}

void Client::publish(const std::string &topic, const std::string &data) {
    boost::asio::post(exec_, [this, topic, data]() {
        BOOST_LOG_TRIVIAL(debug) << "[" << client_name_ << "] Publishing to topic: " << topic;
        Message msg;
        msg.type = MessageType::PUBLISH;
        msg.topic = topic;
        msg.data = data;
        msg.client_name = client_name_;
        write(msg);
    });
}

void Client::subscribe(const std::string &topic) {
    boost::asio::post(exec_, [this, topic]() {
        BOOST_LOG_TRIVIAL(info) << "[" << client_name_ << "] Subscribing to topic: " << topic;
        Message msg;
        msg.type = MessageType::SUBSCRIBE;
        msg.topic = topic;
        msg.client_name = client_name_;
        write(msg);
    });
}

void Client::unsubscribe(const std::string &topic) {
    boost::asio::post(exec_, [this, topic]() {
        BOOST_LOG_TRIVIAL(info) << "[" << client_name_ << "] Unsubscribing from topic: " << topic;
        Message msg;
        msg.type = MessageType::UNSUBSCRIBE;
        msg.topic = topic;
        msg.client_name = client_name_;
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
        BOOST_LOG_TRIVIAL(debug) << "[" << client_name_ << "] Received data: " << bytes_transferred << " bytes";
        // Print the received message
        std::string message(buffer_.data(), bytes_transferred);
        BOOST_LOG_TRIVIAL(info) << "[" << client_name_ << "] Received message: " << message;

        // Continue reading asynchronously
        start_reading();
    } else if (error == boost::asio::error::eof) {
        BOOST_LOG_TRIVIAL(error) << "[" << client_name_ << "] Server disconnected";
    } else {
        BOOST_LOG_TRIVIAL(error) << "[" << client_name_ << "] Error reading from server: " << error.message();
    }
}
