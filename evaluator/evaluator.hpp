#ifndef _EVALUATOR_EVALUATOR_HPP_
#define _EVALUATOR_EVALUATOR_HPP_

#include "ast.hpp"
#include "object.hpp"

#include <memory>
#include <vector>

/**
 * @brief evaluate the node
 *
 * @param node the unique_ptr parsed by `Parser::program()`
 * @return std::unique_ptr<Object>
 */
std::unique_ptr<Object> eval(Node *node);

/**
 * @brief iteratively evaluate the program
 *
 * @param statements
 * @return std::unique_ptr<Object>
 */
std::unique_ptr<Object> evalProgram(std::vector<std::unique_ptr<Statement>> &statements);

/**
 * @brief First calculate the right object, and then calculate
 * the prefix operation
 *
 * @param op the prefix operation
 * @param right the right evaluated object
 * @return std::unique_ptr<Object>
 */
std::unique_ptr<Object> evalPrefixExpression(const std::string &op, std::unique_ptr<Object> &right);

/**
 * @brief should be called by `evalPrefixExpression`
 *
 * @param right the right evaluated object
 * @return std::unique_ptr<Object>
 */
std::unique_ptr<Object> evalBangOperationExpression(std::unique_ptr<Object> &right);

/**
 * @brief should be called by `evalPrefixExpression`
 *
 * @param right the right evaluated object
 * @return std::unique_ptr<Object>
 */
std::unique_ptr<Object> evalMinusOperationExpression(std::unique_ptr<Object> &right);

/**
 * @brief First calculate the left expression and right
 * expression. And then calculate
 *
 * @param op the infix operator
 * @param left the left evaluated object
 * @param right the right evaluated object
 * @return std::unique_ptr<Object>
 */
std::unique_ptr<Object> evalInfixExpression(const std::string &op,
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
std::unique_ptr<Object> evalIntegerInfixExpression(const std::string &op,
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
std::unique_ptr<Object> evalBooleanInfixExpression(const std::string &op,
                                                   std::unique_ptr<Object> &left,
                                                   std::unique_ptr<Object> &right);

/**
 * @brief Evaluate the `IfExpression`
 *
 * @param ie IfExpression
 * @return std::unique_ptr<Object>
 */
std::unique_ptr<Object> evalIfExpression(IfExpression *ie);

/**
 * @brief Evaluate the the block statement
 *
 * @param bs
 * @return std::unique_ptr<Object>
 */
std::unique_ptr<Object> evalBlockStatement(BlockStatement *bs);

#endif  // _EVALUATOR_EVALUATOR_HPP_