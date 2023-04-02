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
#include <utility>
#include <variant>
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
bool testInstructions(std::vector<Instructions> &expected, const Instructions &actual);
bool testIntegerObject(int expected, Object *actual);
bool testStringObject(const std::string &expected, Object *actual);

template <typename T>
bool testConstants(std::vector<T> &expected, std::vector<std::shared_ptr<Object>> &actual);

template <typename T>
bool testConstants(std::vector<T> &expected, std::vector<std::shared_ptr<Object>> &actual) {
  if (actual.size() != expected.size()) {
    spdlog::error("wrong number of constants. got={}, want={}", actual.size(), expected.size());
    return false;
  }

  for (size_t i = 0; i < actual.size(); i++) {
    if constexpr (std::is_same_v<int, T>) {
      if (!testIntegerObject(expected[i], actual[i].get())) {
        return false;
      }
    } else if constexpr (std::is_same_v<std::string, T>) {
      if (!testStringObject(expected[i], actual[i].get())) {
        return false;
      }
    } else if constexpr (std::is_same_v<std::variant<int, std::vector<Instructions>>, T>) {
      if (std::holds_alternative<int>(expected[i])) {
        if (!testIntegerObject(std::get<int>(expected[i]), actual[i].get())) {
          return false;
        }
      } else if (std::holds_alternative<std::vector<Instructions>>(expected[i])) {
        CompiledFunction *compiledFunction = dynamic_cast<CompiledFunction *>(actual[i].get());
        if (compiledFunction == nullptr) {
          spdlog::error("object is not CompiledFunction. got={}", actual[i]->type());
          return false;
        }

        auto &instructions = std::get<std::vector<Instructions>>(expected[i]);

        if (!testInstructions(instructions, compiledFunction->instructions)) {
          return false;
        }
      }
    } else {
      spdlog::error("unsupported type for testConstants");
      return false;
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

bool testStringObject(const std::string &expected, Object *actual) {
  auto str = dynamic_cast<const String *>(actual);
  if (str == nullptr) {
    spdlog::error("object is not String. got={}", actual->type());
    return false;
  }

  if (str->value != expected) {
    spdlog::error("object has wrong value. want={}, got={}", expected, str->value);
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
      {
          "-1",
          {1},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpMinus, {}),
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

TEST(Compiler, TestBooleanExpressions) {
  std::vector<CompilerTestCase<int>> tests{
      {
          "true",
          {},
          {
              Code::make(Ops::OpTrue, {}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "false",
          {},
          {
              Code::make(Ops::OpFalse, {}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "1 > 2",
          {1, 2},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpGreaterThan, {}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "1 < 2",
          {2, 1},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpGreaterThan, {}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "1 == 2",
          {1, 2},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpEqual, {}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "1 != 2",
          {1, 2},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpNotEqual, {}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "true == false",
          {},
          {
              Code::make(Ops::OpTrue, {}),
              Code::make(Ops::OpFalse, {}),
              Code::make(Ops::OpEqual, {}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "true != false",
          {},
          {
              Code::make(Ops::OpTrue, {}),
              Code::make(Ops::OpFalse, {}),
              Code::make(Ops::OpNotEqual, {}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "!true",
          {},
          {
              Code::make(Ops::OpTrue, {}),
              Code::make(Ops::OpBang, {}),
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

TEST(Compiler, TestConditionals) {
  std::vector<CompilerTestCase<int>> tests{
      {
          "if (true) { 10 }; 3333;",
          {10, 3333},
          {
              // 0000
              Code::make(Ops::OpTrue, {}),
              // 0001
              Code::make(Ops::OpJumpNotTruthy, {8}),
              // 0004
              Code::make(Ops::OpConstant, {0}),
              // 0007
              Code::make(Ops::OpPop, {}),
              // 0008
              Code::make(Ops::OpConstant, {1}),
              // 0011
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "if (true) { 10 } else { 20 }; 3333;",
          {10, 20, 3333},
          {
              // 0000
              Code::make(Ops::OpTrue, {}),
              // 0001
              Code::make(Ops::OpJumpNotTruthy, {10}),
              // 0004
              Code::make(Ops::OpConstant, {0}),
              // 0007
              Code::make(Ops::OpJump, {13}),
              // 0010
              Code::make(Ops::OpConstant, {1}),
              // 0013
              Code::make(Ops::OpPop, {}),
              // 0014
              Code::make(Ops::OpConstant, {2}),
              // 0017
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

TEST(Compiler, TestGlobalLetStatements) {
  std::vector<CompilerTestCase<int>> tests{
      {
          "let one = 1; let two = 2;",
          {1, 2},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpSetGlobal, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpSetGlobal, {1}),
          },
      },
      {
          "let one = 1; one;",
          {1},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpSetGlobal, {0}),
              Code::make(Ops::OpGetGlobal, {0}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "let one = 1; let two = one; two;",
          {1},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpSetGlobal, {0}),
              Code::make(Ops::OpGetGlobal, {0}),
              Code::make(Ops::OpSetGlobal, {1}),
              Code::make(Ops::OpGetGlobal, {1}),
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

TEST(Compiler, TestStringExpressions) {
  std::vector<CompilerTestCase<std::string>> tests{
      {
          R"("monkey")",
          {"monkey"},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          R"("mon" + "key")",
          {"mon", "key"},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpAdd, {}),
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
    EXPECT_TRUE(testConstants<std::string>(test.expectedConstants, compiler.getBytecode().constants));
  }
}

TEST(Compiler, TestArrayLiterals) {
  std::vector<CompilerTestCase<int>> tests{
      {
          "[]",
          {},
          {
              Code::make(Ops::OpArray, {0}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "[1, 2, 3]",
          {1, 2, 3},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpConstant, {2}),
              Code::make(Ops::OpArray, {3}),
              Code::make(Ops::OpPop, {}),
          },
      },
      {
          "[1 + 2, 3 - 4, 5 * 6]",
          {1, 2, 3, 4, 5, 6},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpAdd, {}),
              Code::make(Ops::OpConstant, {2}),
              Code::make(Ops::OpConstant, {3}),
              Code::make(Ops::OpSub, {}),
              Code::make(Ops::OpConstant, {4}),
              Code::make(Ops::OpConstant, {5}),
              Code::make(Ops::OpMul, {}),
              Code::make(Ops::OpArray, {3}),
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

TEST(Compiler, TestIndexExpressions) {
  std::vector<CompilerTestCase<int>> tests{
      {
          "[1, 2, 3][1 + 1]",
          {1, 2, 3, 1, 1},
          {
              Code::make(Ops::OpConstant, {0}),
              Code::make(Ops::OpConstant, {1}),
              Code::make(Ops::OpConstant, {2}),
              Code::make(Ops::OpArray, {3}),
              Code::make(Ops::OpConstant, {3}),
              Code::make(Ops::OpConstant, {4}),
              Code::make(Ops::OpAdd, {}),
              Code::make(Ops::OpIndex, {}),
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

TEST(Compiler, TestFunctions) {
  std::vector<CompilerTestCase<std::variant<int, std::vector<Instructions>>>> tests{
      {
          "fn() { return 5 + 10 }",
          {
              5,
              10,
              std::variant<int, std::vector<Instructions>>{
                  std::in_place_index<1>,
                  {
                      Code::make(Ops::OpConstant, {0}),
                      Code::make(Ops::OpConstant, {1}),
                      Code::make(Ops::OpAdd, {}),
                      Code::make(Ops::OpReturnValue, {}),
                  },
              },
          },
          {
              Code::make(Ops::OpConstant, {2}),
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

TEST(Compiler, TestCompilerScopes) {
  Compiler compiler{};

  EXPECT_EQ(0, compiler.getScopeIndex());

  compiler.emit(Ops::OpMul, {});

  compiler.enterScope();
  EXPECT_EQ(1, compiler.getScopeIndex());

  compiler.emit(Ops::OpSub, {});
  EXPECT_EQ(1, compiler.currentInstructions().size());
  EXPECT_EQ(Ops::OpSub, compiler.currentScope().lastInstruction.op);

  Instructions instructions = compiler.leaveScope();
  EXPECT_EQ(1, instructions.size());

  EXPECT_EQ(0, compiler.getScopeIndex());

  compiler.emit(Ops::OpAdd, {});
  EXPECT_EQ(2, compiler.currentInstructions().size());
  EXPECT_EQ(Ops::OpAdd, compiler.currentScope().lastInstruction.op);
  EXPECT_EQ(Ops::OpMul, compiler.currentScope().previousInstruction.op);
}
