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
 * @brief iteratively evaluate the statements
 *
 * @param statements
 * @return std::unique_ptr<Object>
 */
std::unique_ptr<Object> evalStatements(std::vector<std::unique_ptr<Statement>> &statements);

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

#endif  // _EVALUATOR_EVALUATOR_HPP_
