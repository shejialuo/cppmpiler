#include "evaluator.hpp"
#include "lexer.hpp"
#include "object.hpp"
#include "parser.hpp"
#include "spdlog/spdlog.h"

#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

std::unique_ptr<Object> testEval(const std::string &input);
bool testIntegerObject(Object *object, int64_t expected);
bool testBooleanObject(Object *object, bool expected);

std::unique_ptr<Object> testEval(const std::string &input) {
  Lexer lexer{input};
  Parser parser{&lexer};

  auto program = parser.parseProgram();

  return std::move(eval(program.get()));
}

bool testIntegerObject(Object *object, int64_t expected) {
  Integer *integer = dynamic_cast<Integer *>(object);

  if (integer == nullptr) {
    spdlog::error("object is not Integer");
    return false;
  }

  if (integer->value != expected) {
    spdlog::error("object has wrong value. got={}, want={}", integer->value, expected);
    return false;
  }

  return true;
}

bool testBooleanObject(Object *object, bool expected) {
  Boolean *boolean = dynamic_cast<Boolean *>(object);

  if (boolean == nullptr) {
    spdlog::error("object is not Boolean");
    return false;
  }

  if (boolean->value != expected) {
    spdlog::error("object has wrong value. got={}, want={}", boolean->value, expected);
    return false;
  }

  return true;
}

TEST(Evaluator, TestEvalIntegerExpression) {
  struct TestData {
    std::string input;
    int64_t expected;

    TestData(const std::string &s, int64_t v) : input{s}, expected{v} {}
  };

  std::vector<TestData> tests{
      {"5", 5},
      {"10", 10},
      {"-5", -5},
      {"-10", -10},
  };

  for (auto &&test : tests) {
    auto evaluated = testEval(test.input);
    if (!testIntegerObject(evaluated.get(), test.expected)) {
      FAIL();
    }
  }
}

TEST(Evaluator, TestEvalBooleanExpression) {
  struct TestData {
    std::string input;
    bool expected;

    TestData(const std::string &s, bool v) : input{s}, expected{v} {}
  };

  std::vector<TestData> tests{
      {"true", true},
      {"false", false},
  };

  for (auto &&test : tests) {
    auto evaluated = testEval(test.input);
    if (!testBooleanObject(evaluated.get(), test.expected)) {
      FAIL();
    }
  }
}

TEST(Evaluator, TestBangOperator) {
  struct TestData {
    std::string input;
    bool expected;

    TestData(const std::string &s, bool v) : input{s}, expected{v} {}
  };

  std::vector<TestData> tests{
      {"!true", false},
      {"!false", true},
      {"!5", false},
      {"!!true", true},
      {"!!false", false},
      {"!!5", true},
  };

  for (auto &&test : tests) {
    auto evaluated = testEval(test.input);
    if (!testBooleanObject(evaluated.get(), test.expected)) {
      FAIL();
    }
  }
}
