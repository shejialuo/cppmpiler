#ifndef _CODE_CODE_HPP
#define _CODE_CODE_HPP

#include <cstddef>
#include <string>
#include <unordered_map>
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
};

/**
 * @brief Code class is a just wrapper
 *
 */
class Code {
private:
  static const std::unordered_map<Opcode, Definition> definitions;

public:
  static Instructions make(const Opcode &op, const std::vector<int> &operands);
};

#endif  // _CODE_CODE_HPP
