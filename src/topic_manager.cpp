#include "topic_manager.h"
#include <boost/log/trivial.hpp>
#include "message.h"

void TopicManager::subscribe(const std::string &topic, std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    topics_[topic].insert(socket);
}

void TopicManager::unsubscribe(const std::string &topic, std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    auto it = topics_.find(topic);
    if (it != topics_.end()) {
        it->second.erase(socket);
        if (it->second.empty()) {
            topics_.erase(it);
        }
    }
}

void TopicManager::unsubscribe_all(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    for (auto &topic_entry : topics_) {
        topic_entry.second.erase(socket);
    }

    // Optionally, remove topics that have no subscribers left
    for (auto it = topics_.begin(); it != topics_.end();) {
        if (it->second.empty()) {
            it = topics_.erase(it);
        } else {
            ++it;
        }
    }
}

void TopicManager::publish(const std::string &topic, const std::string &data) {
    if (topics_.find(topic) == topics_.end()) {
        BOOST_LOG_TRIVIAL(warning) << "[topic_manager] Topic not found: " << topic;
        return;
    }

    for (auto &socket : topics_[topic]) {
        if (socket->is_open()) {
            Message msg;
            msg.type = MessageType::PUBLISH;
            msg.topic = topic;
            msg.data = data;

            std::string message = msg.serialize();
            BOOST_LOG_TRIVIAL(debug) << "[topic_manager] Publishing message to topic: " << topic;
            boost::system::error_code ec;
            boost::asio::write(*socket, boost::asio::buffer(message), ec);

            if (ec) {
                BOOST_LOG_TRIVIAL(warning) << "[topic_manager] Write failed: " << ec.message();
            }
        } else {
            BOOST_LOG_TRIVIAL(warning) << "[topic_manager] Socket is closed, skipping message for topic: " << topic;
        }
    }
}
