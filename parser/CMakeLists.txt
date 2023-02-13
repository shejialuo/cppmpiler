add_library(parser STATIC parser.cpp)

target_include_directories(parser PUBLIC ../token)
target_include_directories(parser PUBLIC ../lexer)
target_include_directories(parser PUBLIC ../ast)

target_link_libraries(parser lexer token ast)

add_subdirectory(./tests)
