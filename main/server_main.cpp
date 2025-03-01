#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <iostream>
#include "server.h"

void init_logging() {
    boost::log::add_console_log(std::cout, boost::log::keywords::format = "%TimeStamp% [%Severity%]: %Message%");
    boost::log::add_common_attributes();  // Add common attributes like timestamps
}

int main(int argc, char *argv[]) {
    init_logging();

    if (argc != 2) {
        std::cerr << "Usage: server_app <port>\n";
        return 1;
    }

    boost::asio::io_context io_context;
    Server server(io_context, std::atoi(argv[1]));
    io_context.run();
    return 0;
}
