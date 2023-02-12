enable_testing()

add_executable(
  lexerTest
  lexerTest.cpp
)

target_include_directories(lexerTest PRIVATE ../)

target_link_libraries(
  lexerTest
  lexer
  token
  spdlog::spdlog
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(lexerTest)
