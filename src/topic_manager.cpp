#include "topic_manager.h"
#include <boost/log/trivial.hpp>
#include <iostream>

TopicManager &TopicManager::get_instance() {
    static TopicManager instance;
    return instance;
}

void TopicManager::subscribe(const std::string &topic, std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    topics_[topic].insert(socket);
    BOOST_LOG_TRIVIAL(info) << "[topic_manager] Client subscribed to topic: " << topic;
}

void TopicManager::unsubscribe(const std::string &topic, std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    topics_[topic].erase(socket);
    BOOST_LOG_TRIVIAL(info) << "[topic_manager] Client unsubscribed from topic: " << topic;
}

void TopicManager::publish(const std::string &topic, const std::string &data) {
    if (topics_.find(topic) == topics_.end()) {
        BOOST_LOG_TRIVIAL(warning) << "[topic_manager] Topic not found: " << topic;
        return;
    }

    for (auto &socket : topics_[topic]) {
        if (socket->is_open()) {
            std::string message = "[Message] Topic: " + topic + " Data: " + data + "\n";
            BOOST_LOG_TRIVIAL(info) << "[topic_manager] Publishing message to topic: " << topic;
            boost::asio::write(*socket, boost::asio::buffer(message));
        } else {
            BOOST_LOG_TRIVIAL(warning) << "[topic_manager] Socket is closed, skipping message for topic: " << topic;
        }
    }
}
