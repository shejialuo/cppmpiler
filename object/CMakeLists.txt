add_library(object STATIC object.cpp environment.cpp)

target_include_directories(object PUBLIC ../ast)

target_link_libraries(object ast)
