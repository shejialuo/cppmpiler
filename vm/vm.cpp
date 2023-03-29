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
