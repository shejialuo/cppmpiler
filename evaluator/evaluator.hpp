#ifndef _EVALUATOR_EVALUATOR_HPP_
#define _EVALUATOR_EVALUATOR_HPP_

#include "ast.hpp"
#include "environment.hpp"
#include "object.hpp"

#include <memory>
#include <vector>

class Evaluator {
public:
  /**
   * @brief evaluate the node
   *
   * @param node the unique_ptr parsed by `Parser::program()`
   * @param env the environment
   * @return std::unique_ptr<Object>
   */
  static std::unique_ptr<Object> eval(Node *node, std::unique_ptr<Environment> &env);

  /**
   * @brief iteratively evaluate the program
   *
   * @param statements
   * @param env
   * @return std::unique_ptr<Object>
   */
  static std::unique_ptr<Object> evalProgram(std::vector<std::unique_ptr<Statement>> &statements,
                                             std::unique_ptr<Environment> &env);

  /**
   * @brief First calculate the right object, and then calculate
   * the prefix operation
   *
   * @param op the prefix operation
   * @param right the right evaluated object
   * @return std::unique_ptr<Object>
   */
  static std::unique_ptr<Object> evalPrefixExpression(const std::string &op, std::unique_ptr<Object> &right);

  /**
   * @brief should be called by `evalPrefixExpression`
   *
   * @param right the right evaluated object
   * @return std::unique_ptr<Object>
   */
  static std::unique_ptr<Object> evalBangOperationExpression(std::unique_ptr<Object> &right);

  /**
   * @brief should be called by `evalPrefixExpression`
   *
   * @param right the right evaluated object
   * @return std::unique_ptr<Object>
   */
  static std::unique_ptr<Object> evalMinusOperationExpression(std::unique_ptr<Object> &right);

  /**
   * @brief First calculate the left expression and right
   * expression. And then calculate
   *
   * @param op the infix operator
   * @param left the left evaluated object
   * @param right the right evaluated object
   * @return std::unique_ptr<Object>
   */
  static std::unique_ptr<Object> evalInfixExpression(const std::string &op,
                                                     std::unique_ptr<Object> &left,
                                                     std::unique_ptr<Object> &right);

  /**
   * @brief Integer infix expression evaluation
   *
   * @param op the infix operator
   * @param left the left evaluated object
   * @param right the right evaluated object
   * @return std::unique_ptr<Object>
   */
  static std::unique_ptr<Object> evalIntegerInfixExpression(const std::string &op,
                                                            std::unique_ptr<Object> &left,
                                                            std::unique_ptr<Object> &right);

  /**
   * @brief Boolean infix expression evaluation
   *
   * @param op the infix operator
   * @param left the left evaluated object
   * @param right the right evaluated object
   * @return std::unique_ptr<Object>
   */
  static std::unique_ptr<Object> evalBooleanInfixExpression(const std::string &op,
                                                            std::unique_ptr<Object> &left,
                                                            std::unique_ptr<Object> &right);

  /**
   * @brief Evaluate the `IfExpression`
   *
   * @param ie IfExpression
   * @param en
   * @return std::unique_ptr<Object>
   */
  static std::unique_ptr<Object> evalIfExpression(IfExpression *ie, std::unique_ptr<Environment> &env);

  /**
   * @brief Evaluate the the block statement
   *
   * @param bs
   * @param env
   * @return std::unique_ptr<Object>
   */
  static std::unique_ptr<Object> evalBlockStatement(BlockStatement *bs, std::unique_ptr<Environment> &env);

  /**
   * @brief Generate the Error message
   *
   * @param s the error message
   * @return std::unique_ptr<Error>
   */
  static std::unique_ptr<Error> newError(const std::string &s);

  /**
   * @brief eval the identifier
   *
   * @param i
   * @param env
   * @return std::unique_ptr<Object>
   */
  static std::unique_ptr<Object> evalIdentifier(Identifier *i, std::unique_ptr<Environment> &env);
};

#endif  // _EVALUATOR_EVALUATOR_HPP_
