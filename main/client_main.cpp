#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <iostream>
#include <sstream>
#include <thread>
#include "console.h"
#include "pubsub_client.h"

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
    bool debug{false};

    if (argc > 1) {
        std::string flag = argv[1];
        if (flag == "--help") {
            std::cerr << "Usage: cleint_app [--debug/--help]\n";
            return 0;
        } else if (flag == "--debug") {
            debug = true;
        }
    }

    init_logging(debug);

    boost::asio::io_context io_context;
    pubsub::client::PubSubClient client(io_context);

    // Set up asynchronous console input
    boost::asio::posix::stream_descriptor input(io_context, ::dup(STDIN_FILENO));
    pubsub::client::handle_console_input(input, client);
    pubsub::client::print_help();

    io_context.run();
    return 0;
}
