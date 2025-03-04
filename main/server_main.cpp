#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <iostream>
#include "pubsub_server.h"

void init_logging(bool debug) {
    boost::log::add_console_log(std::cout, boost::log::keywords::format = "%TimeStamp% [%Severity%]: %Message%");
    boost::log::add_common_attributes();

    const auto severity = debug ? boost::log::trivial::debug : boost::log::trivial::info;

    boost::log::core::get()->set_filter(boost::log::trivial::severity >= severity);

    if (debug) {
        BOOST_LOG_TRIVIAL(debug) << "Debug mode enabled.\n";
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: server_app <port> [--debug]\n";
        return 1;
    }

    bool debug{false};

    // Check for debug flag
    if (argc > 2) {
        std::string debug_flag = argv[2];
        if (debug_flag == "--debug") {
            debug = true;
        } else {
            BOOST_LOG_TRIVIAL(error) << "Unknown argument: " << debug_flag << "\n";
            return 1;
        }
    }

    init_logging(debug);

    short port{0};

    try {
        port = std::stoi(argv[1]);
    } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(error) << "Invalid port number.\n";
        return 1;
    }

    boost::asio::io_context io_context;
    pubsub::server::PubSubServer server(io_context, port);
    io_context.run();
    return 0;
}
