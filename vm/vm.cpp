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

std::unique_ptr<Object> VM::lastPoppedStackElem() { return std::move(lastPopped); }
