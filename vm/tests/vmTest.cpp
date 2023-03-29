#include "ast.hpp"
#include "compiler.hpp"
#include "lexer.hpp"
#include "object.hpp"
#include "parser.hpp"
#include "spdlog/spdlog.h"
#include "vm.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>

template <typename T>
struct vmTestCase {
  std::string input;
  T expected;

  vmTestCase(const std::string &i, const T &e) : input{i}, expected{e} {}
};

template <typename T>
bool testExpectedObject(const T &expected, Object *actual);

std::unique_ptr<Program> parse(const std::string &input);
bool testIntegerObject(int expected, Object *actual);

std::unique_ptr<Program> parse(const std::string &input) {
  Lexer lexer{input};
  Parser parser{&lexer};
  return parser.parseProgram();
}

template <typename T>
bool testExpectedObject(const T &expected, Object *actual) {
  if constexpr (std::is_same_v<int, T>) {
    return testIntegerObject(expected, actual);
  }
  return false;
}

bool testIntegerObject(int expected, Object *actual) {
  auto integer = dynamic_cast<const Integer *>(actual);
  if (integer == nullptr) {
    spdlog::error("object is not Integer. got={}", actual->type());
    return false;
  }

  if (integer->value != expected) {
    spdlog::error("object has wrong value. want={}\ngot={}\n", expected, integer->value);
    return false;
  }

  return true;
}

TEST(VM, TestIntegerArithmetic) {
  std::vector<vmTestCase<int>> tests{
      {"1", 1},
      {"2", 2},
      {"1;2;3", 3},

  };

  for (auto &&test : tests) {
    auto program = parse(test.input);

    Compiler compiler;
    compiler.compile(program.get());

    VM vm{std::move(compiler.getBytecode().constants), std::move(compiler.getBytecode().instructions)};
    vm.run();

    auto stackElem = vm.stackTop();

    testExpectedObject(test.expected, stackElem);
  }
}
