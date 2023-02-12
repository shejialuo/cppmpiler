cmake_minimum_required(VERSION 3.14)

project(simpleInterpreter LANGUAGES CXX)

# GoogleTest requires are least C++14
set(CMAKE_CXX_STANDARD 17)

# Here, we must include the CTest, because we
# call `enabletesting` internally.
# See https://stackoverflow.com/questions/13550153/no-tests-found-when-using-gtest-with-cmake-ctest
include(CTest)

# Here we use CMake `FetchContent` module to download
# the dependency to the build system
# See https://cmake.org/cmake/help/latest/module/FetchContent.html
# And the code snippet is copied from the GTest docs.
# See https://google.github.io/googletest/quickstart-cmake.html
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

FetchContent_MakeAvailable(googletest)

# See https://cmake.org/cmake/help/latest/module/FindDoxygen.html
find_package(Doxygen
  REQUIRED dot
  OPTIONAL_COMPONENTS mscgen dia)

if(DOXYGEN_FOUND)
  set(DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/docs)

  # Here we must make Doxygen do not generate
  # the docs for dependency
  # See https://cmake.org/cmake/help/latest/module/FindDoxygen.html#variable:DOXYGEN_EXCLUDE_PATTERNS
  set(DOXYGEN_EXCLUDE_PATTERNS "*/_deps/*" "*/deps/*")
  doxygen_add_docs(DOXYGEN ${PROJECT_SOURCE_DIR})
  set(DOXYGEN_COLLABORATION_GRAPH YES)
  set(DOXYGEN_EXTRACT_ALL YES)
  set(DOXYGEN_CLASS_DIAGRAMS YES)
  set(DOXYGEN_HIDE_UNDOC_RELATIONS NO)
  set(DOXYGEN_HAVE_DOT YES)
  set(DOXYGEN_CLASS_GRAPH YES)
  set(DOXYGEN_CALL_GRAPH YES)
  set(DOXYGEN_CALLER_GRAPH YES)
  set(DOXYGEN_COLLABORATION_GRAPH YES)
  set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
  set(DOXYGEN_EXTRACT_PRIVATE YES)
  set(DOXYGEN_EXTRACT_PACKAGE YES)
  set(DOXYGEN_EXTRACT_STATIC YES)
  set(DOXYGEN_EXTRACT_LOCALMETHODS YES)
  set(DOXYGEN_UML_LOOK YES)
  set(DOXYGEN_UML_LIMIT_NUM_FIELDS 50)
  set(DOXYGEN_TEMPLATE_RELATIONS YES)
  set(DOXYGEN_DOT_GRAPH_MAX_NODES 100)
  set(DOXYGEN_MAX_DOT_GRAPH_DEPTH 0)
  set(DOXYGEN_DOT_TRANSPARENT YES)
else()
  message("Doxygen need to be installed to generate the doxygen documentation")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(./deps/spdlog)

add_subdirectory(./token)
add_subdirectory(./lexer)
add_subdirectory(./repl)

add_executable(simpleInterpreter main.cpp)
target_include_directories(simpleInterpreter PRIVATE ./repl)
target_link_libraries(simpleInterpreter repl)

set_target_properties(simpleInterpreter PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/)
