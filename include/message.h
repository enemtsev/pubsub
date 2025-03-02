#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>
#include <sstream>

// Message types
enum class MessageType {
    CONNECT,
    DISCONNECT,
    PUBLISH,
    SUBSCRIBE,
    UNSUBSCRIBE,
    UNKNOWN
};

// Message structure
struct Message {
    MessageType type;
    std::string topic;
    std::string data;
    std::string client_name;

    // Serialize the message into a string
    std::string serialize() const;
    // Deserialize a string into a Message object
    static Message deserialize(const std::string& message);

    static constexpr std::string kConnectCommand{"CONNECT"};
    static constexpr std::string kDisconnectCommand{"DISCONNECT"};
    static constexpr std::string kPublishCommand{"PUBLISH"};
    static constexpr std::string kSubscribeCommand{"SUBSCRIBE"};
    static constexpr std::string kUnsubscribeCommand{"UNSUBSCRIBE"};
    static constexpr std::string kHelpCommand{"HELP"};
    static constexpr std::string kUnknowndCommand{"UNKNOWN"};
    static constexpr std::string kDelim{"\n"};
};

#endif // MESSAGE_H
