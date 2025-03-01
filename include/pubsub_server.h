#ifndef PUBSUB_SERVER_H
#define PUBSUB_SERVER_H

#include <boost/asio.hpp>
#include <memory>
#include <unordered_map>
#include <set>
#include <string>
#include "server.h"

class PubSubServer : public Server {
public:
    PubSubServer(boost::asio::io_context& io_context, short port);
    void process_message(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string &message) override;
};

#endif // PUBSUB_SERVER_H
