#ifndef _COMPILER_COMPILER_HPP_
#define _COMPILER_COMPILER_HPP_

#include "ast.hpp"
#include "code.hpp"
#include "object.hpp"
#include "symbolTable.hpp"

#include <cstddef>
#include <memory>
#include <vector>

struct Bytecode {
  Instructions instructions;
  std::vector<std::shared_ptr<Object>> constants;

  Bytecode() = default;
  Bytecode(const Bytecode &b) = delete;
  Bytecode(Bytecode &&b) = delete;
};

struct EmittedInstruction {
  Opcode op;
  int position;

  EmittedInstruction() = default;
  EmittedInstruction(const EmittedInstruction &b) = default;
  EmittedInstruction(const Opcode &op_, int p) : op{op_}, position{p} {}
};

struct CompilationScope {
  Instructions instructions{};
  EmittedInstruction lastInstruction{};
  EmittedInstruction previousInstruction{};

  CompilationScope() = default;
};

class Compiler {
private:
  Bytecode bytecode;
  std::shared_ptr<SymbolTable> symbolTable;
  std::vector<CompilationScope> scopes;
  int scopeIndex;

public:
  Compiler() : scopeIndex{0} {
    scopes.push_back(CompilationScope{});
    symbolTable = std::make_shared<SymbolTable>();
  }
  Compiler(std::vector<std::shared_ptr<Object>> &constants, std::shared_ptr<SymbolTable> &table)
      : symbolTable{table}, scopeIndex{0} {
    bytecode.constants = constants;
    scopes.push_back(CompilationScope{});
  }
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
  int addConstant(std::unique_ptr<Object> &object);

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
   * @brief Set the Last Instruction object
   *
   */
  void setLastInstruction(const Opcode &op, int pos);

  /**
   * @brief Return whether the last instruction op is pop
   *
   */
  inline bool lastInstructionIs(const Opcode &op) {
    return currentInstructions().empty() || scopes[scopeIndex].lastInstruction.op == op;
  }

  /**
   * @brief Remove the last pop instruction
   *
   */
  void removeLastPop();

  /**
   * @brief Replace the instruction at pos with instructions
   *
   * @param pos the instruction position
   * @param instructions
   */
  void replaceInstruction(int pos, const Instructions &instruction);

  /**
   * @brief Change the operand at pos with operand
   *
   * @param pos the instruction postion
   * @param operand
   */
  void changeOperand(int pos, int operand);

  /**
   * @brief return current scope's instructions reference
   *
   * @return Instructions&
   */
  inline Instructions &currentInstructions() { return scopes[scopeIndex].instructions; }

  /**
   * @brief Return the current bytecode, and the bytecode
   * in the class should not use anymore.
   *
   * @return Bytecode
   */
  inline Bytecode &getBytecode() {
    bytecode.instructions = currentInstructions();
    return bytecode;
  };

  /**
   * @brief Enter a new scope
   *
   */
  void enterScope();

  /**
   * @brief Leave the current scope
   *
   * @return Instructions
   */
  Instructions leaveScope();

  /**
   * @brief Replace the last pop instruction with return
   *
   */
  void replaceLastPopWithReturn();

  inline int getScopeIndex() { return scopeIndex; }

  inline CompilationScope &currentScope() { return scopes[scopeIndex]; }
};

#endif  // _COMPILER_COMPILER_HPP_
