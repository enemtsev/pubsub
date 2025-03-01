cmake_minimum_required(VERSION 3.14)
project(pubsub)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system log log_setup)
find_package(Threads REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(benchmarks)
add_subdirectory(main)
