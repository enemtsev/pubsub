#ifndef TOPIC_MANAGER_H
#define TOPIC_MANAGER_H

#include <unordered_map>
#include <set>
#include <string>
#include <boost/asio.hpp>
#include <memory>

class TopicManager {
public:
    static TopicManager& get_instance();
    void subscribe(const std::string& topic, std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void unsubscribe(const std::string& topic, std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void publish(const std::string& topic, const std::string& data);

private:
    TopicManager() {}
    std::unordered_map<std::string, std::set<std::shared_ptr<boost::asio::ip::tcp::socket>>> topics_;
};

#endif // TOPIC_MANAGER_H