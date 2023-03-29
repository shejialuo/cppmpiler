#ifndef _COMPILER_COMPILER_HPP_
#define _COMPILER_COMPILER_HPP_

#include "ast.hpp"
#include "code.hpp"
#include "object.hpp"

#include <cstddef>
#include <memory>
#include <vector>

struct Bytecode {
  Instructions instructions;
  std::vector<std::unique_ptr<Object>> constants;

  Bytecode() = default;
  Bytecode(const Bytecode &b) = delete;
  Bytecode(Bytecode &&b) = delete;
};

class Compiler {
private:
  Bytecode bytecode;

public:
  Compiler() = default;
  Compiler(const Bytecode &b) = delete;

  /**
   * @brief compile the statement recursively one by one.
   *
   * @param node
   */
  void compile(Node *node);

  /**
   * @brief add the constant operation, return the instruction
   * length
   *
   * @param object
   * @return int
   */
  int addConstant(std::unique_ptr<Integer> &object);

  /**
   * @brief emit the instruction, return the instruction length
   *
   * @param op
   * @param operands
   * @return int
   */
  int emit(const Opcode &op, const std::vector<int> &operands);

  /**
   * @brief add the instruction, return the instruction length
   *
   * @param instructions
   * @return int
   */
  int addInstruction(Instructions &instructions);

  /**
   * @brief Return the current bytecode, and the bytecode
   * in the class should not use anymore.
   *
   * @return Bytecode
   */
  inline Bytecode &getBytecode() { return bytecode; };
};

#endif  // _COMPILER_COMPILER_HPP_
