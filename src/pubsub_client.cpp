#include "pubsub_client.h"
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace pubsub::client {

PubSubClient::PubSubClient(boost::asio::io_context &io_context)
    : Client(),
      exec_{io_context.get_executor()},
      socket_{std::make_shared<boost::asio::ip::tcp::socket>(io_context)} {}

void PubSubClient::connect(const std::string &client_name) {
    client_name_ = client_name;
    boost::asio::post(exec_, [this, client_name]() {
        BOOST_LOG_TRIVIAL(debug) << "[" << client_name << "] Connect as : " << client_name;
        Message msg;
        msg.type = MessageType::CONNECT;
        msg.data = client_name;
        write(msg);
    });
}

void PubSubClient::disconnect() {
    boost::asio::post(exec_, [this]() {
        BOOST_LOG_TRIVIAL(debug) << "[" << client_name_ << "] Disconnect";
        Message msg;
        msg.type = MessageType::DISCONNECT;
        write(msg);
    });
}

void PubSubClient::publish(const std::string &topic, const std::string &data) {
    boost::asio::post(exec_, [this, topic, data]() {
        BOOST_LOG_TRIVIAL(debug) << "[" << client_name_ << "] Publishing to topic: " << topic;
        Message msg;
        msg.type = MessageType::PUBLISH;
        msg.topic = topic;
        msg.data = data;
        write(msg);
    });
}

void PubSubClient::subscribe(const std::string &topic) {
    boost::asio::post(exec_, [this, topic]() {
        BOOST_LOG_TRIVIAL(debug) << "[" << client_name_ << "] Subscribing to topic: " << topic;
        Message msg;
        msg.type = MessageType::SUBSCRIBE;
        msg.topic = topic;
        write(msg);
    });
}

void PubSubClient::unsubscribe(const std::string &topic) {
    boost::asio::post(exec_, [this, topic]() {
        BOOST_LOG_TRIVIAL(debug) << "[" << client_name_ << "] Unsubscribing from topic: " << topic;
        Message msg;
        msg.type = MessageType::UNSUBSCRIBE;
        msg.topic = topic;
        write(msg);
    });
}

void PubSubClient::on_message_received(const std::string &topic, const std::string &message) {
    // Handle incoming messages (e.g., published data from the server)
    // [Message] Topic : <topic name> Data : < data > "
    BOOST_LOG_TRIVIAL(info) << "[" << client_name_ << "] [Message] Topic: \"" << topic << "\" Data: \"" << message
                            << "\"";
}

void PubSubClient::write(const Message &message) {
    boost::system::error_code error;
    boost::asio::write(*socket_, boost::asio::buffer(message.serialize()), error);

    if (!error) {
        BOOST_LOG_TRIVIAL(debug) << "[" << client_name_ << "] Message written successfully";
    } else if (error == boost::asio::error::eof) {
        BOOST_LOG_TRIVIAL(error) << "[" << client_name_ << "] Write failed: disconnected";
    } else {
        BOOST_LOG_TRIVIAL(error) << "[" << client_name_ << "] Error writing to server: " << error.message();
    }
}

void PubSubClient::connect_socket(const std::string &host, const std::string &port) {
    BOOST_LOG_TRIVIAL(debug) << "[client] Posting Connect socket : " << host << ":" << port;

    boost::asio::post(exec_, [this, host, port]() {
        BOOST_LOG_TRIVIAL(debug) << "[client] Attempting to connect " << host << ":" << port;
        boost::asio::ip::tcp::resolver resolver(socket_->get_executor());
        auto endpoints = resolver.resolve(host, port);
        boost::system::error_code error;
        boost::asio::connect(*socket_, endpoints, error);

        if (!error) {
            BOOST_LOG_TRIVIAL(info) << "[client] Connected to server " << host << ":" << port;
            // Start reading messages asynchronously
            start_reading();
        } else {
            BOOST_LOG_TRIVIAL(error) << "[client] Connection failed " << host << ":" << port << " " << error.message();
        }
    });
}

void PubSubClient::disconnect_socket() {
    client_name_.clear();
    socket_->close();
    BOOST_LOG_TRIVIAL(info) << "[" << client_name_ << "] Disconnected from server";
}

void PubSubClient::start_reading() {
    // Create a buffer for the client
    auto client_buffer = std::make_shared<std::string>();
    auto read_buffer = std::make_shared<std::array<char, 1024>>();

    socket_->async_read_some(
        boost::asio::buffer(*read_buffer),
        [this, client_buffer, read_buffer](const boost::system::error_code &error, std::size_t bytes_transferred) {
            if (!error) {
                BOOST_LOG_TRIVIAL(debug) << "[" << client_name_ << "] Received data: " << bytes_transferred << " bytes";
                std::string message(read_buffer->data(), bytes_transferred);
                BOOST_LOG_TRIVIAL(debug) << "[" << client_name_ << "] Received raw message: " << message;

                handle_read(socket_, client_buffer, read_buffer, error, bytes_transferred);
            } else if (error == boost::asio::error::eof) {
                BOOST_LOG_TRIVIAL(error) << "[" << client_name_ << "] Server disconnected";
            } else {
                BOOST_LOG_TRIVIAL(error) << "[" << client_name_ << "] Error reading from server: " << error.message();
            }
        });
}

void PubSubClient::handle_read(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                               std::shared_ptr<std::string> client_buffer,
                               std::shared_ptr<std::array<char, 1024>> read_buffer,
                               const boost::system::error_code &error, std::size_t bytes_transferred) {
    if (!error) {
        // Append the received data to the client buffer
        client_buffer->append(read_buffer->data(), bytes_transferred);

        // Check if the buffer contains a complete message (delimited by endl)
        std::size_t delimiter_pos = client_buffer->find(Message::kDelim);
        while (delimiter_pos != std::string::npos) {
            // Extract the complete message
            std::string message = client_buffer->substr(0, delimiter_pos);
            client_buffer->erase(0, delimiter_pos + 1);  // Remove the processed message from the buffer

            // Log and process the message
            BOOST_LOG_TRIVIAL(debug) << "[" << client_name_ << "] Received message: " << message;
            Message msg = Message::deserialize(message);
            if (msg.type == MessageType::PUBLISH) {
                on_message_received(msg.topic, msg.data);
            }

            // Look for the next message in the buffer
            delimiter_pos = client_buffer->find(Message::kDelim);
        }

        // Continue reading from the client
        socket->async_read_some(boost::asio::buffer(*read_buffer),
                                [this, socket, client_buffer, read_buffer](const boost::system::error_code &error,
                                                                           std::size_t bytes_transferred) {
                                    handle_read(socket, client_buffer, read_buffer, error, bytes_transferred);
                                });
    } else {
        BOOST_LOG_TRIVIAL(error) << "[" << client_name_ << "] Error reading from server: " << error.message();
    }
}

}  // namespace pubsub::client
