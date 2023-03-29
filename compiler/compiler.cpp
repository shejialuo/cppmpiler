#include "compiler.hpp"

#include "ast.hpp"
#include "code.hpp"
#include "object.hpp"

#include <memory>

void Compiler::compile(Node *node) {
  Program *program = dynamic_cast<Program *>(node);

  if (program != nullptr) {
    for (auto &&statement : program->statements) {
      compile(statement.get());
    }
  }

  ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(node);
  if (expressionStatement != nullptr) {
    compile(expressionStatement->expression.get());
  }

  InfixExpression *infixExpression = dynamic_cast<InfixExpression *>(node);
  if (infixExpression != nullptr) {
    compile(infixExpression->left.get());
    compile(infixExpression->right.get());

    if (infixExpression->_operator == "+") {
      emit(Ops::OpAdd, {});
    }
  }

  IntegerLiteral *integerLiteral = dynamic_cast<IntegerLiteral *>(node);
  if (integerLiteral != nullptr) {
    auto integer = std::make_unique<Integer>(integerLiteral->value);
    // Here, we push the index for the constant, not the number itself.
    emit(Ops::OpConstant, {addConstant(integer)});
  }
}

int Compiler::addConstant(std::unique_ptr<Integer> &object) {
  bytecode.constants.emplace_back(std::move(object));
  return bytecode.constants.size() - 1;
}

int Compiler::emit(const Opcode &op, const std::vector<int> &operands) {
  auto instruction = Code::make(op, operands);
  return addInstruction(instruction);
}

int Compiler::addInstruction(Instructions &instructions) {
  int posNewInstruction = bytecode.instructions.size();

  for (auto &&instruction : instructions) {
    bytecode.instructions.push_back(instruction);
  }

  return posNewInstruction;
}
