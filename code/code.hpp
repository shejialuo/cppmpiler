#ifndef _CODE_CODE_HPP
#define _CODE_CODE_HPP

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * @brief Definition struct holds the name of the instruction and the width of
 * the operands
 *
 */
struct Definition {
  std::string name;
  std::vector<int> operandWidths;

  Definition() = default;
  Definition(const std::string &n, const std::vector<int> &o) : name{n}, operandWidths{o} {}
};

// type for holding many bytes
using Instructions = std::vector<std::byte>;
// Opcode type only holding one bytes
using Opcode = std::byte;

/**
 * @brief The wrapper class for define different
 * operators
 *
 */
struct Ops {
  static const Opcode OpConstant;
  static const Opcode OpAdd;
  static const Opcode OpPop;
  static const Opcode OpSub;
  static const Opcode OpMul;
  static const Opcode OpDiv;
  static const Opcode OpTrue;
  static const Opcode OpFalse;
  static const Opcode OpEqual;
  static const Opcode OpNotEqual;
  static const Opcode OpGreaterThan;
  static const Opcode OpMinus;
  static const Opcode OpBang;
  static const Opcode OpJumpNotTruthy;
  static const Opcode OpJump;
  static const Opcode OpGetGlobal;
  static const Opcode OpSetGlobal;
  static const Opcode OpArray;
  static const Opcode OpIndex;
  static const Opcode OpCall;
  static const Opcode OpReturnValue;
  static const Opcode OpReturn;
  static const Opcode OpGetLocal;
  static const Opcode OpSetLocal;
};

/**
 * @brief Code class is a just wrapper
 *
 */
class Code {
private:
  static const std::unordered_map<Opcode, Definition> definitions;

public:
  /**
   * @brief Make a new instruction
   *
   * @param op the operator
   * @param operands the operands
   */
  static Instructions make(const Opcode &op, const std::vector<int> &operands);

  /**
   * @brief reverse operation of the `Make`. Get the operands and the bytes read
   *
   * @param definition
   * @param instructions the instructions which holds operators and operands
   * @param curPos the current position
   * @return std::pair<std::vector<int>, int>
   */
  static std::pair<std::vector<int>, int> readOperands(const Definition &definition,
                                                       const Instructions &instructions,
                                                       int curPos);

  /**
   * @brief Get the string representation of the instruction
   *
   */
  static std::string getString(Instructions &instruction);

  /**
   * @brief Helper function get the definition
   *
   */
  static inline const std::unordered_map<Opcode, Definition> &getDefinition() { return definitions; }
};

#endif  // _CODE_CODE_HPP
