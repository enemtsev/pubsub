#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "message.h"

namespace pubsub::client {

// Client base class
class Client {

protected:
    virtual void on_message_received(const std::string& topic, const std::string& message) = 0;
    virtual void write(const Message &message) = 0;
};

}  // namespace pubsub::client

#endif // CLIENT_H
