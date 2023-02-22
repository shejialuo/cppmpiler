#include "evaluator.hpp"

#include "ast.hpp"
#include "object.hpp"

#include <memory>

std::unique_ptr<Object> eval(Node *node) {
  Program *program = dynamic_cast<Program *>(node);

  if (program != nullptr) {
    return std::move(evalStatements(program->statements));
  }

  ExpressionStatement *es = dynamic_cast<ExpressionStatement *>(node);

  if (es != nullptr) {
    return std::move(eval(es->expression.get()));
  }

  IntegerLiteral *integer = dynamic_cast<IntegerLiteral *>(node);

  if (integer != nullptr) {
    return std::move(std::make_unique<Integer>(integer->value));
  }

  return nullptr;
}

std::unique_ptr<Object> evalStatements(std::vector<std::unique_ptr<Statement>> &statements) {
  std::unique_ptr<Object> result{};

  for (auto &&statement : statements) {
    result = std::move(eval(statement.get()));
  }

  return std::move(result);
}
