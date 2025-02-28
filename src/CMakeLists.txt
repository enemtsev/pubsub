add_library(publish_subscribe_lib
    client.cpp
    server.cpp
    message.cpp
    topic_manager.cpp
)

target_include_directories(publish_subscribe_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(publish_subscribe_lib Boost::system Threads::Threads)