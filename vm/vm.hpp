#ifndef _VM_VM_HPP_
#define _VM_VM_HPP_

#include "code.hpp"
#include "frame.hpp"
#include "object.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <vector>

static constexpr int StackSize = 2048;
static constexpr int GlobalSize = 65536;
static constexpr int MaxFrames = 1024;

class VM {
private:
  static std::shared_ptr<Object> True;
  static std::shared_ptr<Object> False;

  // For test only
  std::shared_ptr<Object> lastPopped;

public:
  std::vector<std::shared_ptr<Object>> constants;
  std::shared_ptr<std::vector<std::shared_ptr<Object>>> globals;

  std::vector<std::shared_ptr<Object>> stack;
  int sp;

  std::vector<std::shared_ptr<Frame>> frames;
  int framesIndex;

  VM() = delete;

  VM(const VM &v) = delete;
  VM(std::vector<std::shared_ptr<Object>> &&constants_, Instructions &&instructions)
      : constants{std::move(constants_)}, sp{0}, stack(StackSize), frames(MaxFrames), framesIndex{1} {
    globals = std::make_shared<std::vector<std::shared_ptr<Object>>>(GlobalSize);
    auto mainFn = std::make_shared<CompiledFunction>(std::move(instructions), 0);
    // Here, we must set the frame's ip to -1
    auto mainFrame = std::make_shared<Frame>(mainFn, 0);
    frames[0] = mainFrame;
  }

  VM(std::vector<std::shared_ptr<Object>> &&constants_,
     std::shared_ptr<std::vector<std::shared_ptr<Object>>> &globals_,
     Instructions &&instructions)
      : constants{std::move(constants_)}
      , globals{globals_}
      , sp{0}
      , stack(StackSize)
      , frames(MaxFrames)
      , framesIndex{1} {
    auto mainFn = std::make_shared<CompiledFunction>(std::move(instructions), 0);
    auto mainFrame = std::make_shared<Frame>(mainFn, 0);
    frames[0] = mainFrame;
  }

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
  void push(std::shared_ptr<Object> &object);

  /**
   * @brief pop the object from the stack
   *
   * @return std::shared_ptr<Object>
   */
  std::shared_ptr<Object> pop();

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
  void executeBinaryIntegerOperation(const Opcode &op, std::shared_ptr<Object> &left, std::shared_ptr<Object> &right);

  /**
   * @brief execute the string operator such as add
   *
   * @param op the operator
   * @param left the left object
   * @param right the right object
   */
  void executeBinaryStringOperation(const Opcode &op, std::shared_ptr<Object> &left, std::shared_ptr<Object> &right);

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
  void executeIntegerComparision(const Opcode &op, std::shared_ptr<Object> &left, std::shared_ptr<Object> &right);

  /**
   * @brief execute the boolean comparison such as equal, not equal
   *
   */
  void executeBooleanComparision(const Opcode &op, std::shared_ptr<Object> &left, std::shared_ptr<Object> &right);

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
   * @brief execute the index expression
   *
   * @param left the left object
   * @param index the index object
   */
  void executeIndexExpression(std::shared_ptr<Object> &left, std::shared_ptr<Object> &index);

  /**
   * @brief execute the array index
   *
   */
  void executeArrayIndex(std::shared_ptr<Object> &left, std::shared_ptr<Object> &index);

  /**
   * @brief execute the function call
   *
   * @param argumentSize
   */
  void executeCall(int argumentSize);

  /**
   * @brief execute the function call
   *
   * @param fn
   * @param argumentSize
   */
  void callFunction(std::shared_ptr<CompiledFunction> &fn, int argumentSize);

  /**
   * @brief execute the builtin function call
   *
   * @param builtin
   * @param argumentSize
   */
  void callBuiltin(std::shared_ptr<Builtin> &builtin, int argumentSize);

  /**
   * @brief For test only get last popped
   *
   */
  std::shared_ptr<Object> lastPoppedStackElem();

  /**
   * @brief build the array object
   *
   * @param startIndex the start index of the array
   * @param endIndex the end index of the array
   * @return std::shared_ptr<Object>
   */
  std::shared_ptr<Object> buildArray(int startIndex, int endIndex);

  /**
   * @brief get the boolean value from the object
   *
   */
  bool isTruthy(std::shared_ptr<Object> &object);

  /**
   * @brief get the current frame
   *
   * @return std::shared_ptr<Frame>
   */
  std::shared_ptr<Frame> &currentFrame();

  /**
   * @brief push the frame
   *
   */
  void pushFrame(std::shared_ptr<Frame> &frame);

  /**
   * @brief pop the frame
   *
   * @return std::shared_ptr<Frame>
   */
  std::shared_ptr<Frame> popFrame();
};

#endif  // _VM_VM_HPP_
