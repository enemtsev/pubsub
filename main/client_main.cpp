#include <boost/log/trivial.hpp>                          // Include Boost.Log headers
#include <boost/log/utility/setup/common_attributes.hpp>  // For common attributes like timestamps
#include <boost/log/utility/setup/console.hpp>            // For console logging
#include <chrono>
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

int main() {
    init_logging();  // Initialize Boost.Log

    boost::asio::io_context io_context;
    Client client(io_context);

    // Print the help message at the start
    print_help();

    boost::asio::io_context::work work(io_context);

    // Start a thread to run the IO context for asynchronous operations
    std::thread t([&io_context]() {
        io_context.run();
        BOOST_LOG_TRIVIAL(info) << "IO context finished";
    });

    std::string command;
    while (true) {
        std::getline(std::cin, command);
        if (command.substr(0, 7) == "CONNECT") {
            size_t space1 = command.find(' ', 8);
            if (space1 == std::string::npos) {
                BOOST_LOG_TRIVIAL(error) << "Invalid CONNECT command. Usage: CONNECT <port> <client_name>";
                continue;
            }
            std::string port = command.substr(8, space1 - 8);
            std::string name = command.substr(space1 + 1);
            client.connect("127.0.0.1", port, name);
        } else if (command == "DISCONNECT") {
            client.disconnect();
            break;
        } else if (command.substr(0, 7) == "PUBLISH") {
            size_t space1 = command.find(' ', 8);
            if (space1 == std::string::npos) {
                BOOST_LOG_TRIVIAL(error) << "Invalid PUBLISH command. Usage: PUBLISH <topic> <data>";
                continue;
            }
            std::string topic = command.substr(8, space1 - 8);
            std::string data = command.substr(space1 + 1);
            client.publish(topic, data);
        } else if (command.substr(0, 9) == "SUBSCRIBE") {
            std::string topic = command.substr(10);
            if (topic.empty()) {
                BOOST_LOG_TRIVIAL(error) << "Invalid SUBSCRIBE command. Usage: SUBSCRIBE <topic>";
                continue;
            }
            client.subscribe(topic);
        } else if (command.substr(0, 11) == "UNSUBSCRIBE") {
            std::string topic = command.substr(12);
            if (topic.empty()) {
                BOOST_LOG_TRIVIAL(error) << "Invalid UNSUBSCRIBE command. Usage: UNSUBSCRIBE <topic>";
                continue;
            }
            client.unsubscribe(topic);
        } else if (command == "HELP") {
            print_help();
        } else {
            BOOST_LOG_TRIVIAL(error) << "Unknown command. Type 'HELP' for a list of commands.";
        }
    }

    t.join();
    return 0;
}
