enable_testing()

add_executable(
  parserTest
  parserTest.cpp
)

target_include_directories(parserTest PRIVATE ../ ../../lexer)

target_link_libraries(
  parserTest
  lexer
  parser
  spdlog::spdlog
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(parserTest)
