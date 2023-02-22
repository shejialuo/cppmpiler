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

#endif  // _EVALUATOR_EVALUATOR_HPP_
