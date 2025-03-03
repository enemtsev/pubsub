#include <benchmark/benchmark.h>

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

#include "pubsub_client.h"
#include "pubsub_server.h"

static void BM_Publish(benchmark::State &state) {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::error);

    boost::asio::io_context io_server_context;
    boost::asio::io_context client1_io_context;
    boost::asio::io_context client2_io_context;

    pubsub::server::PubSubServer server(io_server_context, 12345);
    pubsub::client::PubSubClient client1(client1_io_context);
    pubsub::client::PubSubClient client2(client2_io_context);

    std::thread server_thread([&io_server_context]() {
        io_server_context.run();
    });

    std::thread client1_thread([&client1_io_context]() {
        boost::asio::io_context::work work(client1_io_context);
        client1_io_context.run();
    });

    std::thread client2_thread([&client2_io_context]() {
        boost::asio::io_context::work work(client2_io_context);
        client2_io_context.run();
    });

    client1.connect_socket("127.0.0.1", "12345");
    client1.connect("client1");
    client1.subscribe("topic");

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    client2.connect_socket("127.0.0.1", "12345");
    client2.connect("client2");

    for (auto _ : state) {
        client2.publish("topic", "test_data");
    }

    client1.disconnect();
    client2.disconnect();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    io_server_context.stop();
    client1_io_context.stop();
    client2_io_context.stop();

    if (server_thread.joinable()) {
        server_thread.join();
    }
    if (client1_thread.joinable()) {
        client1_thread.join();
    }
    if (client2_thread.joinable()) {
        client2_thread.join();
    }
}

BENCHMARK(BM_Publish);

BENCHMARK_MAIN();
