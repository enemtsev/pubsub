#include <benchmark/benchmark.h>
#include "topic_manager.h"

static void BM_TopicManagerPublish(benchmark::State& state) {
    boost::asio::io_context io_context;
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
    TopicManager& manager = TopicManager::get_instance();
    manager.subscribe("test_topic", socket);

    for (auto _ : state) {
        manager.publish("test_topic", "test_data");
    }
}
BENCHMARK(BM_TopicManagerPublish);

BENCHMARK_MAIN();