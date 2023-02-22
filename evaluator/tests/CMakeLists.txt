enable_testing()

add_executable(evaulatorTest evaluatorTest.cpp)

target_include_directories(evaulatorTest PUBLIC ../ ../../parser ../../lexer ../../token ../../object)

target_link_libraries(evaulatorTest evaluator object parser lexer token spdlog::spdlog GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(evaulatorTest)
