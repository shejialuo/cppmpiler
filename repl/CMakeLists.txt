add_library(repl STATIC repl.cpp)

target_include_directories(repl PRIVATE ../lexer)
target_include_directories(repl PRIVATE ../token)
target_include_directories(repl PRIVATE ../parser)
target_include_directories(repl PRIVATE ../evaluator)
target_include_directories(repl PRIVATE ../compiler)
target_include_directories(repl PRIVATE ../vm)

target_link_libraries(repl lexer token parser evaluator compiler vm)
