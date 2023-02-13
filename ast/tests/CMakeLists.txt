enable_testing()

add_executable(astTest astTest.cpp)

target_include_directories(astTest PRIVATE ../ ../../token)

target_link_libraries(astTest lexer parser spdlog::spdlog GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(astTest)
