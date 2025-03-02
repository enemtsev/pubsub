#include "message.h"
#include <algorithm>
#include <boost/log/trivial.hpp>
#include <sstream>
#include <vector>

// Serialize the message into a string with endl as delimiter
std::string Message::serialize() const {
    std::ostringstream oss;
    switch (type) {
        case MessageType::CONNECT:
            oss << kConnectCommand << " " << client_name;
            break;
        case MessageType::DISCONNECT:
            oss << kDisconnectCommand;
            break;
        case MessageType::PUBLISH:
            oss << kPublishCommand << " " << topic << " " << data;
            break;
        case MessageType::SUBSCRIBE:
            oss << kSubscribeCommand << " " << topic;
            break;
        case MessageType::UNSUBSCRIBE:
            oss << kUnsubscribeCommand << " " << topic;
            break;
        default:
            oss << kUnknowndCommand;
            break;
    }
    oss << kDelim;  // Add delimiter
    return oss.str();
}

// Deserialize a string into a Message object using endl as delimiter
Message Message::deserialize(const std::string &message) {
    Message msg;
    std::istringstream iss(message);
    std::string command;
    iss >> command;

    if (command == kConnectCommand) {
        msg.type = MessageType::CONNECT;
        iss >> msg.client_name;
    } else if (command == kDisconnectCommand) {
        msg.type = MessageType::DISCONNECT;
    } else if (command == kPublishCommand) {
        msg.type = MessageType::PUBLISH;
        iss >> msg.topic;
        if (msg.topic.empty()) {
            msg.type = MessageType::UNKNOWN;
        } else {
            std::getline(iss, msg.data);  // Read the rest of the line as data
            msg.data.erase(0, 1);         // Remove leading space
        }
    } else if (command == kSubscribeCommand) {
        msg.type = MessageType::SUBSCRIBE;
        iss >> msg.topic;
    } else if (command == kUnsubscribeCommand) {
        msg.type = MessageType::UNSUBSCRIBE;
        iss >> msg.topic;
    } else {
        msg.type = MessageType::UNKNOWN;
    }

    return msg;
}
