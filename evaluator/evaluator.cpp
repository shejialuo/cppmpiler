#include "evaluator.hpp"

#include "ast.hpp"
#include "object.hpp"

#include <memory>

constexpr std::string_view INTEGER_OBJ = "INTEGER";
constexpr std::string_view BOOLEAN_OBJ = "BOOLEAN";

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

  BooleanExpression *bs = dynamic_cast<BooleanExpression *>(node);
  if (bs != nullptr) {
    return std::move(std::make_unique<Boolean>(bs->value));
  }

  PrefixExpression *pe = dynamic_cast<PrefixExpression *>(node);
  if (pe != nullptr) {
    auto right = eval(pe->right.get());
    return std::move(evalPrefixExpression(pe->_operator, right));
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

std::unique_ptr<Object> evalPrefixExpression(const std::string &op, std::unique_ptr<Object> &right) {
  if (op == "!") {
    return std::move(evalBangOperationExpression(right));
  } else if (op == "-") {
    return std::move(evalMinusOperationExpression(right));
  }
  return nullptr;
}

std::unique_ptr<Object> evalBangOperationExpression(std::unique_ptr<Object> &right) {
  Boolean *boolean = dynamic_cast<Boolean *>(right.get());

  // Here, I don't use `Null` class, I just use `nullptr`
  // for simplicity.
  if (boolean == nullptr) {
    return std::move(std::make_unique<Boolean>(false));
  }

  if (!boolean->value) {
    return std::move(std::make_unique<Boolean>(true));
  }

  return std::move(std::make_unique<Boolean>(false));
}

std::unique_ptr<Object> evalMinusOperationExpression(std::unique_ptr<Object> &right) {
  if (right->type() != INTEGER_OBJ) {
    return nullptr;
  }

  Integer *integer = dynamic_cast<Integer *>(right.get());

  return std::move(std::make_unique<Integer>(-integer->value));
}
