#ifndef _EVALUATOR_EVALUATOR_HPP_
#define _EVALUATOR_EVALUATOR_HPP_

#include "ast.hpp"
#include "object.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

class Evaluator {
private:
  static std::shared_ptr<Boolean> True;
  static std::shared_ptr<Boolean> False;

  static std::vector<std::shared_ptr<Environment>> environments;
  static std::unordered_map<std::string, std::shared_ptr<Builtin>> builtins;

public:
  /**
   * @brief evaluate the node
   *
   * @param node the unique_ptr parsed by `Parser::program()`
   * @param env the environment
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> eval(Node *node, std::shared_ptr<Environment> &env);

  /**
   * @brief iteratively evaluate the program
   *
   * @param statements
   * @param env
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> evalProgram(std::vector<std::unique_ptr<Statement>> &statements,
                                             std::shared_ptr<Environment> &env);

  /**
   * @brief First calculate the right object, and then calculate
   * the prefix operation
   *
   * @param op the prefix operation
   * @param right the right evaluated object
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> evalPrefixExpression(const std::string &op, std::shared_ptr<Object> &right);

  /**
   * @brief should be called by `evalPrefixExpression`
   *
   * @param right the right evaluated object
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> evalBangOperationExpression(std::shared_ptr<Object> &right);

  /**
   * @brief should be called by `evalPrefixExpression`
   *
   * @param right the right evaluated object
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> evalMinusOperationExpression(std::shared_ptr<Object> &right);

  /**
   * @brief First calculate the left expression and right
   * expression. And then calculate
   *
   * @param op the infix operator
   * @param left the left evaluated object
   * @param right the right evaluated object
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> evalInfixExpression(const std::string &op,
                                                     std::shared_ptr<Object> &left,
                                                     std::shared_ptr<Object> &right);

  /**
   * @brief Integer infix expression evaluation
   *
   * @param op the infix operator
   * @param left the left evaluated object
   * @param right the right evaluated object
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> evalIntegerInfixExpression(const std::string &op,
                                                            std::shared_ptr<Object> &left,
                                                            std::shared_ptr<Object> &right);

  /**
   * @brief Boolean infix expression evaluation
   *
   * @param op the infix operator
   * @param left the left evaluated object
   * @param right the right evaluated object
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> evalBooleanInfixExpression(const std::string &op,
                                                            std::shared_ptr<Object> &left,
                                                            std::shared_ptr<Object> &right);

  /**
   * @brief String infix expression evaluation
   *
   * @param op the infix operator
   * @param left the left evaluated object
   * @param right the right evaluated object
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> evalStringInfixExpression(const std::string &op,
                                                           std::shared_ptr<Object> &left,
                                                           std::shared_ptr<Object> &right);

  /**
   * @brief Evaluate the `IfExpression`
   *
   * @param ie IfExpression
   * @param en
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> evalIfExpression(IfExpression *ie, std::shared_ptr<Environment> &env);

  /**
   * @brief Evaluate the the block statement
   *
   * @param bs
   * @param env
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> evalBlockStatement(BlockStatement *bs, std::shared_ptr<Environment> &env);

  /**
   * @brief Generate the Error message
   *
   * @param s the error message
   * @return std::shared_ptr<Error>
   */
  static std::shared_ptr<Error> newError(const std::string &s);

  /**
   * @brief eval the identifier
   *
   * @param i
   * @param env
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> evalIdentifier(Identifier *i, std::shared_ptr<Environment> &env);

  /**
   * @brief eval the call arguments
   *
   * @param arguments the call arguments
   * @param env the environment
   * @return std::vector<std::shared_ptr<Object>>
   */
  static std::vector<std::shared_ptr<Object>> evalArguments(std::vector<std::unique_ptr<Expression>> &arguments,
                                                            std::shared_ptr<Environment> &env);

  /**
   * @brief Evaluate functions
   *
   * @param fn the function pointer
   * @param arguments the evaluated arguments.
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> evalFunctions(Object *fn, std::vector<std::shared_ptr<Object>> &arguments);

  /**
   * @brief The built function len to calculate the string length
   *
   * @param arguments the arguments
   * @return std::shared_ptr<Object>;
   */
  static std::shared_ptr<Object> len(std::vector<std::shared_ptr<Object>> &arguments);
};

#endif  // _EVALUATOR_EVALUATOR_HPP_
