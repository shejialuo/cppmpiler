#include "vm.hpp"

#include "code.hpp"
#include "object.hpp"
#include "spdlog/spdlog.h"

#include <memory>
#include <stdexcept>

constexpr std::string_view INTEGER_OBJ = "INTEGER";
constexpr std::string_view BOOLEAN_OBJ = "BOOLEAN";
constexpr std::string_view RETURN_VALUE_OBJ = "RETURN_VALUE";
constexpr std::string_view ERROR_OBJ = "ERROR";
constexpr std::string_view STRING_OBJ = "STRING";
constexpr std::string_view ARRAY_OBJ = "ARRAY";

Object *VM::stackTop() {
  if (sp == 0) {
    return nullptr;
  }
  return stack[sp - 1].get();
}

void VM::run() {
  for (int ip = 0; ip < instructions.size(); ip++) {
    auto op = Opcode(instructions[ip]);
    if (op == Ops::OpConstant) {
      // Restore the index value here
      int index = (int(instructions[ip + 1]) << 8) | int(instructions[ip + 2]);
      ip += 2;

      push(constants[index]);
    } else if (op == Ops::OpAdd || op == Ops::OpSub || op == Ops::OpMul || op == Ops::OpDiv) {
      executeBinaryOperation(op);
    } else if (op == Ops::OpPop) {
      lastPopped = pop();
    } else if (op == Ops::OpTrue) {
      std::unique_ptr<Object> object = std::make_unique<Boolean>(true);
      push(object);
    } else if (op == Ops::OpFalse) {
      std::unique_ptr<Object> object = std::make_unique<Boolean>(false);
      push(object);
    } else if (op == Ops::OpEqual || op == Ops::OpNotEqual || op == Ops::OpGreaterThan) {
      executeComparision(op);
    } else if (op == Ops::OpBang) {
      executeBangOperator();
    } else if (op == Ops::OpMinus) {
      executeMinusOperator();
    } else if (op == Ops::OpJump) {
      int position = (int(instructions[ip + 1]) << 8) | int(instructions[ip + 2]);
      // We should jump to the position - 1 because the for loop will increment it
      ip = position - 1;
    } else if (op == Ops::OpJumpNotTruthy) {
      int position = (int(instructions[ip + 1]) << 8) | int(instructions[ip + 2]);

      // Go to the if branch
      ip += 2;

      auto condition = pop();
      if (!isTruthy(condition)) {
        // Go to the else branch
        ip = position - 1;
      }

    } else {
      spdlog::error("unknown opcode: {}", op);
    }
  }
}

void VM::push(std::unique_ptr<Object> &object) {
  if (sp >= StackSize) {
    throw std::runtime_error("stack overflow");
  }

  stack[sp] = std::move(object);
  sp++;
}

std::unique_ptr<Object> VM::pop() {
  if (sp == 0) {
    throw std::runtime_error("stack underflow");
  }

  auto object = std::move(stack[sp - 1]);
  sp--;
  return object;
}

void VM::executeBinaryOperation(const Opcode &op) {
  auto right = pop();
  auto left = pop();
  auto leftType = left->type();
  auto rightType = right->type();

  if (leftType == INTEGER_OBJ && rightType == INTEGER_OBJ) {
    executeBinaryIntegerOperation(op, left, right);
  } else {
    spdlog::error("unsupported types for binary operation: {} {}", leftType, rightType);
  }
}

void VM::executeBinaryIntegerOperation(const Opcode &op,
                                       std::unique_ptr<Object> &left,
                                       std::unique_ptr<Object> &right) {
  Integer *rightInteger = dynamic_cast<Integer *>(right.get());
  Integer *leftInteger = dynamic_cast<Integer *>(left.get());

  int result{};
  if (op == Ops::OpAdd) {
    result = leftInteger->value + rightInteger->value;
  } else if (op == Ops::OpSub) {
    result = leftInteger->value - rightInteger->value;
  } else if (op == Ops::OpMul) {
    result = leftInteger->value * rightInteger->value;
  } else if (op == Ops::OpDiv) {
    result = leftInteger->value / rightInteger->value;
  } else {
    spdlog::error("unknown operator for integers: {}", op);
    return;
  }

  std::unique_ptr<Object> resultObject = std::make_unique<Integer>(result);
  push(resultObject);
}

void VM::executeComparision(const Opcode &op) {
  auto right = pop();
  auto left = pop();
  auto leftType = left->type();
  auto rightType = right->type();

  if (leftType == INTEGER_OBJ && rightType == INTEGER_OBJ) {
    executeIntegerComparision(op, left, right);
  } else if (leftType == BOOLEAN_OBJ && rightType == BOOLEAN_OBJ) {
    executeBooleanComparision(op, left, right);
  } else {
    spdlog::error("unsupported types for binary operation: {} {}", leftType, rightType);
  }
}

void VM::executeIntegerComparision(const Opcode &op, std::unique_ptr<Object> &left, std::unique_ptr<Object> &right) {
  Integer *rightInteger = dynamic_cast<Integer *>(right.get());
  Integer *leftInteger = dynamic_cast<Integer *>(left.get());

  bool result{};
  if (op == Ops::OpEqual) {
    result = leftInteger->value == rightInteger->value;
  } else if (op == Ops::OpNotEqual) {
    result = leftInteger->value != rightInteger->value;
  } else if (op == Ops::OpGreaterThan) {
    result = leftInteger->value > rightInteger->value;
  } else {
    spdlog::error("unknown operator for integers: {}", op);
    return;
  }

  std::unique_ptr<Object> resultObject = std::make_unique<Boolean>(result);
  push(resultObject);
}

void VM::executeBooleanComparision(const Opcode &op, std::unique_ptr<Object> &left, std::unique_ptr<Object> &right) {
  Boolean *rightBoolean = dynamic_cast<Boolean *>(right.get());
  Boolean *leftBoolean = dynamic_cast<Boolean *>(left.get());

  bool result{};
  if (op == Ops::OpEqual) {
    result = leftBoolean->value == rightBoolean->value;
  } else if (op == Ops::OpNotEqual) {
    result = leftBoolean->value != rightBoolean->value;
  } else {
    spdlog::error("unknown operator for booleans: {}", op);
    return;
  }

  std::unique_ptr<Object> resultObject = std::make_unique<Boolean>(result);
  push(resultObject);
}

void VM::executeBangOperator() {
  auto operand = pop();
  if (operand->type() == BOOLEAN_OBJ) {
    auto boolean = dynamic_cast<Boolean *>(operand.get());
    std::unique_ptr<Object> result = std::make_unique<Boolean>(!boolean->value);
    push(result);
  } else {
    std::unique_ptr<Object> result = std::make_unique<Boolean>(false);
    push(result);
  }
}

void VM::executeMinusOperator() {
  auto operand = pop();

  if (operand->type() == INTEGER_OBJ) {
    auto integer = dynamic_cast<Integer *>(operand.get());
    std::unique_ptr<Object> result = std::make_unique<Integer>(-integer->value);
    push(result);
  } else {
    spdlog::error("unsupported type for negation: {}", operand->type());
  }
}

std::unique_ptr<Object> VM::lastPoppedStackElem() { return std::move(lastPopped); }

bool VM::isTruthy(std::unique_ptr<Object> &object) {
  if (object->type() == BOOLEAN_OBJ) {
    auto boolean = dynamic_cast<Boolean *>(object.get());
    return boolean->value;
  }

  return true;
}
