#include "topic_manager.h"

TopicManager& TopicManager::get_instance() {
    static TopicManager instance;
    return instance;
}

void TopicManager::subscribe(const std::string& topic, std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    topics_[topic].insert(socket);
}

void TopicManager::unsubscribe(const std::string& topic, std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    topics_[topic].erase(socket);
}

void TopicManager::publish(const std::string& topic, const std::string& data) {
    for (auto& socket : topics_[topic]) {
        std::string message = "[Message] Topic: " + topic + " Data: " + data + "\n";
        boost::asio::write(*socket, boost::asio::buffer(message));
    }
}