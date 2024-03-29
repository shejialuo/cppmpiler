#include "vm.hpp"

#include "builtins.hpp"
#include "code.hpp"
#include "frame.hpp"
#include "object.hpp"
#include "spdlog/spdlog.h"

#include <memory>
#include <stdexcept>
#include <vector>

constexpr std::string_view INTEGER_OBJ = "INTEGER";
constexpr std::string_view BOOLEAN_OBJ = "BOOLEAN";
constexpr std::string_view RETURN_VALUE_OBJ = "RETURN_VALUE";
constexpr std::string_view ERROR_OBJ = "ERROR";
constexpr std::string_view STRING_OBJ = "STRING";
constexpr std::string_view ARRAY_OBJ = "ARRAY";

std::shared_ptr<Object> VM::True = std::make_shared<Boolean>(true);
std::shared_ptr<Object> VM::False = std::make_shared<Boolean>(false);

static int readTwoBytes(Instructions &instructions, int ip);

static int readTwoBytes(Instructions &instructions, int ip) {
  return (int(instructions[ip + 1]) << 8) | int(instructions[ip + 2]);
}

Object *VM::stackTop() {
  if (sp == 0) {
    return nullptr;
  }
  return stack[sp - 1].get();
}

void VM::run() {
  while (currentFrame()->ip < static_cast<int>(currentFrame()->instructions().size() - 1)) {
    currentFrame()->ip++;
    int ip = currentFrame()->ip;
    Instructions &instructions = currentFrame()->instructions();
    auto op = Opcode(instructions[ip]);
    if (op == Ops::OpConstant) {
      // Restore the index value here
      int index = readTwoBytes(instructions, ip);
      currentFrame()->ip += 2;

      push(constants[index]);
    } else if (op == Ops::OpAdd || op == Ops::OpSub || op == Ops::OpMul || op == Ops::OpDiv) {
      executeBinaryOperation(op);
    } else if (op == Ops::OpPop) {
      lastPopped = pop();
    } else if (op == Ops::OpTrue) {
      push(True);
    } else if (op == Ops::OpFalse) {
      push(False);
    } else if (op == Ops::OpEqual || op == Ops::OpNotEqual || op == Ops::OpGreaterThan) {
      executeComparision(op);
    } else if (op == Ops::OpBang) {
      executeBangOperator();
    } else if (op == Ops::OpMinus) {
      executeMinusOperator();
    } else if (op == Ops::OpJump) {
      int position = readTwoBytes(instructions, ip);
      // We should jump to the position - 1 because the for loop will increment it
      currentFrame()->ip = position - 1;
    } else if (op == Ops::OpJumpNotTruthy) {
      int position = readTwoBytes(instructions, ip);

      // Go to the if branch
      currentFrame()->ip += 2;

      auto condition = pop();
      if (!isTruthy(condition)) {
        // Go to the else branch
        currentFrame()->ip = position - 1;
      }
    } else if (op == Ops::OpSetGlobal) {
      int globalIndex = readTwoBytes(instructions, ip);
      currentFrame()->ip += 2;
      (*globals)[globalIndex] = pop();
    } else if (op == Ops::OpGetGlobal) {
      int globalIndex = readTwoBytes(instructions, ip);
      currentFrame()->ip += 2;
      auto &value = (*globals)[globalIndex];
      push((*globals)[globalIndex]);
    } else if (op == Ops::OpArray) {
      int numElements = readTwoBytes(instructions, ip);
      currentFrame()->ip += 2;

      auto array = buildArray(sp - numElements, sp);
      sp -= numElements;

      push(array);

    } else if (op == Ops::OpIndex) {
      auto index = pop();
      auto left = pop();

      executeIndexExpression(left, index);
    } else if (op == Ops::OpCall) {
      int argumentSize = int(instructions[ip + 1]);
      currentFrame()->ip++;

      executeCall(argumentSize);
    } else if (op == Ops::OpReturnValue) {
      auto returnValue = pop();
      auto frame = popFrame();
      sp = frame->basePointer - 1;

      push(returnValue);
    } else if (op == Ops::OpSetLocal) {
      int localIndex = int(instructions[ip + 1]);
      currentFrame()->ip++;
      // The base pointer is the index of the first local variable
      stack[currentFrame()->basePointer + localIndex] = pop();
    } else if (op == Ops::OpGetLocal) {
      int localIndex = int(instructions[ip + 1]);
      currentFrame()->ip++;

      push(stack[currentFrame()->basePointer + localIndex]);
    } else if (op == Ops::OpGetBuiltin) {
      int builtIndex = int(instructions[ip + 1]);
      currentFrame()->ip++;

      std::shared_ptr<Object> definition = Builtins::getBuiltinByIndex(builtIndex);

      push(definition);
    } else if (op == Ops::OpClosure) {
      int constantIndex = readTwoBytes(instructions, ip);
      int freeVariableSize = int(instructions[ip + 3]);
      currentFrame()->ip += 3;

      pushClosure(constantIndex, freeVariableSize);

    } else if (op == Ops::OpGetFree) {
      int freeIndex = int(instructions[ip + 1]);
      currentFrame()->ip++;

      auto currentClosure = currentFrame()->closure;
      push(currentClosure->free[freeIndex]);

    } else {
      spdlog::error("unknown opcode: {}", op);
    }
  }
}

void VM::push(std::shared_ptr<Object> &object) {
  if (sp >= StackSize) {
    throw std::runtime_error("stack overflow");
  }

  stack[sp] = object;
  sp++;
}

std::shared_ptr<Object> VM::pop() {
  if (sp == 0) {
    throw std::runtime_error("stack underflow");
  }

  auto object = stack[sp - 1];
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
  } else if (leftType == STRING_OBJ && rightType == STRING_OBJ) {
    executeBinaryStringOperation(op, left, right);
  } else {
    spdlog::error("unsupported types for binary operation: {} {}", leftType, rightType);
  }
}

