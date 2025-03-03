#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <memory>
#include <unordered_map>
#include <set>
#include <string>
#include "topic_manager.h"

namespace pubsub::server {

class Server {

protected:
    virtual void process_message(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string &message) = 0;
    virtual void handle_disconnect(std::shared_ptr<boost::asio::ip::tcp::socket> socket) = 0;
};

}

#endif // SERVER_H
