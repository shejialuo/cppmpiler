add_library(compiler STATIC compiler.cpp)

target_include_directories(compiler PUBLIC ../code ../object ../ast ../token)

target_link_libraries(compiler code object ast token)

add_subdirectory(./tests)
