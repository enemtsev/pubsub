#include "server.h"
#include <boost/asio.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: server_app <port>\n";
        return 1;
    }

    boost::asio::io_context io_context;
    Server server(io_context, std::atoi(argv[1]));
    io_context.run();
    return 0;
}