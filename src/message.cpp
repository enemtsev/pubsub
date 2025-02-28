#include "message.h"

// Serialize the message into a string
std::string Message::serialize() const {
    std::ostringstream oss;
    switch (type) {
        case MessageType::CONNECT:
            oss << "CONNECT " << client_name;
            break;
        case MessageType::DISCONNECT:
            oss << "DISCONNECT";
            break;
        case MessageType::PUBLISH:
            oss << "PUBLISH " << topic << " " << data;
            break;
        case MessageType::SUBSCRIBE:
            oss << "SUBSCRIBE " << topic;
            break;
        case MessageType::UNSUBSCRIBE:
            oss << "UNSUBSCRIBE " << topic;
            break;
        default:
            oss << "UNKNOWN";
            break;
    }
    return oss.str();
}

// Deserialize a string into a Message object
Message Message::deserialize(const std::string& message) {
    Message msg;
    std::istringstream iss(message);
    std::string command;
    iss >> command;

    if (command == "CONNECT") {
        msg.type = MessageType::CONNECT;
        iss >> msg.client_name;
    } else if (command == "DISCONNECT") {
        msg.type = MessageType::DISCONNECT;
    } else if (command == "PUBLISH") {
        msg.type = MessageType::PUBLISH;
        iss >> msg.topic;
        std::getline(iss, msg.data); // Read the rest of the line as data
        msg.data.erase(0, 1); // Remove leading space
    } else if (command == "SUBSCRIBE") {
        msg.type = MessageType::SUBSCRIBE;
        iss >> msg.topic;
    } else if (command == "UNSUBSCRIBE") {
        msg.type = MessageType::UNSUBSCRIBE;
        iss >> msg.topic;
    } else {
        msg.type = MessageType::UNKNOWN;
    }

    return msg;
}