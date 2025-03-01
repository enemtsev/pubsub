#include "topic_manager.h"
#include <iostream>

TopicManager &TopicManager::get_instance() {
    static TopicManager instance;
    return instance;
}

void TopicManager::subscribe(const std::string &topic, std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    topics_[topic].insert(socket);
}

void TopicManager::unsubscribe(const std::string &topic, std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    topics_[topic].erase(socket);
}

void TopicManager::publish(const std::string &topic, const std::string &data) {
    std::cerr << "TopicManager::publish found: " << (topics_.find(topic) == topics_.cend()) << std::endl;
    for (auto &socket : topics_[topic]) {
        std::string message = "[Message] Topic: " + topic + " Data: " + data + "\n";
        std::cerr << "TopicManager::publish: " << message << std::endl;
        boost::asio::write(*socket, boost::asio::buffer(message));
    }
}
