#include "code.hpp"

#include <cstdio>
#include <numeric>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

const Opcode Ops::OpConstant{0};

const std::unordered_map<Opcode, Definition> Code::definitions{
    // For OpConstant, we store the index not the number itself
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

std::pair<std::vector<int>, int> Code::readOperands(const Definition &definition,
                                                    const Instructions &instructions,
                                                    int curPos) {
  std::vector<int> operands;

  int offset = 0;

  for (int width : definition.operandWidths) {
    int operand = 0;

    for (int i = 0; i < width; i++) {
      // Here, we should restore the operand. For example, if the operand is
      // <255, 254>, we should restore it to 65534.
      operand = (operand << 8) | static_cast<int>(instructions[curPos + offset + i]);
    }

    operands.push_back(operand);
    offset += width;
  }

  return {operands, offset};
}

std::string Code::getString(Instructions &instruction) {
  std::string result{};

  int curPos = 0;

  while (curPos < instruction.size()) {
    auto iter = definitions.find(instruction[curPos]);

    if (iter == definitions.cend()) {
      result += "Error: unknown operator\n";
      break;
    }

    const Definition &definition = iter->second;

    // We need to skip the opcode, so use curPos + 1
    auto &&[operands, bytesRead] = readOperands(definition, instruction, curPos + 1);

    int operandCount = definition.operandWidths.size();

    std::string info(5, ' ');
    sprintf(info.data(), "%04d ", curPos);

    if (operands.size() != operandCount) {
      info += "ERROR: operand length " + std::to_string(operands.size()) + " does not match defined " +
              std::to_string(operandCount) = "";
    } else if (operandCount == 1) {
      info += definition.name + " " + std::to_string(operands[0]);
    } else {
      info += "ERROR: unhandled operandCount for" + definition.name;
    }

    info += "\n";
    result += info;

    curPos += bytesRead + 1;
  }

  return result;
}
