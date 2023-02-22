#include "evaluator.hpp"

#include "ast.hpp"
#include "object.hpp"
#include "spdlog/spdlog.h"

#include <iostream>
#include <memory>
#include <utility>

constexpr std::string_view INTEGER_OBJ = "INTEGER";
constexpr std::string_view BOOLEAN_OBJ = "BOOLEAN";
constexpr std::string_view RETURN_VALUE_OBJ = "RETURN_VALUE";
constexpr std::string_view ERROR_OBJ = "ERROR";

std::unique_ptr<Object> eval(Node *node) {
  Program *program = dynamic_cast<Program *>(node);

  if (program != nullptr) {
    return std::move(evalProgram(program->statements));
  }

  ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(node);

  if (expressionStatement != nullptr) {
    return std::move(eval(expressionStatement->expression.get()));
  }

  BlockStatement *blockStatement = dynamic_cast<BlockStatement *>(node);
  if (blockStatement != nullptr) {
    return std::move(evalBlockStatement(blockStatement));
  }

  IntegerLiteral *integer = dynamic_cast<IntegerLiteral *>(node);

  if (integer != nullptr) {
    return std::move(std::make_unique<Integer>(integer->value));
  }

  BooleanExpression *booleanExpression = dynamic_cast<BooleanExpression *>(node);
  if (booleanExpression != nullptr) {
    return std::move(std::make_unique<Boolean>(booleanExpression->value));
  }

  PrefixExpression *prefixExpression = dynamic_cast<PrefixExpression *>(node);
  if (prefixExpression != nullptr) {
    auto right = eval(prefixExpression->right.get());
    return std::move(evalPrefixExpression(prefixExpression->_operator, right));
  }

  InfixExpression *infixExpression = dynamic_cast<InfixExpression *>(node);
  if (infixExpression != nullptr) {
    auto left = eval(infixExpression->left.get());
    auto right = eval(infixExpression->right.get());
    return std::move(evalInfixExpression(infixExpression->_operator, left, right));
  }

  IfExpression *ifExpression = dynamic_cast<IfExpression *>(node);
  if (ifExpression != nullptr) {
    return std::move(evalIfExpression(ifExpression));
  }

  ReturnStatement *returnStatement = dynamic_cast<ReturnStatement *>(node);
  if (returnStatement != nullptr) {
    auto val = eval(returnStatement->returnValue.get());
    auto returnValue = std::make_unique<ReturnValue>();
    returnValue->value = std::move(val);
    return std::move(returnValue);
  }

  return nullptr;
}

std::unique_ptr<Object> evalProgram(std::vector<std::unique_ptr<Statement>> &statements) {
  std::unique_ptr<Object> result{};

  for (auto &&statement : statements) {
    result = std::move(eval(statement.get()));

    ReturnValue *returnValue = dynamic_cast<ReturnValue *>(result.get());
    if (returnValue != nullptr) {
      return std::move(returnValue->value);
    }

    Error *error = dynamic_cast<Error *>(result.get());
    if (error != nullptr) {
      return std::move(result);
    }
  }

  return std::move(result);
}

std::unique_ptr<Object> evalPrefixExpression(const std::string &op, std::unique_ptr<Object> &right) {
  if (op == "!") {
    return std::move(evalBangOperationExpression(right));
  } else if (op == "-") {
    return std::move(evalMinusOperationExpression(right));
  }
  return newError("unkown operator: " + op + right->type());
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
    return newError("unknown operator: -" + right->type());
  }

  Integer *integer = dynamic_cast<Integer *>(right.get());

  return std::move(std::make_unique<Integer>(-integer->value));
}

std::unique_ptr<Object> evalInfixExpression(const std::string &op,
                                            std::unique_ptr<Object> &left,
                                            std::unique_ptr<Object> &right) {
  if (left->type() == INTEGER_OBJ && right->type() == INTEGER_OBJ) {
    return std::move(evalIntegerInfixExpression(op, left, right));
  } else if (left->type() == BOOLEAN_OBJ && right->type() == BOOLEAN_OBJ) {
    return std::move(evalBooleanInfixExpression(op, left, right));
  } else if (left->type() != right->type()) {
    return newError("type mismatch: " + left->type() + " " + op + " " + right->type());
  }
  return newError("unknown operator: " + left->type() + " " + op + " " + right->type());
}

std::unique_ptr<Object> evalIntegerInfixExpression(const std::string &op,
                                                   std::unique_ptr<Object> &left,
                                                   std::unique_ptr<Object> &right) {
  Integer *leftInteger = dynamic_cast<Integer *>(left.get());
  Integer *rightInteger = dynamic_cast<Integer *>(right.get());

  if (op == "+") {
    return std::move(std::make_unique<Integer>(leftInteger->value + rightInteger->value));
  } else if (op == "-") {
    return std::move(std::make_unique<Integer>(leftInteger->value - rightInteger->value));
  } else if (op == "*") {
    return std::move(std::make_unique<Integer>(leftInteger->value * rightInteger->value));
  } else if (op == "/") {
    return std::move(std::make_unique<Integer>(leftInteger->value / rightInteger->value));
  } else if (op == "<") {
    return std::make_unique<Boolean>(leftInteger->value < rightInteger->value);
  } else if (op == ">") {
    return std::make_unique<Boolean>(leftInteger->value > rightInteger->value);
  } else if (op == "==") {
    return std::make_unique<Boolean>(leftInteger->value == rightInteger->value);
  } else if (op == "!=") {
    return std::make_unique<Boolean>(leftInteger->value != rightInteger->value);
  }

  return newError("unknown operator: " + left->type() + " " + op + " " + right->type());
}

std::unique_ptr<Object> evalBooleanInfixExpression(const std::string &op,
                                                   std::unique_ptr<Object> &left,
                                                   std::unique_ptr<Object> &right) {
  Boolean *leftBoolean = dynamic_cast<Boolean *>(left.get());
  Boolean *rightBoolean = dynamic_cast<Boolean *>(right.get());

  if (op == "==") {
    return std::make_unique<Boolean>(leftBoolean->value == rightBoolean->value);
  } else if (op == "!=") {
    return std::make_unique<Boolean>(leftBoolean->value != rightBoolean->value);
  }

  return newError("unknown operator: " + left->type() + " " + op + " " + right->type());
}

std::unique_ptr<Object> evalIfExpression(IfExpression *ie) {
  auto condition = eval(ie->condition.get());

  if (condition == nullptr) {
    return nullptr;
  }

  Boolean *result = dynamic_cast<Boolean *>(condition.get());

  // Corner case: if (1) {10} else {20}
  Integer *integer = dynamic_cast<Integer *>(condition.get());

  if (result == nullptr && integer != nullptr) {
    if (integer->value != 0) {
      return std::move(eval(ie->consequence.get()));
    } else {
      return std::move(eval(ie->alternative.get()));
    }
  } else if (!result->value) {
    if (ie->alternative != nullptr) {
      return std::move(eval(ie->alternative.get()));
    }
  } else {
    return std::move(eval(ie->consequence.get()));
  }

  return nullptr;
}

std::unique_ptr<Object> evalBlockStatement(BlockStatement *bs) {
  std::unique_ptr<Object> result{};

  for (auto &&statement : bs->statements) {
    result = std::move(eval(statement.get()));

    if (result != nullptr) {
      if (result->type() == RETURN_VALUE_OBJ || result->type() == ERROR_OBJ)
        return result;
    }
  }

  return result;
}

std::unique_ptr<Error> newError(const std::string &s) { return std::make_unique<Error>(s); }
