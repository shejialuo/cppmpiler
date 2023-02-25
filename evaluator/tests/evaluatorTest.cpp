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

Evaluator evaluator{};

std::shared_ptr<Object> testEval(const std::string &input, std::shared_ptr<Environment> &env);
bool testIntegerObject(Object *object, int64_t expected);
bool testBooleanObject(Object *object, bool expected);
bool testNullObject(Object *object);

std::shared_ptr<Object> testEval(const std::string &input) {
  Lexer lexer{input};
  Parser parser{&lexer};

  auto program = parser.parseProgram();
  auto env = std::make_shared<Environment>();

  return evaluator.eval(program.get(), env);
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

bool testNullObject(Object *object) {
  if (object != nullptr) {
    spdlog::error("object is not nullptr");
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
      {"5 + 5 + 5 + 5 - 10", 10},
      {"2 * 2 * 2 * 2 * 2", 32},
      {"-50 + 100 + -50", 0},
      {"5 * 2 + 10", 20},
      {"5 + 2 * 10", 25},
      {"20 + 2 * -10", 0},
      {"50 / 2 * 2 + 10", 60},
      {"2 * (5 + 10)", 30},
      {"3 * 3 * 3 + 10", 37},
      {"3 * (3 * 3) + 10", 37},
      {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
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

TEST(Evaluator, TestIfElseExpressions1) {
  struct TestData {
    std::string input;
    int64_t expected;

    TestData(const std::string &s, int64_t v) : input{s}, expected{v} {}
  };

  std::vector<TestData> tests{
      {"if (true) { 10 }", 10},
      {"if (1) { 10 }", 10},
      {"if (1 < 2) { 10 }", 10},
      {"if (1 > 2) { 10 } else { 20 }", 20},
      {"if (1 < 2) { 10 } else { 20 }", 10},
  };

  for (auto &&test : tests) {
    auto evaluated = testEval(test.input);
    if (!testIntegerObject(evaluated.get(), test.expected)) {
      FAIL();
    }
  }
}

TEST(Evaluator, TestIfElseExpressions2) {
  struct TestData {
    std::string input;

    TestData(const std::string &s) : input{s} {}
  };

  std::vector<TestData> tests{
      {"if (false) { 10 }"},
      {"if (0) {10}"},
      {"if (1 > 2) {10}"},
      {"if (!1) {10}"},
  };

  for (auto &&test : tests) {
    auto evaluated = testEval(test.input);
    if (!testNullObject(evaluated.get())) {
      FAIL();
    }
  }
}

TEST(Evaluator, TestReturnStatements) {
  struct TestData {
    std::string input;
    int64_t expected;

    TestData(const std::string &s, int64_t v) : input{s}, expected{v} {}
  };

  std::vector<TestData> tests{
      {"return 10;", 10},
      {"return 10; 9;", 10},
      {"return 2 * 5; 9;", 10},
      {"9; return 2 * 5; 9;", 10},
      {"if (10 > 1) { if (10 > 1) {return 10;} return 1;}", 10},
  };

  for (auto &&test : tests) {
    auto evaluated = testEval(test.input);
    if (!testIntegerObject(evaluated.get(), test.expected)) {
      FAIL();
    }
  }
}

TEST(Evaluator, TestErrorHandling) {
  struct TestData {
    std::string input;
    std::string expectedMessage;

    TestData(const std::string &s, const std::string &m) : input{s}, expectedMessage{m} {}
  };

  std::vector<TestData> tests{
      {
          "5 + true;",
          "type mismatch: INTEGER + BOOLEAN",
      },
      {
          "5 + true; 5;",
          "type mismatch: INTEGER + BOOLEAN",
      },
      {
          "-true;",
          "unknown operator: -BOOLEAN",
      },
      {
          "true + false;",
          "unknown operator: BOOLEAN + BOOLEAN",
      },
      {
          "5; true + false; 5",
          "unknown operator: BOOLEAN + BOOLEAN",
      },
      {
          "if (10 > 1) { true + false; }",
          "unknown operator: BOOLEAN + BOOLEAN",
      },
      {
          "if (10 > 1) { if (10 > 1) {return true + false;} return 1;}",
          "unknown operator: BOOLEAN + BOOLEAN",
      },
      {
          "foobar",
          "identifier not found: foobar",
      },
      {
          R"("Hello" - "World")",
          "unknown operator: STRING - STRING",
      },
  };

  for (auto &&test : tests) {
    auto evaluated = testEval(test.input);

    Error *error = dynamic_cast<Error *>(evaluated.get());

    if (error == nullptr) {
      spdlog::error("no error object returned");
      FAIL();
    }

    if (error->message != test.expectedMessage) {
      spdlog::error("wrong error message. expected='{}', got='{}'", test.expectedMessage, error->message);
      FAIL();
    }
  }
}

TEST(Evaluator, TestLetStatements) {
  struct TestData {
    std::string input;
    int64_t expected;

    TestData(const std::string &s, int64_t v) : input{s}, expected{v} {}
  };

  std::vector<TestData> tests{
      {"let a = 5; a;", 5},
      {"let a = 5 * 5; a;", 25},
      {"let a = 5; let b = a; b;", 5},
      {"let a = 5; let b = a; let c = a + b + 5; c;", 15},
  };

  for (auto &&test : tests) {
    if (!testIntegerObject(testEval(test.input).get(), test.expected)) {
      FAIL();
    }
  }
}

TEST(Evaluator, TestFunctionObject) {
  std::string input = "fn(x) { x + 2; };";

  auto evaluated = testEval(input);

  Function *function = dynamic_cast<Function *>(evaluated.get());
  if (function == nullptr) {
    spdlog::error("object is not Function.");
    FAIL();
  }

  if (function->parameters.size() != 1) {
    spdlog::error("function has wrong parameters.");
    FAIL();
  }

  if (function->parameters[0]->getString() != "x") {
    spdlog::error("parameter is not x");
    FAIL();
  }

  if (function->body->getString() != "(x + 2)") {
    spdlog::error("body is not x + 2");
    FAIL();
  }
}

TEST(Evaluator, TestFunctionApplication) {
  struct TestData {
    std::string input;
    int64_t expected;

    TestData(const std::string &s, int64_t v) : input{s}, expected{v} {}
  };

  std::vector<TestData> tests{
      {"let identity = fn(x) { x; }; identity(5);", 5},
      {"let identity = fn(x) { return x; }; identity(5);", 5},
      {"let double = fn(x) { x * 2; }; double(5);", 10},
      {"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
      {"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
  };

  for (auto &&test : tests) {
    if (!testIntegerObject(testEval(test.input).get(), test.expected)) {
      FAIL();
    }
  }
}

TEST(Evaluator, TestClosures) {
  std::string input = "let newAdder = fn(x) { fn(y){x + y}; }; \
                       let addTwo = newAdder(2); \
                       addTwo(2);";
  if (!testIntegerObject(testEval(input).get(), 4)) {
    FAIL();
  }
}

TEST(Evaluator, TestHighOrder1) {
  std::string input = "let add = fn(a, b) {a + b}; \
                       let sub = fn(a, b) {a - b}; \
                       let applyFunc = fn(a, b, func) { func(a, b)}; \
                       applyFunc(2, 2, add)";
  if (!testIntegerObject(testEval(input).get(), 4)) {
    FAIL();
  }
}

TEST(Evaluator, TestHighOrder2) {
  std::string input = "let add = fn(a, b) {a + b}; \
                       let sub = fn(a, b) {a - b}; \
                       let applyFunc = fn(a, b, func) { func(a, b)}; \
                       applyFunc(10, 2, sub)";
  if (!testIntegerObject(testEval(input).get(), 8)) {
    FAIL();
  }
}

TEST(Evaluator, TestStringLiterator) {
  std::string input = R"("Hello World!")";

  auto evaluated = testEval(input);
  String *str = dynamic_cast<String *>(evaluated.get());
  if (str == nullptr) {
    spdlog::error("object is not String");
    FAIL();
  }

  if (str->value != "Hello World!") {
    spdlog::error("String has wrong value. got='{}'", str->value);
    FAIL();
  }
}

TEST(Evaluator, TestStringConcatenation) {
  std::string input = R"("Hello" + " " + "World!")";

  auto evaluated = testEval(input);
  String *str = dynamic_cast<String *>(evaluated.get());
  if (str == nullptr) {
    spdlog::error("object is not String");
    FAIL();
  }

  if (str->value != "Hello World!") {
    spdlog::error("String has wrong value. got='{}'", str->value);
    FAIL();
  }
}

TEST(Evaluator, TestBuiltinLenCorrect) {
  struct TestData {
    std::string input;
    int64_t expected;

    TestData(const std::string &s, int64_t v) : input{s}, expected{v} {}
  };

  std::vector<TestData> tests{
      {R"(len(""))", 0},
      {R"(len("four"))", 4},
      {R"(len("hello world"))", 11},
  };

  for (auto &&test : tests) {
    auto evaluated = testEval(test.input);

    if (!testIntegerObject(evaluated.get(), test.expected)) {
      FAIL();
    }
  }
}

TEST(Evaluator, TestBuiltinLenError) {
  struct TestData {
    std::string input;
    std::string error;

    TestData(const std::string &s, const std::string &e) : input{s}, error{e} {}
  };

  std::vector<TestData> tests{
      {R"(len(1))", "argument to len not supported, got INTEGER"},
      {R"(len("one", "two"))", "wrong number of arguments. got=2, want=1"},
  };

  for (auto &&test : tests) {
    auto evaluated = testEval(test.input);

    Error *err = dynamic_cast<Error *>(evaluated.get());
    if (err == nullptr) {
      spdlog::error("object is not Error");
      FAIL();
    }

    if (err->message != test.error) {
      spdlog::error("wrong error message. expected={}, got={}", test.error, err->message);
      FAIL();
    }
  }
}

TEST(Evaluator, TestArrayLiterals) {
  std::string input{"[1, 2 * 2, 3 + 3]"};

  auto evaluated = testEval(input);

  Array *array = dynamic_cast<Array *>(evaluated.get());
  if (array == nullptr) {
    spdlog::error("object is not Array");
    FAIL();
  }

  if (array->elements.size() != 3) {
    spdlog::error("array has wrong number of elements. got={}", array->elements.size());
    FAIL();
  }

  if (!testIntegerObject(array->elements[0].get(), 1)) {
    FAIL();
  }

  if (!testIntegerObject(array->elements[1].get(), 4)) {
    FAIL();
  }

  if (!testIntegerObject(array->elements[2].get(), 6)) {
    FAIL();
  }
}

TEST(Evaluator, TestArrayIndexExpressions) {
  struct TestData {
    std::string input;
    int64_t expected;

    TestData(const std::string &s, int64_t v) : input{s}, expected{v} {}
  };

  std::vector<TestData> tests{
      {"[1,2,3][0]", 1},
      {"[1,2,3][1]", 2},
      {"[1,2,3][2]", 3},
      {"let i = 0; [1][i];", 1},
      {"[1,2,3][1+1];", 3},
      {"let myArray = [1,2,3]; myArray[2];", 3},
      {"let myArray = [1,2,3]; let i = myArray[0]; myArray[i]", 2},
      {"let myArray = [1,2,3]; myArray[0] + myArray[1] + myArray[2];", 6},
  };

  for (auto &&test : tests) {
    if (!testIntegerObject(testEval(test.input).get(), test.expected)) {
      FAIL();
    }
  }
}
