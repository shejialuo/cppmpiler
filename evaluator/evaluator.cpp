#include "evaluator.hpp"

#include "ast.hpp"
#include "object.hpp"
#include "spdlog/spdlog.h"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

constexpr std::string_view INTEGER_OBJ = "INTEGER";
constexpr std::string_view BOOLEAN_OBJ = "BOOLEAN";
constexpr std::string_view RETURN_VALUE_OBJ = "RETURN_VALUE";
constexpr std::string_view ERROR_OBJ = "ERROR";

std::shared_ptr<Boolean> Evaluator::True = std::make_shared<Boolean>(true);
std::shared_ptr<Boolean> Evaluator::False = std::make_shared<Boolean>(false);

std::shared_ptr<Object> Evaluator::eval(Node *node, std::unique_ptr<Environment> &env) {
  Program *program = dynamic_cast<Program *>(node);

  if (program != nullptr) {
    return evalProgram(program->statements, env);
  }

  ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(node);

  if (expressionStatement != nullptr) {
    return std::move(eval(expressionStatement->expression.get(), env));
  }

  BlockStatement *blockStatement = dynamic_cast<BlockStatement *>(node);
  if (blockStatement != nullptr) {
    return std::move(evalBlockStatement(blockStatement, env));
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
    auto right = eval(prefixExpression->right.get(), env);
    return std::move(evalPrefixExpression(prefixExpression->_operator, right));
  }

  InfixExpression *infixExpression = dynamic_cast<InfixExpression *>(node);
  if (infixExpression != nullptr) {
    auto left = eval(infixExpression->left.get(), env);
    auto right = eval(infixExpression->right.get(), env);
    return evalInfixExpression(infixExpression->_operator, left, right);
  }

  IfExpression *ifExpression = dynamic_cast<IfExpression *>(node);
  if (ifExpression != nullptr) {
    return std::move(evalIfExpression(ifExpression, env));
  }

  ReturnStatement *returnStatement = dynamic_cast<ReturnStatement *>(node);
  if (returnStatement != nullptr) {
    auto val = eval(returnStatement->returnValue.get(), env);
    auto returnValue = std::make_shared<ReturnValue>();
    returnValue->value = val;
    return std::move(returnValue);
  }

  LetStatement *letStatement = dynamic_cast<LetStatement *>(node);
  if (letStatement != nullptr) {
    auto val = eval(letStatement->value.get(), env);
    env->set(letStatement->name->value, std::move(val));
  }

  Identifier *identifier = dynamic_cast<Identifier *>(node);
  if (identifier != nullptr) {
    return evalIdentifier(identifier, env);
  }

  FunctionLiteral *functionLiteral = dynamic_cast<FunctionLiteral *>(node);
  if (functionLiteral != nullptr) {
    auto function = std::make_unique<Function>(functionLiteral->parameters, std::move(functionLiteral->body), env);
    return std::move(function);
  }

  CallExpression *callExpression = dynamic_cast<CallExpression *>(node);
  if (callExpression != nullptr) {
    auto function = eval(callExpression->function.get(), env);
    auto arguments = evalArguments(callExpression->arguments, env);
    return evalFunctions(function.get(), arguments);
  }

  return nullptr;
}

