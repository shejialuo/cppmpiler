#include "ast.hpp"
#include "code.hpp"
#include "compiler.hpp"
#include "lexer.hpp"
#include "object.hpp"
#include "parser.hpp"
#include "spdlog/spdlog.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

template <typename T>
struct CompilerTestCase {
  std::string input;
  std::vector<T> expectedConstants;
  std::vector<Instructions> expectedInstructions;

  CompilerTestCase(const std::string &input,
                   const std::vector<T> &expectedConstants,
                   const std::vector<Instructions> &expectedInstructions)
      : input{input}, expectedConstants{expectedConstants}, expectedInstructions{expectedInstructions} {}
};

std::unique_ptr<Program> parse(const std::string &input);
Instructions concatInstructions(std::vector<Instructions> &instructions);
bool testInstructions(const Instructions &expected, const Instructions &actual);
bool testIntegerObject(int expected, Object *actual);

template <typename T>
bool testConstants(std::vector<T> &expected, std::vector<std::unique_ptr<Object>> &actual);

template <typename T>
bool testConstants(std::vector<T> &expected, std::vector<std::unique_ptr<Object>> &actual) {
  if (actual.size() != expected.size()) {
    spdlog::error("wrong number of constants. got={}, want={}", actual.size(), expected.size());
    return false;
  }

  for (size_t i = 0; i < actual.size(); i++) {
    if constexpr (std::is_same_v<int, T>) {
      if (!testIntegerObject(expected[i], actual[i].get())) {
        return false;
      }
    }
  }

  return true;
}

std::unique_ptr<Program> parse(const std::string &input) {
  Lexer lexer{input};
  Parser parser{&lexer};
  return parser.parseProgram();
}

Instructions concatInstructions(std::vector<Instructions> &instructions) {
  Instructions result;
  for (auto &ins : instructions) {
    result.insert(result.end(), ins.begin(), ins.end());
  }
  return result;
}

bool testInstructions(std::vector<Instructions> &expected, const Instructions &actual) {
  auto expectedInstructions = concatInstructions(expected);

  if (actual.size() != expectedInstructions.size()) {
    spdlog::error("wrong instructions length. want={}, got={}", expectedInstructions.size(), actual.size());
    return false;
  }

  for (size_t i = 0; i < actual.size(); i++) {
    if (actual[i] != expectedInstructions[i]) {
      spdlog::error("wrong instruction at {}. want={}, got={}", i, expectedInstructions[i], actual[i]);
      return false;
    }
  }

  return true;
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

TEST(Compiler, TestIntegerArithmetic) {
  std::vector<CompilerTestCase<int>> tests{
      {
          "1 + 2",
          {1, 2},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpAdd, {}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "1; 2",
          {1, 2},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpPop, {}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "1 - 2",
          {1, 2},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpSub, {}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "1 * 2",
          {1, 2},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpMul, {}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          " 2 / 1",
          {2, 1},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpDiv, {}),
              Code::make(Ops::OpPop, {}),
          },
      },
  };

  for (auto &&test : tests) {
    auto program = parse(test.input);
    Compiler compiler;
    compiler.compile(program.get());
    auto instructions = compiler.getBytecode().instructions;
    EXPECT_TRUE(testInstructions(test.expectedInstructions, instructions));
    EXPECT_TRUE(testConstants(test.expectedConstants, compiler.getBytecode().constants));
  }
}
