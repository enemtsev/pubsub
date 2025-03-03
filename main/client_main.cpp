#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <iostream>
#include <sstream>
#include <thread>
#include "console.h"
#include "pubsub_client.h"

void init_logging() {
    boost::log::add_console_log(std::cout, boost::log::keywords::format = "%TimeStamp% [%Severity%]: %Message%");
    boost::log::add_common_attributes();
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
    // Debug
    // boost::log::core::get()->set_filter(logging::trivial::severity >= boost::log::trivial::debug);
}

int main() {
    init_logging();  // Initialize Boost.Log

    boost::asio::io_context io_context;
    pubsub::client::PubSubClient client(io_context);

    // Set up asynchronous console input
    boost::asio::posix::stream_descriptor input(io_context, ::dup(STDIN_FILENO));
    pubsub::client::handle_console_input(input, client);
    pubsub::client::print_help();

    io_context.run();
    return 0;
}
