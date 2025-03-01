find_package(GTest REQUIRED)

# Add all test executables
add_executable(test_client test_client.cpp)
add_executable(test_server test_server.cpp)
add_executable(test_topic_manager test_topic_manager.cpp)
add_executable(test_message test_message.cpp)

# Link libraries for each test executable
target_link_libraries(test_client publish_subscribe_lib GTest::GTest GTest::Main GTest::gmock)
target_link_libraries(test_server publish_subscribe_lib GTest::GTest GTest::Main GTest::gmock)
target_link_libraries(test_topic_manager publish_subscribe_lib GTest::GTest GTest::Main)
target_link_libraries(test_message publish_subscribe_lib GTest::GTest GTest::Main)

# Enable testing and add tests
include(CTest)
enable_testing()

add_test(NAME test_client COMMAND test_client)
add_test(NAME test_server COMMAND test_server)
add_test(NAME test_topic_manager COMMAND test_topic_manager)
add_test(NAME test_message COMMAND test_message)

# Add a custom target for running all tests
add_custom_target(tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_client test_server test_topic_manager test_message
)
