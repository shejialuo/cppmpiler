#include "compiler.hpp"

#include "ast.hpp"
#include "code.hpp"
#include "object.hpp"
#include "spdlog/spdlog.h"
#include "symbolTable.hpp"

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
    emit(Ops::OpPop, {});
  }

  InfixExpression *infixExpression = dynamic_cast<InfixExpression *>(node);
  if (infixExpression != nullptr) {
    // special case
    if (infixExpression->_operator == "<") {
      compile(infixExpression->right.get());
      compile(infixExpression->left.get());
      emit(Ops::OpGreaterThan, {});
      return;
    }

    compile(infixExpression->left.get());
    compile(infixExpression->right.get());

    if (infixExpression->_operator == "+") {
      emit(Ops::OpAdd, {});
    } else if (infixExpression->_operator == "-") {
      emit(Ops::OpSub, {});
    } else if (infixExpression->_operator == "*") {
      emit(Ops::OpMul, {});
    } else if (infixExpression->_operator == "/") {
      emit(Ops::OpDiv, {});
    } else if (infixExpression->_operator == ">") {
      emit(Ops::OpGreaterThan, {});
    } else if (infixExpression->_operator == "==") {
      emit(Ops::OpEqual, {});
    } else if (infixExpression->_operator == "!=") {
      emit(Ops::OpNotEqual, {});
    }
  }

  IntegerLiteral *integerLiteral = dynamic_cast<IntegerLiteral *>(node);
  if (integerLiteral != nullptr) {
    std::unique_ptr<Object> integer = std::make_unique<Integer>(integerLiteral->value);
    // Here, we push the index for the constant, not the number itself.
    emit(Ops::OpConstant, {addConstant(integer)});
  }

  BooleanExpression *booleanExpression = dynamic_cast<BooleanExpression *>(node);
  if (booleanExpression != nullptr) {
    if (booleanExpression->value) {
      emit(Ops::OpTrue, {});
    } else {
      emit(Ops::OpFalse, {});
    }
  }

  PrefixExpression *prefixExpression = dynamic_cast<PrefixExpression *>(node);
  if (prefixExpression != nullptr) {
    compile(prefixExpression->right.get());

    if (prefixExpression->_operator == "!") {
      emit(Ops::OpBang, {});
    } else if (prefixExpression->_operator == "-") {
      emit(Ops::OpMinus, {});
    }
  }

  IfExpression *ifExpression = dynamic_cast<IfExpression *>(node);
  if (ifExpression != nullptr) {
    // For `IfExpression`, we need to handle the length of the
    // `consequence` and `alternative` blocks. So the question
    // is to change the operand of the `OpJumpNotTruthy` instruction.
    // and we will call `changeOperand` to do this.

    compile(ifExpression->condition.get());

    // emit a jump instruction with a placeholder operand
    int jumpNotTruthyPosition = emit(Ops::OpJumpNotTruthy, {9999});

    compile(ifExpression->consequence.get());

    if (lastInstructionIs(Ops::OpPop)) {
      // remove the `OpPop` instruction in a block statement
      removeLastPop();
    }

    // If there is no else branch, we can set the `OpJumpNotTruthy` operand
    if (ifExpression->alternative == nullptr) {
      // get the consequence position
      int afterConsequencePosition = currentInstructions().size();
      //! Pay attention
      // Here, we change the value to be 1, it may seem strange, but
      // it is because we need to jump over the `OpPop` instruction.
      // So we need to add 1 to the position. And I don't think using
      // NULL object is a good idea. We don't need this, why not just
      // jump over the region. Although there may be situations
      // let a = if (false) {10}, we use nullptr to represent it.
      // This is the same as the way I deal with interpreter
      changeOperand(jumpNotTruthyPosition, afterConsequencePosition + 1);
    } else {
      // If there is an alternative branch, we need to emit a `OpJump`
      // instruction to jump over the alternative branch. Actually it is not
      // difficult, it is the same idea.
      int jumpPosition = emit(Ops::OpJump, {9999});

      int afterConsequencePosition = currentInstructions().size();
      changeOperand(jumpNotTruthyPosition, afterConsequencePosition);

      compile(ifExpression->alternative.get());

      if (lastInstructionIs(Ops::OpPop)) {
        removeLastPop();
      }

      int afterAlternativePosition = currentInstructions().size();
      changeOperand(jumpPosition, afterAlternativePosition);
    }
  }

  BlockStatement *blockStatement = dynamic_cast<BlockStatement *>(node);
  if (blockStatement != nullptr) {
    for (auto &&statement : blockStatement->statements) {
      compile(statement.get());
    }
  }

  LetStatement *letStatement = dynamic_cast<LetStatement *>(node);
  if (letStatement != nullptr) {
    compile(letStatement->value.get());

    Symbol &symbol = symbolTable->define(letStatement->name->value);
    emit(Ops::OpSetGlobal, {symbol.index});
  }

  Identifier *identifier = dynamic_cast<Identifier *>(node);
  if (identifier != nullptr) {
    auto symbol = symbolTable->resolve(identifier->value);
    if (!symbol.has_value()) {
      spdlog::error("identifier not found: {}", identifier->value);
      return;
    }

    emit(Ops::OpGetGlobal, {symbol.value().get().index});
  }

  StringLiteral *stringLiteral = dynamic_cast<StringLiteral *>(node);
  if (stringLiteral != nullptr) {
    std::unique_ptr<Object> string = std::make_unique<String>(stringLiteral->value);
    emit(Ops::OpConstant, {addConstant(string)});
  }

  ArrayLiteral *arrayLiteral = dynamic_cast<ArrayLiteral *>(node);
  if (arrayLiteral != nullptr) {
    for (auto &&element : arrayLiteral->elements) {
      compile(element.get());
    }

    emit(Ops::OpArray, {static_cast<int>(arrayLiteral->elements.size())});
  }

  IndexExpression *indexExpression = dynamic_cast<IndexExpression *>(node);
  if (indexExpression != nullptr) {
    compile(indexExpression->left.get());
    compile(indexExpression->index.get());

    emit(Ops::OpIndex, {});
  }

  FunctionLiteral *functionLiteral = dynamic_cast<FunctionLiteral *>(node);
  if (functionLiteral != nullptr) {
    // We should create a new scope for this new function
    enterScope();

    compile(functionLiteral->body.get());

    if (lastInstructionIs(Ops::OpPop)) {
      replaceLastPopWithReturn();
    }

    Instructions instructions = leaveScope();

    std::unique_ptr<Object> compiledFunction = std::make_unique<CompiledFunction>(std::move(instructions));

    emit(Ops::OpConstant, {addConstant(compiledFunction)});
  }

  ReturnStatement *returnStatement = dynamic_cast<ReturnStatement *>(node);
  if (returnStatement != nullptr) {
    compile(returnStatement->returnValue.get());

    emit(Ops::OpReturnValue, {});
  }
}

