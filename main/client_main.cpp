#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>                          // Include Boost.Log headers
#include <boost/log/utility/setup/common_attributes.hpp>  // For common attributes like timestamps
#include <boost/log/utility/setup/console.hpp>            // For console logging
#include <iostream>
#include <sstream>
#include <thread>
#include "client.h"

void init_logging() {
    boost::log::add_console_log(std::cout, boost::log::keywords::format = "%TimeStamp% [%Severity%]: %Message%");
    boost::log::add_common_attributes();
}

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

void handle_console_input(boost::asio::posix::stream_descriptor &input, Client &client) {
    auto buffer = std::make_shared<boost::asio::streambuf>();
    boost::asio::async_read_until(
        input, *buffer, '\n', [&, buffer](const boost::system::error_code &ec, std::size_t length) {
            if (!ec) {
                std::istream is(buffer.get());
                std::string command;
                std::getline(is, command);

                if (command.substr(0, 7) == "CONNECT") {
                    size_t space1 = command.find(' ', 8);
                    if (space1 == std::string::npos) {
                        BOOST_LOG_TRIVIAL(error) << "Invalid CONNECT command. Usage: CONNECT <port> <client_name>";
                    } else {
                        std::string port = command.substr(8, space1 - 8);
                        std::string name = command.substr(space1 + 1);
                        client.connect("127.0.0.1", port, name);
                    }
                } else if (command == "DISCONNECT") {
                    client.disconnect();
                } else if (command.substr(0, 7) == "PUBLISH") {
                    size_t space1 = command.find(' ', 8);
                    if (space1 == std::string::npos) {
                        BOOST_LOG_TRIVIAL(error) << "Invalid PUBLISH command. Usage: PUBLISH <topic> <data>";
                    } else {
                        std::string topic = command.substr(8, space1 - 8);
                        std::string data = command.substr(space1 + 1);
                        client.publish(topic, data);
                    }
                } else if (command.substr(0, 9) == "SUBSCRIBE") {
                    std::string topic = command.substr(10);
                    if (topic.empty()) {
                        BOOST_LOG_TRIVIAL(error) << "Invalid SUBSCRIBE command. Usage: SUBSCRIBE <topic>";
                    } else {
                        client.subscribe(topic);
                    }
                } else if (command.substr(0, 11) == "UNSUBSCRIBE") {
                    std::string topic = command.substr(12);
                    if (topic.empty()) {
                        BOOST_LOG_TRIVIAL(error) << "Invalid UNSUBSCRIBE command. Usage: UNSUBSCRIBE <topic>";
                    } else {
                        client.unsubscribe(topic);
                    }
                } else if (command == "HELP") {
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

int main() {
    init_logging();  // Initialize Boost.Log

    boost::asio::io_context io_context;
    Client client(io_context);

    // Print the help message at the start
    print_help();

    // Set up asynchronous console input
    boost::asio::posix::stream_descriptor input(io_context, ::dup(STDIN_FILENO));
    handle_console_input(input, client);

    io_context.run();
    return 0;
}
