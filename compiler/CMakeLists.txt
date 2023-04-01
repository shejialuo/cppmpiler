add_library(compiler STATIC compiler.cpp symbolTable.cpp)

target_include_directories(compiler PUBLIC ../code ../object ../ast ../token)

target_link_libraries(compiler code object ast token spdlog::spdlog)

add_subdirectory(./tests)
