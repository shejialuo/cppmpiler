enable_testing()

add_executable(compilerTest compilerTest.cpp)

target_include_directories(compilerTest PUBLIC ../ ../../code ../../lexer ../../parser)

target_link_libraries(compilerTest compiler code lexer parser spdlog::spdlog GTest::gtest_main)

include(GoogleTest)

gtest_discover_tests(compilerTest)
