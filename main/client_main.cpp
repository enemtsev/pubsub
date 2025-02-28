#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include "client.h"

void print_help() {
    std::cout << "Available commands:\n"
              << "  CONNECT <port> <client_name> - Connect to the server on the specified port with a client name\n"
              << "  DISCONNECT                  - Disconnect from the server\n"
              << "  PUBLISH <topic> <data>      - Publish a message to a topic\n"
              << "  SUBSCRIBE <topic>           - Subscribe to a topic\n"
              << "  UNSUBSCRIBE <topic>         - Unsubscribe from a topic\n"
              << "  HELP                        - Display this help message\n";
}

int main() {
    boost::asio::io_context io_context;
    Client client(io_context);

    // Print the help message at the start
    print_help();

    boost::asio::io_context::work work(io_context);

    // Start a thread to run the IO context for asynchronous operations
    std::thread t([&io_context]() {
        io_context.run();
        std::cout << "done context client\n";
    });

    std::string command;
    while (true) {
        std::getline(std::cin, command);
        if (command.substr(0, 7) == "CONNECT") {
            size_t space1 = command.find(' ', 8);
            if (space1 == std::string::npos) {
                std::cerr << "Invalid CONNECT command. Usage: CONNECT <port> <client_name>\n";
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
                std::cerr << "Invalid PUBLISH command. Usage: PUBLISH <topic> <data>\n";
                continue;
            }
            std::string topic = command.substr(8, space1 - 8);
            std::string data = command.substr(space1 + 1);
            client.publish(topic, data);
        } else if (command.substr(0, 9) == "SUBSCRIBE") {
            std::string topic = command.substr(10);
            if (topic.empty()) {
                std::cerr << "Invalid SUBSCRIBE command. Usage: SUBSCRIBE <topic>\n";
                continue;
            }
            client.subscribe(topic);
        } else if (command.substr(0, 11) == "UNSUBSCRIBE") {
            std::string topic = command.substr(12);
            if (topic.empty()) {
                std::cerr << "Invalid UNSUBSCRIBE command. Usage: UNSUBSCRIBE <topic>\n";
                continue;
            }
            client.unsubscribe(topic);
        } else if (command == "HELP") {
            print_help();
        } else {
            std::cerr << "Unknown command. Type 'HELP' for a list of commands.\n";
        }
    }

    t.join();
    return 0;
}
