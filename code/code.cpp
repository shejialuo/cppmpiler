#include "code.hpp"

#include <cstdio>
#include <numeric>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

const Opcode Ops::OpConstant{0};
const Opcode Ops::OpAdd{1};
const Opcode Ops::OpPop{2};
const Opcode Ops::OpSub{3};
const Opcode Ops::OpMul{4};
const Opcode Ops::OpDiv{5};
const Opcode Ops::OpTrue{6};
const Opcode Ops::OpFalse{7};
const Opcode Ops::OpEqual{8};
const Opcode Ops::OpNotEqual{9};
const Opcode Ops::OpGreaterThan{10};
const Opcode Ops::OpMinus{11};
const Opcode Ops::OpBang{12};
const Opcode Ops::OpJumpNotTruthy{13};
const Opcode Ops::OpJump{14};
const Opcode Ops::OpGetGlobal{15};
const Opcode Ops::OpSetGlobal{16};
const Opcode Ops::OpArray{17};
const Opcode Ops::OpIndex{18};
const Opcode Ops::OpCall{19};
const Opcode Ops::OpReturnValue{20};
const Opcode Ops::OpReturn{21};
const Opcode Ops::OpGetLocal{22};
const Opcode Ops::OpSetLocal{23};
const Opcode Ops::OpGetBuiltin{24};
const Opcode Ops::OpClosure{25};
const Opcode Ops::OpGetFree{26};

const std::unordered_map<Opcode, Definition> Code::definitions{
    // For OpConstant, we store the index not the number itself
    {
        Ops::OpConstant,
        Definition{"OpConstant", {2}},
    },
    {
        Ops::OpAdd,
        Definition{"OpAdd", {}},
    },
    {
        Ops::OpPop,
        Definition{"OpPop", {}},
    },
    {
        Ops::OpSub,
        Definition{"OpSub", {}},
    },
    {
        Ops::OpMul,
        Definition{"OpMul", {}},
    },
    {
        Ops::OpDiv,
        Definition{"OpDiv", {}},
    },
    {
        Ops::OpTrue,
        Definition{"OpTrue", {}},
    },
    {
        Ops::OpFalse,
        Definition{"OpFalse", {}},
    },
    {
        Ops::OpEqual,
        Definition{"OpEqual", {}},
    },
    {
        Ops::OpNotEqual,
        Definition{"OpNotEqual", {}},
    },
    {
        Ops::OpGreaterThan,
        Definition{"OpGreaterThan", {}},
    },
    {
        Ops::OpMinus,
        Definition{"OpMinus", {}},
    },
    {
        Ops::OpBang,
        Definition{"OpBang", {}},
    },
    {
        Ops::OpJumpNotTruthy,
        Definition{"OpJumpNotTruthy", {2}},
    },
    {
        Ops::OpJump,
        Definition{"OpJump", {2}},
    },
    {
        Ops::OpGetGlobal,
        Definition{"OpGetGlobal", {2}},
    },
    {
        Ops::OpSetGlobal,
        Definition{"OpSetGlobal", {2}},
    },
    {
        Ops::OpArray,
        Definition{"OpArray", {2}},
    },
    {
        Ops::OpIndex,
        Definition{"OpIndex", {}},
    },
    {
        Ops::OpCall,
        Definition{"OpCall", {1}},
    },
    {
        Ops::OpReturnValue,
        Definition{"OpReturnValue", {}},
    },
    {
        Ops::OpReturn,
        Definition{"OpReturn", {}},
    },
    {
        Ops::OpGetLocal,
        Definition{"OpGetLocal", {1}},
    },
    {
        Ops::OpSetLocal,
        Definition{"OpSetLocal", {1}},
    },
    {
        Ops::OpGetBuiltin,
        Definition{"OpGetBuiltin", {1}},
    },
    {
        Ops::OpClosure,
        // The first operand is the index of the constant, the second operand is
        // the number of free variables
        Definition{"OpClosure", {2, 1}},
    },
    {
        Ops::OpGetFree,
        Definition{"OpGetFree", {1}},
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
    } else if (operandCount == 0) {
      info += definition.name;
    } else if (operandCount == 1) {
      info += definition.name + " " + std::to_string(operands[0]);
    } else if (operandCount == 2) {
      info += definition.name + " " + std::to_string(operands[0]) + " " + std::to_string(operands[1]);
    } else {
      info += "ERROR: unhandled operandCount for" + definition.name;
    }

    info += "\n";
    result += info;

    curPos += bytesRead + 1;
  }

  return result;
}
