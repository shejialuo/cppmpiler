#include "code.hpp"
#include "spdlog/spdlog.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(Code, TestMake) {
  struct TestData {
    Opcode op;
    std::vector<int> operands;
    Instructions expected;
  };

  std::vector<TestData> tests = {
      {Ops::OpConstant, {65534}, {Ops::OpConstant, std::byte(255), std::byte(254)}},
      {Ops::OpConstant, {20}, {Ops::OpConstant, std::byte(0), std::byte(20)}},
      {Ops::OpAdd, {}, {Ops::OpAdd}},
  };

  for (auto &test : tests) {
    Instructions instructions = Code::make(test.op, test.operands);
    ASSERT_EQ(instructions, test.expected);
  }
}

TEST(Code, TestReadOperands) {
  struct TestData {
    Opcode op;
    std::vector<int> operands;
    int bytesRead;
  };

  std::vector<TestData> tests = {
      {Ops::OpConstant, {65534}, 2},
  };

  for (auto &&test : tests) {
    auto instruction = Code::make(test.op, test.operands);

    auto iter = Code::getDefinition().find(test.op);
    if (iter == Code::getDefinition().end()) {
      spdlog::error("definition not found");
      FAIL();
    }

    auto &&[operands, bytesRead] = Code::readOperands(iter->second, instruction, 1);

    ASSERT_EQ(bytesRead, test.bytesRead);
    ASSERT_EQ(operands, test.operands);
  }
}

TEST(Code, TestInstructionString) {
  std::vector<Instructions> instructions{
      Code::make(Ops::OpAdd, {}),
      Code::make(Ops::OpConstant, {1}),
      Code::make(Ops::OpConstant, {2}),
      Code::make(Ops::OpConstant, {65535}),
  };

  std::string expected{"0000 OpAdd\n"
                       "0001 OpConstant 1\n"
                       "0004 OpConstant 2\n"
                       "0007 OpConstant 65535\n"};

  Instructions instruction{};
  for (auto &&ins : instructions) {
    for (auto &&b : ins) {
      instruction.push_back(b);
    }
  }

  ASSERT_EQ(Code::getString(instruction), expected);
}
