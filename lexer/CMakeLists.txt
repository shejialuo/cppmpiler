add_library(lexer STATIC lexer.cpp)

target_include_directories(lexer PUBLIC ../token)

add_subdirectory(./tests)
