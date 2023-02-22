add_library(evaluator STATIC evaluator.cpp)

target_include_directories(evaluator PUBLIC ../ast ../object)

target_link_libraries(evaluator ast object)

add_subdirectory(./tests)