void VM::executeBinaryIntegerOperation(const Opcode &op,
                                       std::shared_ptr<Object> &left,
                                       std::shared_ptr<Object> &right) {
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

  std::shared_ptr<Object> resultObject = std::make_shared<Integer>(result);
  push(resultObject);
}

void VM::executeBinaryStringOperation(const Opcode &op, std::shared_ptr<Object> &left, std::shared_ptr<Object> &right) {
  String *rightString = dynamic_cast<String *>(right.get());
  String *leftString = dynamic_cast<String *>(left.get());

  if (op == Ops::OpAdd) {
    std::string result = leftString->value + rightString->value;
    std::shared_ptr<Object> resultObject = std::make_shared<String>(result);
    push(resultObject);
  } else {
    spdlog::error("unknown operator for strings: {}", op);
  }
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

void VM::executeIntegerComparision(const Opcode &op, std::shared_ptr<Object> &left, std::shared_ptr<Object> &right) {
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

  result ? push(True) : push(False);
}

void VM::executeBooleanComparision(const Opcode &op, std::shared_ptr<Object> &left, std::shared_ptr<Object> &right) {
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

  result ? push(True) : push(False);
}

void VM::executeBangOperator() {
  auto operand = pop();
  if (operand->type() == BOOLEAN_OBJ) {
    auto boolean = dynamic_cast<Boolean *>(operand.get());
    !boolean->value ? push(True) : push(False);
  } else {
    push(False);
  }
}

void VM::executeMinusOperator() {
  auto operand = pop();

  if (operand->type() == INTEGER_OBJ) {
    auto integer = dynamic_cast<Integer *>(operand.get());
    std::shared_ptr<Object> result = std::make_shared<Integer>(-integer->value);
    push(result);
  } else {
    spdlog::error("unsupported type for negation: {}", operand->type());
  }
}

std::shared_ptr<Object> VM::lastPoppedStackElem() { return lastPopped; }

std::shared_ptr<Object> VM::buildArray(int startIndex, int endIndex) {
  std::vector<std::shared_ptr<Object>> elements;
  for (int i = startIndex; i < endIndex; i++) {
    elements.push_back(stack[i]);
  }

  auto array = std::make_shared<Array>();
  array->elements = elements;

  return array;
}

bool VM::isTruthy(std::shared_ptr<Object> &object) {
  if (object->type() == BOOLEAN_OBJ) {
    auto boolean = dynamic_cast<Boolean *>(object.get());
    return boolean->value;
  }

  return true;
}

void VM::executeIndexExpression(std::shared_ptr<Object> &left, std::shared_ptr<Object> &index) {
  if (left->type() == ARRAY_OBJ && index->type() == INTEGER_OBJ) {
    executeArrayIndex(left, index);
  } else {
    spdlog::error("index operator not supported: {} {}", left->type(), index->type());
  }
}

void VM::executeArrayIndex(std::shared_ptr<Object> &left, std::shared_ptr<Object> &index) {
  Array *array = dynamic_cast<Array *>(left.get());
  Integer *integer = dynamic_cast<Integer *>(index.get());

  int i = integer->value;

  if (i < 0 || i >= array->elements.size()) {
    spdlog::error("index out of bounds: {} {}", i, array->elements.size());
  } else {
    push(array->elements[i]);
  }
}

std::shared_ptr<Frame> &VM::currentFrame() { return frames[framesIndex - 1]; }

void VM::pushFrame(std::shared_ptr<Frame> &frame) {
  frames[framesIndex] = frame;
  framesIndex++;
}

std::shared_ptr<Frame> VM::popFrame() {
  framesIndex--;
  return frames[framesIndex];
}

void VM::executeCall(int argumentSize) {
  std::shared_ptr<Closure> closure = std::dynamic_pointer_cast<Closure>(stack[sp - 1 - argumentSize]);
  if (closure != nullptr) {
    return callClosure(closure, argumentSize);
  }

  std::shared_ptr<Builtin> builtin = std::dynamic_pointer_cast<Builtin>(stack[sp - 1 - argumentSize]);
  if (builtin != nullptr) {
    return callBuiltin(builtin, argumentSize);
  }

  spdlog::error("calling non-function and non-builtin");
}
void VM::callClosure(std::shared_ptr<Closure> &closure, int argumentSize) {
  auto frame = std::make_shared<Frame>(closure, sp - argumentSize);
  pushFrame(frame);

  // we extend the sp value for storing locals, the local constant
  // need the binding here.
  sp += closure->fn->numLocals;
}

void VM::callBuiltin(std::shared_ptr<Builtin> &builtin, int argumentSize) {
  std::vector<std::shared_ptr<Object>> args;
  for (int i = sp - argumentSize; i < sp; i++) {
    args.push_back(stack[i]);
  }

  std::shared_ptr<Object> result = builtin->fn(args);
  sp -= argumentSize + 1;

  if (result != nullptr) {
    push(result);
  }
}

void VM::pushClosure(int constantIndex, int numFree) {
  auto constant = constants[constantIndex];
  auto fn = std::dynamic_pointer_cast<CompiledFunction>(constant);
  if (fn == nullptr) {
    spdlog::error("not a function: {}", constant->type());
    return;
  }

  std::vector<std::shared_ptr<Object>> free{};

  for (int i = 0; i < numFree; i++) {
    free.push_back(stack[sp - numFree + i]);
  }
  sp -= numFree;

  std::shared_ptr<Object> closure = std::make_shared<Closure>(fn, std::move(free));

  push(closure);
}
