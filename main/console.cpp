#include "console.h"

#include <boost/log/trivial.hpp>

namespace pubsub_client {

void print_help() {
    BOOST_LOG_TRIVIAL(info) << "Available commands:";
    BOOST_LOG_TRIVIAL(info)
        << "  CONNECT <port> <client_name> - Connect to the server on the specified port with a client name";
    BOOST_LOG_TRIVIAL(info) << "  DISCONNECT                  - Disconnect from the server";
    BOOST_LOG_TRIVIAL(info) << "  PUBLISH <topic> <data>      - Publish a message to a topic";
    BOOST_LOG_TRIVIAL(info) << "  SUBSCRIBE <topic>           - Subscribe to a topic";
    BOOST_LOG_TRIVIAL(info) << "  UNSUBSCRIBE <topic>         - Unsubscribe from a topic";
    BOOST_LOG_TRIVIAL(info) << "  HELP                        - Display this help message";
}

void handle_console_input(boost::asio::posix::stream_descriptor &input, PubSubClient &client) {
    auto buffer = std::make_shared<boost::asio::streambuf>();
    boost::asio::async_read_until(
        input, *buffer, '\n', [&, buffer](const boost::system::error_code &ec, std::size_t length) {
            if (!ec) {
                std::istream is(buffer.get());
                std::string command;
                std::getline(is, command);

                if (command.substr(0, 7) == Message::kConnectCommand) {
                    size_t space1 = command.find(' ', 8);
                    if (space1 == std::string::npos) {
                        BOOST_LOG_TRIVIAL(error) << "Invalid CONNECT command. Usage: CONNECT <port> <client_name>";
                    } else {
                        std::string port = command.substr(8, space1 - 8);
                        std::string name = command.substr(space1 + 1);
                        client.connect_socket("127.0.0.1", port);
                        client.connect(name);
                    }
                } else if (command == Message::kDisconnectCommand) {
                    client.disconnect();
                    client.disconnect_socket();
                } else if (command.substr(0, 7) == Message::kPublishCommand) {
                    size_t space1 = command.find(' ', 8);
                    if (space1 == std::string::npos) {
                        BOOST_LOG_TRIVIAL(error) << "Invalid PUBLISH command. Usage: PUBLISH <topic> <data>";
                    } else {
                        std::string topic = command.substr(8, space1 - 8);
                        std::string data = command.substr(space1 + 1);
                        client.publish(topic, data);
                    }
                } else if (command.substr(0, 9) == Message::kSubscribeCommand) {
                    std::string topic = command.substr(10);
                    if (topic.empty()) {
                        BOOST_LOG_TRIVIAL(error) << "Invalid SUBSCRIBE command. Usage: SUBSCRIBE <topic>";
                    } else {
                        client.subscribe(topic);
                    }
                } else if (command.substr(0, 11) == Message::kUnsubscribeCommand) {
                    std::string topic = command.substr(12);
                    if (topic.empty()) {
                        BOOST_LOG_TRIVIAL(error) << "Invalid UNSUBSCRIBE command. Usage: UNSUBSCRIBE <topic>";
                    } else {
                        client.unsubscribe(topic);
                    }
                } else if (command == Message::kHelpCommand) {
                    print_help();
                } else {
                    BOOST_LOG_TRIVIAL(error) << "Unknown command. Type 'HELP' for a list of commands.";
                }

                // Continue reading the next command
                handle_console_input(input, client);
            } else {
                BOOST_LOG_TRIVIAL(error) << "Error reading from console: " << ec.message();
            }
        });
}

}  // namespace pubsub_client
