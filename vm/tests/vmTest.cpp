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
bool testBooleanObject(bool expected, Object *actual);

std::unique_ptr<Program> parse(const std::string &input) {
  Lexer lexer{input};
  Parser parser{&lexer};
  return parser.parseProgram();
}

template <typename T>
bool testExpectedObject(const T &expected, Object *actual) {
  if constexpr (std::is_same_v<int, T>) {
    return testIntegerObject(expected, actual);
  } else if constexpr (std::is_same_v<bool, T>) {
    return testBooleanObject(expected, actual);
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
    spdlog::error("object has wrong value. want={}, got={}", expected, integer->value);
    return false;
  }

  return true;
}

bool testBooleanObject(bool expected, Object *actual) {
  auto boolean = dynamic_cast<const Boolean *>(actual);
  if (boolean == nullptr) {
    spdlog::error("object is not Boolean. got={}", actual->type());
    return false;
  }

  if (boolean->value != expected) {
    spdlog::error("object has wrong value. want={}, got={}", expected, boolean->value);
    return false;
  }

  return true;
}

TEST(VM, TestIntegerArithmetic) {
  std::vector<vmTestCase<int>> tests{
      {"1", 1},
      {"2", 2},
      {"1 + 2", 3},
      {"1 - 2", -1},
      {"1 * 2", 2},
      {"4 / 2", 2},
      {"50 / 2 * 2 + 10 - 5", 55},
      {"5 + 5 + 5 + 5 - 10", 10},
      {"2 * 2 * 2 * 2 * 2", 32},
      {"5 * 2 + 10", 20},
      {"5 + 2 * 10", 25},
      {"5 * (2 + 10)", 60},
  };

  for (auto &&test : tests) {
    auto program = parse(test.input);

    Compiler compiler;
    compiler.compile(program.get());

    VM vm{std::move(compiler.getBytecode().constants), std::move(compiler.getBytecode().instructions)};
    vm.run();

    auto stackElem = vm.lastPoppedStackElem();

    EXPECT_TRUE(testExpectedObject(test.expected, stackElem.get()));
  }
}

TEST(VM, TestBooleanExpressions) {
  std::vector<vmTestCase<bool>> tests{
      {"true", true},
      {"false", false},
      {"1 < 2", true},
      {"1 > 2", false},
      {"1 < 1", false},
      {"1 > 1", false},
      {"1 == 1", true},
      {"1 != 1", false},
      {"1 == 2", false},
      {"1 != 2", true},
      {"true == true", true},
      {"false == false", true},
      {"true == false", false},
      {"true != false", true},
      {"false != true", true},
      {"(1 < 2) == true", true},
      {"(1 < 2) == false", false},
      {"(1 > 2) == true", false},
      {"(1 > 2) == false", true},
  };

  for (auto &&test : tests) {
    auto program = parse(test.input);

    Compiler compiler;
    compiler.compile(program.get());

    VM vm{std::move(compiler.getBytecode().constants), std::move(compiler.getBytecode().instructions)};
    vm.run();

    auto stackElem = vm.lastPoppedStackElem();

    EXPECT_TRUE(testExpectedObject(test.expected, stackElem.get()));
  }
}