int Compiler::addConstant(std::unique_ptr<Object> &object) {
  bytecode.constants.emplace_back(std::move(object));
  return bytecode.constants.size() - 1;
}

int Compiler::emit(const Opcode &op, const std::vector<int> &operands) {
  auto instruction = Code::make(op, operands);

  int pos = addInstruction(instruction);
  setLastInstruction(op, pos);

  return pos;
}

int Compiler::addInstruction(Instructions &instructions) {
  int posNewInstruction = currentInstructions().size();

  for (auto &&instruction : instructions) {
    currentInstructions().push_back(instruction);
  }

  return posNewInstruction;
}

void Compiler::setLastInstruction(const Opcode &op, int pos) {
  currentScope().previousInstruction.op = currentScope().lastInstruction.op;
  currentScope().previousInstruction.position = currentScope().lastInstruction.position;

  currentScope().lastInstruction.op = op;
  currentScope().lastInstruction.position = pos;
}

void Compiler::removeLastPop() {
  currentInstructions().erase(currentInstructions().begin() + currentScope().lastInstruction.position);

  currentScope().lastInstruction.op = currentScope().previousInstruction.op;
  currentScope().lastInstruction.position = currentScope().previousInstruction.position;
}

void Compiler::replaceInstruction(int pos, const Instructions &instruction) {
  // The size must be the same
  for (int i = 0; i < instruction.size(); i++) {
    currentInstructions()[pos + i] = instruction[i];
  }
}

void Compiler::changeOperand(int pos, int operand) {
  auto op = Opcode(currentInstructions()[pos]);
  auto newInstruction = Code::make(op, {operand});

  replaceInstruction(pos, newInstruction);
}

void Compiler::enterScope() {
  scopeIndex++;
  scopes.emplace_back(CompilationScope{});
}

Instructions Compiler::leaveScope() {
  Instructions instructions = currentInstructions();

  scopes.pop_back();
  scopeIndex--;

  return instructions;
}

void Compiler::replaceLastPopWithReturn() {
  int lastPosition = currentScope().lastInstruction.position;
  replaceInstruction(lastPosition, Code::make(Ops::OpReturnValue, {}));
  currentScope().lastInstruction.op = Ops::OpReturnValue;
}
