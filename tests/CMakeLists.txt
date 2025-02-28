find_package(GTest REQUIRED)

add_executable(test_client test_client.cpp)
add_executable(test_server test_server.cpp)
add_executable(test_topic_manager test_topic_manager.cpp)
add_executable(test_message test_message.cpp)

target_link_libraries(test_client publish_subscribe_lib GTest::GTest GTest::Main)
target_link_libraries(test_server publish_subscribe_lib GTest::GTest GTest::Main)
target_link_libraries(test_topic_manager publish_subscribe_lib GTest::GTest GTest::Main)
target_link_libraries(test_message publish_subscribe_lib GTest::GTest GTest::Main)

include(CTest)
enable_testing()

add_test(NAME test_client COMMAND test_client)
add_test(NAME test_server COMMAND test_server)
add_test(NAME test_topic_manager COMMAND test_topic_manager)
add_test(NAME test_message COMMAND test_message)