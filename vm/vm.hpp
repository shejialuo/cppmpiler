#ifndef _VM_VM_HPP_
#define _VM_VM_HPP_

#include "code.hpp"
#include "object.hpp"

#include <memory>
#include <optional>
#include <vector>

constexpr int StackSize = 2048;

class VM {
private:
  // For test only
  std::unique_ptr<Object> lastPopped;

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

  /**
   * @brief pop the object from the stack
   *
   * @return std::unique_ptr<Object>
   */
  std::unique_ptr<Object> pop();

  /**
   * @brief execute the binary operation
   *
   * @param op the operator
   */
  void executeBinaryOperation(const Opcode &op);

  /**
   * @brief execute the integer operator such as add, sub, mul, div
   *
   * @param op the operator
   * @param left the left object
   * @param right the right object
   */
  void executeBinaryIntegerOperation(const Opcode &op, std::unique_ptr<Object> &left, std::unique_ptr<Object> &right);

  /**
   * @brief execute the comparison such as equal, not equal, greater than, less than
   *
   * @param op
   */
  void executeComparision(const Opcode &op);

  /**
   * @brief execute the integer comparison such as equal, not equal, greater than, less than
   *
   */
  void executeIntegerComparision(const Opcode &op, std::unique_ptr<Object> &left, std::unique_ptr<Object> &right);

  /**
   * @brief execute the boolean comparison such as equal, not equal
   *
   */
  void executeBooleanComparision(const Opcode &op, std::unique_ptr<Object> &left, std::unique_ptr<Object> &right);

  /**
   * @brief execute the bang operator, get the value from the stack
   *
   */
  void executeBangOperator();

  /**
   * @brief execute the minus operator, get the value from the stack
   *
   */
  void executeMinusOperator();

  /**
   * @brief For test only get last popped
   *
   */
  std::unique_ptr<Object> lastPoppedStackElem();

  /**
   * @brief get the boolean value from the object
   *
   */
  bool isTruthy(std::unique_ptr<Object> &object);
};

#endif  // _VM_VM_HPP_