std::shared_ptr<Object> Evaluator::evalProgram(std::vector<std::unique_ptr<Statement>> &statements,
                                               std::unique_ptr<Environment> &env) {
  std::shared_ptr<Object> result{};

  for (auto &&statement : statements) {
    result = std::move(eval(statement.get(), env));

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

std::shared_ptr<Object> Evaluator::evalPrefixExpression(const std::string &op, std::shared_ptr<Object> &right) {
  if (op == "!") {
    return evalBangOperationExpression(right);
  } else if (op == "-") {
    return std::move(evalMinusOperationExpression(right));
  }
  return newError("unkown operator: " + op + right->type());
}

std::shared_ptr<Object> Evaluator::evalBangOperationExpression(std::shared_ptr<Object> &right) {
  Boolean *boolean = dynamic_cast<Boolean *>(right.get());

  // Here, I don't use `Null` class, I just use `nullptr`
  // for simplicity.
  if (boolean == nullptr) {
    return std::shared_ptr<Boolean>(False);
  }

  if (!boolean->value) {
    return std::shared_ptr<Boolean>(True);
  }

  return std::shared_ptr<Boolean>(False);
}

std::shared_ptr<Object> Evaluator::evalMinusOperationExpression(std::shared_ptr<Object> &right) {
  if (right->type() != INTEGER_OBJ) {
    return newError("unknown operator: -" + right->type());
  }

  Integer *integer = dynamic_cast<Integer *>(right.get());

  return std::make_shared<Integer>(-integer->value);
}

std::shared_ptr<Object> Evaluator::evalInfixExpression(const std::string &op,
                                                       std::shared_ptr<Object> &left,
                                                       std::shared_ptr<Object> &right) {
  if (left->type() == INTEGER_OBJ && right->type() == INTEGER_OBJ) {
    return std::move(evalIntegerInfixExpression(op, left, right));
  } else if (left->type() == BOOLEAN_OBJ && right->type() == BOOLEAN_OBJ) {
    return std::move(evalBooleanInfixExpression(op, left, right));
  } else if (left->type() != right->type()) {
    return newError("type mismatch: " + left->type() + " " + op + " " + right->type());
  }
  return newError("unknown operator: " + left->type() + " " + op + " " + right->type());
}

std::shared_ptr<Object> Evaluator::evalIntegerInfixExpression(const std::string &op,
                                                              std::shared_ptr<Object> &left,
                                                              std::shared_ptr<Object> &right) {
  Integer *leftInteger = dynamic_cast<Integer *>(left.get());
  Integer *rightInteger = dynamic_cast<Integer *>(right.get());

  if (op == "+") {
    return std::make_shared<Integer>(leftInteger->value + rightInteger->value);
  } else if (op == "-") {
    return std::make_shared<Integer>(leftInteger->value - rightInteger->value);
  } else if (op == "*") {
    return std::make_shared<Integer>(leftInteger->value * rightInteger->value);
  } else if (op == "/") {
    return std::make_shared<Integer>(leftInteger->value / rightInteger->value);
  } else if (op == "<") {
    return leftInteger->value < rightInteger->value ? std::shared_ptr<Boolean>(True) : std::shared_ptr<Boolean>(False);
  } else if (op == ">") {
    return leftInteger->value > rightInteger->value ? std::shared_ptr<Boolean>(True) : std::shared_ptr<Boolean>(False);
  } else if (op == "==") {
    return leftInteger->value == rightInteger->value ? std::shared_ptr<Boolean>(True) : std::shared_ptr<Boolean>(False);
  } else if (op == "!=") {
    return leftInteger->value != rightInteger->value ? std::shared_ptr<Boolean>(True) : std::shared_ptr<Boolean>(False);
  }

  return newError("unknown operator: " + left->type() + " " + op + " " + right->type());
}

std::shared_ptr<Object> Evaluator::evalBooleanInfixExpression(const std::string &op,
                                                              std::shared_ptr<Object> &left,
                                                              std::shared_ptr<Object> &right) {
  Boolean *leftBoolean = dynamic_cast<Boolean *>(left.get());
  Boolean *rightBoolean = dynamic_cast<Boolean *>(right.get());

  if (op == "==") {
    return leftBoolean->value == rightBoolean->value ? std::shared_ptr<Boolean>(True) : std::shared_ptr<Boolean>(False);
  } else if (op == "!=") {
    return leftBoolean->value != rightBoolean->value ? std::shared_ptr<Boolean>(True) : std::shared_ptr<Boolean>(False);
  }

  return newError("unknown operator: " + left->type() + " " + op + " " + right->type());
}

std::shared_ptr<Object> Evaluator::evalIfExpression(IfExpression *ie, std::unique_ptr<Environment> &env) {
  auto condition = eval(ie->condition.get(), env);

  if (condition == nullptr) {
    return nullptr;
  }

  Boolean *result = dynamic_cast<Boolean *>(condition.get());

  // Corner case: if (1) {10} else {20}
  Integer *integer = dynamic_cast<Integer *>(condition.get());

  if (result == nullptr && integer != nullptr) {
    if (integer->value != 0) {
      return eval(ie->consequence.get(), env);
    } else {
      return eval(ie->alternative.get(), env);
    }
  } else if (!result->value) {
    if (ie->alternative != nullptr) {
      return eval(ie->alternative.get(), env);
    }
  } else {
    return eval(ie->consequence.get(), env);
  }

  return nullptr;
}

std::shared_ptr<Object> Evaluator::evalBlockStatement(BlockStatement *bs, std::unique_ptr<Environment> &env) {
  std::shared_ptr<Object> result{};

  for (auto &&statement : bs->statements) {
    result = eval(statement.get(), env);

    if (result != nullptr) {
      if (result->type() == RETURN_VALUE_OBJ || result->type() == ERROR_OBJ)
        return result;
    }
  }

  return result;
}

std::shared_ptr<Object> Evaluator::evalIdentifier(Identifier *i, std::unique_ptr<Environment> &env) {
  auto result = env->get(i->value);
  if (result == nullptr) {
    return std::make_shared<Error>("identifier not found: " + i->value);
  }
  return result;
}

std::shared_ptr<Error> Evaluator::newError(const std::string &s) { return std::make_shared<Error>(s); }

std::vector<std::shared_ptr<Object>> Evaluator::evalArguments(std::vector<std::unique_ptr<Expression>> &arguments,
                                                              std::unique_ptr<Environment> &env) {
  std::vector<std::shared_ptr<Object>> results{};

  for (auto &&argument : arguments) {
    auto evaluated = eval(argument.get(), env);
    results.push_back(evaluated);
  }

  return results;
}

std::shared_ptr<Object> Evaluator::evalFunctions(Object *fn, std::vector<std::shared_ptr<Object>> &arguments) {
  Function *function = dynamic_cast<Function *>(fn);
  if (function == nullptr) {
    return newError("not a function: " + fn->type());
  }

  auto extendedEnv = std::make_unique<Environment>(&function->env);

  int i = 0;
  for (auto &&parameter : function->parameters) {
    extendedEnv->set(parameter->value, std::move(arguments[i]));
  }

  auto evaluated = eval(function->body.get(), extendedEnv);

  ReturnValue *returnValue = dynamic_cast<ReturnValue *>(evaluated.get());
  if (returnValue != nullptr) {
    return std::move(returnValue->value);
  }
  return std::move(evaluated);
}
