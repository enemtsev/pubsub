#include "pubsub_client.h"
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

PubSubClient::PubSubClient(boost::asio::io_context &io_context)
    : Client(io_context) {}

void PubSubClient::publish(const std::string &topic, const std::string &data) {
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

void PubSubClient::subscribe(const std::string &topic) {
    boost::asio::post(exec_, [this, topic]() {
        BOOST_LOG_TRIVIAL(info) << "[" << client_name_ << "] Subscribing to topic: " << topic;
        Message msg;
        msg.type = MessageType::SUBSCRIBE;
        msg.topic = topic;
        msg.client_name = client_name_;
        write(msg);
    });
}

void PubSubClient::unsubscribe(const std::string &topic) {
    boost::asio::post(exec_, [this, topic]() {
        BOOST_LOG_TRIVIAL(info) << "[" << client_name_ << "] Unsubscribing from topic: " << topic;
        Message msg;
        msg.type = MessageType::UNSUBSCRIBE;
        msg.topic = topic;
        msg.client_name = client_name_;
        write(msg);
    });
}

void PubSubClient::on_message_received(const std::string &message) {
    // Handle incoming messages (e.g., published data from the server)
    BOOST_LOG_TRIVIAL(info) << "[" << client_name_ << "] Received pub/sub message: " << message;
}

void PubSubClient::write(const Message &message) {
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
