#ifndef PUBSUB_CLIENT_H
#define PUBSUB_CLIENT_H

#include <boost/asio.hpp>
#include <string>
#include <functional>
#include "message.h"
#include "client.h"

class PubSubClient : public Client {
public:
    PubSubClient(boost::asio::io_context &io_context);
    void publish(const std::string& topic, const std::string& data);
    void subscribe(const std::string& topic);
    void unsubscribe(const std::string& topic);

    void on_message_received(const std::string& message) override;
    void write(const Message &message) override;
};

#endif // PUBSUB_CLIENT_H
