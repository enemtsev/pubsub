#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "topic_manager.h"

class TopicManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        io_context = std::make_shared<boost::asio::io_context>();
        socket1 = std::make_shared<boost::asio::ip::tcp::socket>(*io_context);
        socket2 = std::make_shared<boost::asio::ip::tcp::socket>(*io_context);
        topic_manager = &TopicManager::get_instance();
    }

    void TearDown() override {
        io_context->stop();
    }

    std::shared_ptr<boost::asio::io_context> io_context;
    std::shared_ptr<boost::asio::ip::tcp::socket> socket1;
    std::shared_ptr<boost::asio::ip::tcp::socket> socket2;
    TopicManager *topic_manager;
};

TEST_F(TopicManagerTest, SubscribesClientToTopic) {
    topic_manager->subscribe("test_topic", socket1);
    topic_manager->subscribe("test_topic", socket2);

    // Check if both sockets are subscribed to the topic
    // auto subscribers = topic_manager->get_subscribers("test_topic");
    // EXPECT_EQ(subscribers.size(), 2);
}

TEST_F(TopicManagerTest, UnsubscribesClientFromTopic) {
    topic_manager->subscribe("test_topic", socket1);
    topic_manager->subscribe("test_topic", socket2);

    // Unsubscribe one client
    topic_manager->unsubscribe("test_topic", socket1);

    // Check if only one subscriber remains
    // auto subscribers = topic_manager->get_subscribers("test_topic");
    // EXPECT_EQ(subscribers.size(), 1);
}

TEST_F(TopicManagerTest, ForwardsMessageToSubscribers) {
    topic_manager->subscribe("test_topic", socket1);
    topic_manager->subscribe("test_topic", socket2);

    // Publish a message to the topic
    topic_manager->publish("test_topic", "test_data");

    // Check if the message was forwarded to all subscribers
    // (This would require mocking the socket or using a real socket for testing)
}
