enable_testing()

add_executable(compilerTest compilerTest.cpp)
add_executable(symbolTableTest symbolTableTest.cpp)

target_include_directories(compilerTest PUBLIC ../ ../../code ../../lexer ../../parser)
target_include_directories(symbolTableTest PUBLIC ../)

target_link_libraries(compilerTest compiler code lexer parser spdlog::spdlog GTest::gtest_main)
target_link_libraries(symbolTableTest compiler spdlog::spdlog GTest::gtest_main)

include(GoogleTest)

gtest_discover_tests(compilerTest)
gtest_discover_tests(symbolTableTest)
