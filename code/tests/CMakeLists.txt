enable_testing()

add_executable(codeTest codeTest.cpp)

target_include_directories(codeTest PUBLIC ../)

target_link_libraries(codeTest code spdlog::spdlog GTest::gtest_main)

include(GoogleTest)

gtest_discover_tests(codeTest)
