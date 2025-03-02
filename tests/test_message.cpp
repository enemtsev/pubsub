#include <gtest/gtest.h>
#include "message.h"

TEST(MessageTest, SerializePublish) {
    Message msg;
    msg.type = MessageType::PUBLISH;
    msg.topic = "my_topic";
    msg.data = "Hello, World!";
    EXPECT_EQ(msg.serialize(), "PUBLISH my_topic Hello, World!\n");
}

TEST(MessageTest, SerializeConnect) {
    Message msg;
    msg.type = MessageType::CONNECT;
    msg.client_name = "client1";
    EXPECT_EQ(msg.serialize(), "CONNECT client1\n");
}

TEST(MessageTest, SerializeDisconnect) {
    Message msg;
    msg.type = MessageType::DISCONNECT;
    EXPECT_EQ(msg.serialize(), "DISCONNECT\n");
}

TEST(MessageTest, SerializeSubscribe) {
    Message msg;
    msg.type = MessageType::SUBSCRIBE;
    msg.topic = "my_topic";
    EXPECT_EQ(msg.serialize(), "SUBSCRIBE my_topic\n");
}

TEST(MessageTest, SerializeUnsubscribe) {
    Message msg;
    msg.type = MessageType::UNSUBSCRIBE;
    msg.topic = "my_topic";
    EXPECT_EQ(msg.serialize(), "UNSUBSCRIBE my_topic\n");
}

TEST(MessageTest, SerializeUnknown) {
    Message msg;
    msg.type = MessageType::UNKNOWN;
    EXPECT_EQ(msg.serialize(), "UNKNOWN\n");
}

TEST(MessageTest, DeserializeConnect) {
    std::string input = "CONNECT client1";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::CONNECT);
    EXPECT_EQ(msg.client_name, "client1");
}

TEST(MessageTest, DeserializeDisconnect) {
    std::string input = "DISCONNECT";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::DISCONNECT);
}

TEST(MessageTest, DeserializeSubscribe) {
    std::string input = "SUBSCRIBE my_topic";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::SUBSCRIBE);
    EXPECT_EQ(msg.topic, "my_topic");
}

TEST(MessageTest, DeserializeUnsubscribe) {
    std::string input = "UNSUBSCRIBE my_topic";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::UNSUBSCRIBE);
    EXPECT_EQ(msg.topic, "my_topic");
}

TEST(MessageTest, DeserializePublish) {
    std::string input = "PUBLISH my_topic Hello, World!";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::PUBLISH);
    EXPECT_EQ(msg.topic, "my_topic");
    EXPECT_EQ(msg.data, "Hello, World!");
}

TEST(MessageTest, DeserializeUnknown) {
    std::string input = "INVALID_COMMAND";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::UNKNOWN);
}

TEST(MessageTest, DeserializeEmptyString) {
    std::string input = "";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::UNKNOWN);
}

TEST(MessageTest, DeserializeWhitespaceString) {
    std::string input = "   ";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::UNKNOWN);
}

TEST(MessageTest, DeserializePartialCommand) {
    std::string input = "PUBLISH";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::UNKNOWN);
}

TEST(MessageTest, DeserializePublishWithNoData) {
    std::string input = "PUBLISH my_topic";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::PUBLISH);
    EXPECT_EQ(msg.topic, "my_topic");
    EXPECT_EQ(msg.data, "");
}

TEST(MessageTest, DeserializePublishWithExtraSpaces) {
    std::string input = "PUBLISH   my_topic   Hello, World!  ";
    Message msg = Message::deserialize(input);
    EXPECT_EQ(msg.type, MessageType::PUBLISH);
    EXPECT_EQ(msg.topic, "my_topic");
    EXPECT_EQ(msg.data, "  Hello, World!  ");
}
