#include "vm.hpp"

#include "code.hpp"
#include "object.hpp"

#include <memory>
#include <stdexcept>

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
    } else if (op == Ops::OpAdd) {
      auto right = pop();
      auto left = pop();

      Integer *rightInteger = dynamic_cast<Integer *>(right.get());
      Integer *leftInteger = dynamic_cast<Integer *>(left.get());

      std::unique_ptr<Object> result = std::make_unique<Integer>(leftInteger->value + rightInteger->value);
      push(result);
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
