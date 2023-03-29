enable_testing()

add_executable(vmTest vmTest.cpp)

target_include_directories(vmTest PUBLIC ../ ../../code ../../lexer ../../parser)

target_link_libraries(vmTest vm compiler code lexer parser spdlog::spdlog GTest::gtest_main)

include(GoogleTest)

gtest_discover_tests(vmTest)
