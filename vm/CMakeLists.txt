add_library(vm STATIC vm.cpp)

target_include_directories(vm PUBLIC ../compiler ../code ../object ../ast ../token)

target_link_libraries(vm compiler code object ast token spdlog::spdlog)

add_subdirectory(./tests)
