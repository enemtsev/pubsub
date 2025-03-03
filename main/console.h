#include "pubsub_client.h"

namespace pubsub::client {

void handle_console_input(boost::asio::posix::stream_descriptor &input, PubSubClient &client);
void print_help();

}
