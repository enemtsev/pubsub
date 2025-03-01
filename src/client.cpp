#include "client.h"
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

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

            // Start reading messages asynchronously
            start_reading();
        } else {
            BOOST_LOG_TRIVIAL(error) << "[" << client_name << "] Connection failed: " << error.message();
        }
    });
}

void Client::disconnect() {
    client_name_.clear();
    socket_.close();
    BOOST_LOG_TRIVIAL(info) << "[" << client_name_ << "] Client disconnected";
}

void Client::start_reading() {
    socket_.async_read_some(
        boost::asio::buffer(buffer_), [this](const boost::system::error_code &error, std::size_t bytes_transferred) {
            if (!error) {
                BOOST_LOG_TRIVIAL(debug) << "[" << client_name_ << "] Received data: " << bytes_transferred << " bytes";
                std::string message(buffer_.data(), bytes_transferred);
                BOOST_LOG_TRIVIAL(info) << "[" << client_name_ << "] Received message: " << message;

                // Call the pure virtual function to handle the message
                on_message_received(message);

                // Continue reading asynchronously
                start_reading();
            } else if (error == boost::asio::error::eof) {
                BOOST_LOG_TRIVIAL(error) << "[" << client_name_ << "] Server disconnected";
            } else {
                BOOST_LOG_TRIVIAL(error) << "[" << client_name_ << "] Error reading from server: " << error.message();
            }
        });
}
