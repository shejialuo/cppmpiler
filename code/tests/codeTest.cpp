#include "code.hpp"

#include <gtest/gtest.h>
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
  };

  for (auto &test : tests) {
    Instructions instructions = Code::make(test.op, test.operands);
    ASSERT_EQ(instructions, test.expected);
  }
}
