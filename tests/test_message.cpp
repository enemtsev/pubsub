#include <gtest/gtest.h>
#include "message.h"

TEST(MessageTest, SerializePublish) {
    Message msg;
    msg.type = MessageType::PUBLISH;
    msg.topic = "my_topic";
    msg.data = "Hello, World!";
    EXPECT_EQ(msg.serialize(), "PUBLISH my_topic Hello, World!");
}

TEST(MessageTest, DeserializeSubscribe) {
    std::string input = "SUBSCRIBE my_topic";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::SUBSCRIBE);
    EXPECT_EQ(msg.topic, "my_topic");
}

TEST(MessageTest, DeserializeUnknown) {
    std::string input = "INVALID_COMMAND";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::UNKNOWN);
}