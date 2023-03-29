#ifndef _VM_VM_HPP_
#define _VM_VM_HPP_

#include "code.hpp"
#include "object.hpp"

#include <memory>
#include <optional>
#include <vector>

constexpr int StackSize = 2048;

class VM {
public:
  std::vector<std::unique_ptr<Object>> constants;
  Instructions instructions;

  std::vector<std::unique_ptr<Object>> stack;
  int sp;

  VM() = delete;
  VM(const VM &v) = delete;
  VM(std::vector<std::unique_ptr<Object>> &&constants, Instructions &&instructions)
      : constants{std::move(constants)}, instructions{std::move(instructions)}, sp{0}, stack(StackSize) {}

  /**
   * @brief get the top stack object
   *
   * @return Object*
   */
  Object *stackTop();

  /**
   * @brief start the vm
   *
   */
  void run();

  /**
   * @brief push the object to the stack
   *
   */
  void push(std::unique_ptr<Object> &object);
};

#endif  // _VM_VM_HPP_
