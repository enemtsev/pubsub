#include "pubsub_server.h"
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "message.h"

PubSubServer::PubSubServer(boost::asio::io_context &io_context, short port)
    : Server(io_context, port) {}

void PubSubServer::process_message(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string &message) {
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
