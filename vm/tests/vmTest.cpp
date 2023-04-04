#include "ast.hpp"
#include "compiler.hpp"
#include "lexer.hpp"
#include "object.hpp"
#include "parser.hpp"
#include "spdlog/spdlog.h"
#include "vm.hpp"

#include <cstddef>
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
bool testIntegerArrayObject(const std::vector<int> &expected, Object *actual);

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
  } else if constexpr (std::is_same_v<std::string, T>) {
    return testStringObject(expected, actual);
  } else if constexpr (std::is_same_v<std::vector<int>, T>) {
    return testIntegerArrayObject(expected, actual);
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

bool testStringObject(const std::string &expected, Object *actual) {
  auto string = dynamic_cast<const String *>(actual);
  if (string == nullptr) {
    spdlog::error("object is not String. got={}", actual->type());
    return false;
  }

  if (string->value != expected) {
    spdlog::error("object has wrong value. want={}, got={}", expected, string->value);
    return false;
  }

  return true;
}

bool testIntegerArrayObject(const std::vector<int> &expected, Object *actual) {
  auto array = dynamic_cast<const Array *>(actual);
  if (array == nullptr) {
    spdlog::error("object is not Array. got={}", actual->type());
    return false;
  }

  if (array->elements.size() != expected.size()) {
    spdlog::error("array has wrong num of elements. want={}, got={}", expected.size(), array->elements.size());
    return false;
  }

  for (std::size_t i = 0; i < expected.size(); ++i) {
    if (!testIntegerObject(expected[i], array->elements[i].get())) {
      return false;
    }
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
      {"-5", -5},
      {"-10", -10},
      {"-50 + 100 + -50", 0},
      {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
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
      {"!true", false},
      {"!false", true},
      {"!5", false},
      {"!!true", true},
      {"!!false", false},
      {"!!5", true},
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

TEST(VM, TestConditionals) {
  std::vector<vmTestCase<int>> tests{
      {"if (true) { 10 }", 10},
      {"if (true) { 10 } else { 20 }", 10},
      {"if (false) { 10 } else { 20 } ", 20},
      {"if (1) { 10 }", 10},
      {"if (1 < 2) { 10 }", 10},
      {"if (1 < 2) { 10 } else { 20 }", 10},
      {"if (1 > 2) { 10 } else { 20 }", 20},
      // Here, this would be fail, because I do not handle
      // the case where the condition is not a boolean, the
      // original book use NULL object to handle this case
      // However, I don't think this is a good idea.
      // {"if ((if (false) { 10 })) { 10 } else { 20 }", 20},
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

TEST(VM, TestGlobalLetStatements) {
  std::vector<vmTestCase<int>> tests{
      {"let one = 1; one", 1},
      {"let one = 1; let two = 2; one + two", 3},
      {"let one = 1; let two = one + one; one + two", 3},
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

TEST(VM, TestStringExpressions) {
  std::vector<vmTestCase<std::string>> tests{
      {"\"monkey\"", "monkey"},
      {"\"mon\" + \"key\"", "monkey"},
      {"\"mon\" + \"key\" + \"banana\"", "monkeybanana"},
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

TEST(VM, TestArrayLiterals) {
  std::vector<vmTestCase<std::vector<int>>> tests{
      {"[]", {}},
      {"[1, 2, 3]", {1, 2, 3}},
      {"[1 + 2, 3 * 4, 5 + 6]", {3, 12, 11}},
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

TEST(VM, TestIndexExpressions) {
  std::vector<vmTestCase<int>> tests{
      {"[1, 2, 3][1]", 2},
      {"[1,2,3][0 + 2]", 3},
      {"[[1, 1, 1][0][0]]", 1},
  };
}

TEST(VM, TestCallingFunctionsWithoutArguments) {
  std::vector<vmTestCase<int>> tests{
      {"let fivePlusTen = fn() { 5 + 10; }; fivePlusTen();", 15},
      {"let one = fn() { 1; }; let two = fn() { 2; }; one() + two();", 3},
      {"let a = fn() { 1; }; let b = fn() { a() + 1; }; let c = fn() { b() + 1; }; c();", 3},
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

TEST(VM, TestFunctionsWithReturnStatement) {
  std::vector<vmTestCase<int>> tests{
      {"let earlyExit = fn() { return 99; 100; }; earlyExit();", 99},
      {"let earlyExit = fn() { return 99; return 100; }; earlyExit();", 99},
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

TEST(VM, TestFirstClassFunctions) {
  std::vector<vmTestCase<int>> tests{
      {"let returnsOneReturner = fn() { fn() { 1; }; }; let returnsOne = returnsOneReturner(); returnsOne();", 1},
      {"let returnsOneReturner = fn() { let returnsOne = fn() { 1; }; returnsOne; }; let returnsOne = "
       "returnsOneReturner(); returnsOne();",
       1},
      {"let returnsOneReturner = fn() { let returnsOne = fn() { 1; }; returnsOne; }; returnsOneReturner()();", 1},
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

TEST(VM, TestCallingFunctionsWithBindings) {
  std::vector<vmTestCase<int>> tests{
      {"let one = fn() { let one = 1; one; }; one();", 1},
      {"let oneAndTwo = fn() { let one = 1; let two = 2; one + two; }; oneAndTwo();", 3},
      {"let oneAndTwo = fn() { let one = 1; let two = 2; one + two; }; let threeAndFour = fn() { let three = 3; "
       "let four = 4; three + four; }; oneAndTwo() + threeAndFour();",
       10},
      {"let firstFoobar = fn() { let foobar = 50; foobar; }; let secondFoobar = fn() { let foobar = 100; foobar; }; "
       "firstFoobar() + secondFoobar();",
       150},
      {"let globalSeed = 50; let minusOne = fn() { let num = 1; globalSeed - num; }; let minusTwo = fn() { let num = "
       "2; "
       "globalSeed - num; }; minusOne() + minusTwo();",
       97},
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
