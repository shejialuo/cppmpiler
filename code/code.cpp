#include "code.hpp"

#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

const Opcode Ops::OpConstant{0};

const std::unordered_map<Opcode, Definition> Code::definitions{
    {
        Ops::OpConstant,
        Definition{"OpConstant", {2}},
    },
};

Instructions Code::make(const Opcode &op, const std::vector<int> &operands) {
  auto iter = definitions.find(op);

  if (iter == definitions.end()) {
    return {};
  }

  const Definition &definition = iter->second;

  int totalLength = std::accumulate(definition.operandWidths.begin(), definition.operandWidths.end(), 1);

  Instructions instructions(totalLength);

  instructions[0] = op;

  // The first position 0 is for the opcode, So we start from 1
  int curPos = 1;

  for (size_t i = 0; i < operands.size(); i++) {
    int operand = operands[i];
    int width = definition.operandWidths[i];

    for (int j = 0; j < width; j++) {
      // Here, we should put the number in the right order
      // For example, if the operand is 65534, we should put 255 in the first
      // position and 254 in the second position
      instructions[curPos + width - j - 1] = (std::byte(operand & 0xff));
      operand >>= 8;
    }
    curPos += width;
  }

  return instructions;
}
