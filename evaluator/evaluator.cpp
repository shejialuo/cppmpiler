#include "evaluator.hpp"

#include "ast.hpp"
#include "object.hpp"
#include "spdlog/spdlog.h"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

constexpr std::string_view INTEGER_OBJ = "INTEGER";
constexpr std::string_view BOOLEAN_OBJ = "BOOLEAN";
constexpr std::string_view RETURN_VALUE_OBJ = "RETURN_VALUE";
constexpr std::string_view ERROR_OBJ = "ERROR";
constexpr std::string_view STRING_OBJ = "STRING";
constexpr std::string_view ARRAY_OBJ = "ARRAY";

std::shared_ptr<Boolean> Evaluator::True = std::make_shared<Boolean>(true);
std::shared_ptr<Boolean> Evaluator::False = std::make_shared<Boolean>(false);
std::vector<std::shared_ptr<Environment>> Evaluator::environments = {};

std::unordered_map<std::string, std::shared_ptr<Builtin>> Evaluator::builtins = {
    {"len", std::make_shared<Builtin>(len)},
    {"first", std::make_shared<Builtin>(first)},
    {"last", std::make_shared<Builtin>(last)},
    {"rest", std::make_shared<Builtin>(rest)},
    {"push", std::make_shared<Builtin>(push)},
};

std::shared_ptr<Object> Evaluator::eval(Node *node, std::shared_ptr<Environment> &env) {
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
    auto function =
        std::make_shared<Function>(std::move(functionLiteral->parameters), std::move(functionLiteral->body), env);
    return function;
  }

  CallExpression *callExpression = dynamic_cast<CallExpression *>(node);
  if (callExpression != nullptr) {
    auto function = eval(callExpression->function.get(), env);
    auto arguments = evalExpressions(callExpression->arguments, env);
    return evalFunctions(function.get(), arguments);
  }

  StringLiteral *stringLiteral = dynamic_cast<StringLiteral *>(node);
  if (stringLiteral != nullptr) {
    return std::make_shared<String>(stringLiteral->value);
  }

  ArrayLiteral *arrayLiteral = dynamic_cast<ArrayLiteral *>(node);
  if (arrayLiteral != nullptr) {
    auto elements = evalExpressions(arrayLiteral->elements, env);
    auto result = std::make_shared<Array>();
    result->elements = std::move(elements);
    return result;
  }

  IndexExpression *indexExpression = dynamic_cast<IndexExpression *>(node);
  if (indexExpression != nullptr) {
    auto left = eval(indexExpression->left.get(), env);
    auto index = eval(indexExpression->index.get(), env);
    return evalIndexExpression(left, index);
  }

  return nullptr;
}

std::shared_ptr<Object> Evaluator::evalProgram(std::vector<std::unique_ptr<Statement>> &statements,
                                               std::shared_ptr<Environment> &env) {
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
  } else if (left->type() == STRING_OBJ && right->type() == STRING_OBJ) {
    return std::move(evalStringInfixExpression(op, left, right));
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

std::shared_ptr<Object> Evaluator::evalStringInfixExpression(const std::string &op,
                                                             std::shared_ptr<Object> &left,
                                                             std::shared_ptr<Object> &right) {
  if (op != "+") {
    return newError("unknown operator: " + left->type() + " " + op + " " + right->type());
  }

  String *leftString = dynamic_cast<String *>(left.get());
  String *rightString = dynamic_cast<String *>(right.get());

  return std::make_shared<String>(leftString->value + rightString->value);
}

std::shared_ptr<Object> Evaluator::evalIfExpression(IfExpression *ie, std::shared_ptr<Environment> &env) {
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

std::shared_ptr<Object> Evaluator::evalBlockStatement(BlockStatement *bs, std::shared_ptr<Environment> &env) {
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

std::shared_ptr<Object> Evaluator::evalIdentifier(Identifier *i, std::shared_ptr<Environment> &env) {
  auto result = env->get(i->value);
  if (result == nullptr) {
    auto builtin = builtins[i->value];
    if (builtin != nullptr) {
      return builtin;
    }
    return std::make_shared<Error>("identifier not found: " + i->value);
  }
  return result;
}

std::shared_ptr<Error> Evaluator::newError(const std::string &s) { return std::make_shared<Error>(s); }

std::vector<std::shared_ptr<Object>> Evaluator::evalExpressions(std::vector<std::unique_ptr<Expression>> &arguments,
                                                                std::shared_ptr<Environment> &env) {
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
    Builtin *builtin = dynamic_cast<Builtin *>(fn);
    if (builtin != nullptr) {
      return builtin->fn(arguments);
    }
    return newError("not a function: " + fn->type());
  }

  auto extendedEnv = std::make_shared<Environment>(function->env.lock());

  int i = 0;
  for (auto &&parameter : function->parameters) {
    extendedEnv->set(parameter->value, arguments[i++]);
  }

  auto evaluated = eval(function->body.get(), extendedEnv);

  // Here, we must push this ptr into the environ vector, because we use
  // weak_ptr for function, we should keep its lifetime.
  environments.push_back(extendedEnv);

  ReturnValue *returnValue = dynamic_cast<ReturnValue *>(evaluated.get());
  if (returnValue != nullptr) {
    return returnValue->value;
  }
  return evaluated;
}

std::shared_ptr<Object> Evaluator::evalIndexExpression(std::shared_ptr<Object> left, std::shared_ptr<Object> index) {
  if (left->type() == ARRAY_OBJ && index->type() == INTEGER_OBJ) {
    return evalArrayIndexExpression(left, index);
  }

  return newError("index operator not supported: " + left->type());
}

std::shared_ptr<Object> Evaluator::evalArrayIndexExpression(std::shared_ptr<Object> left,
                                                            std::shared_ptr<Object> index) {
  Array *array = dynamic_cast<Array *>(left.get());

  Integer *integer = dynamic_cast<Integer *>(index.get());

  if (integer->value < 0 || integer->value > array->elements.size() - 1) {
    return nullptr;
  }

  return array->elements[integer->value];
}

std::shared_ptr<Object> Evaluator::len(std::vector<std::shared_ptr<Object>> &arguments) {
  if (arguments.size() != 1) {
    return newError("wrong number of arguments. got=" + std::to_string(arguments.size()) + ", want=1");
  }

  String *str = dynamic_cast<String *>(arguments[0].get());
  if (str == nullptr) {
    Array *array = dynamic_cast<Array *>(arguments[0].get());
    if (array != nullptr) {
      return std::make_shared<Integer>(array->elements.size());
    }
    return newError("argument to len not supported, got " + arguments[0]->type());
  }

  return std::make_shared<Integer>(str->value.size());
}

std::shared_ptr<Object> Evaluator::first(std::vector<std::shared_ptr<Object>> &arguments) {
  if (arguments.size() != 1) {
    return newError("wrong number of arguments. got=" + std::to_string(arguments.size()) + ", want=1");
  }

  if (arguments[0]->type() != ARRAY_OBJ) {
    return newError("argument to first must be ARRAY");
  }

  Array *array = dynamic_cast<Array *>(arguments[0].get());
  if (array->elements.size() > 0) {
    return array->elements[0];
  }
  return nullptr;
}

std::shared_ptr<Object> Evaluator::last(std::vector<std::shared_ptr<Object>> &arguments) {
  if (arguments.size() != 1) {
    return newError("wrong number of arguments. got=" + std::to_string(arguments.size()) + ", want=1");
  }

  if (arguments[0]->type() != ARRAY_OBJ) {
    return newError("argument to first must be ARRAY");
  }

  Array *array = dynamic_cast<Array *>(arguments[0].get());
  if (array->elements.size() > 0) {
    return array->elements[array->elements.size() - 1];
  }
  return nullptr;
}

std::shared_ptr<Object> Evaluator::rest(std::vector<std::shared_ptr<Object>> &arguments) {
  if (arguments.size() != 1) {
    return newError("wrong number of arguments. got=" + std::to_string(arguments.size()) + ", want=1");
  }

  if (arguments[0]->type() != ARRAY_OBJ) {
    return newError("argument to first must be ARRAY");
  }

  Array *array = dynamic_cast<Array *>(arguments[0].get());
  int length = array->elements.size();
  if (length > 0) {
    auto result = std::make_shared<Array>();
    for (int i = 1; i < length; ++i) {
      result->elements.push_back(array->elements[i]);
    }
    return result;
  }

  return nullptr;
}

std::shared_ptr<Object> Evaluator::push(std::vector<std::shared_ptr<Object>> &arguments) {
  if (arguments.size() != 2) {
    return newError("wrong number of arguments. got=" + std::to_string(arguments.size()) + ", want=1");
  }

  if (arguments[0]->type() != ARRAY_OBJ) {
    return newError("argument to first must be ARRAY");
  }

  Array *array = dynamic_cast<Array *>(arguments[0].get());
  int length = array->elements.size();

  auto result = std::make_shared<Array>();
  for (int i = 0; i < length; ++i) {
    result->elements.push_back(array->elements[i]);
  }
  result->elements.push_back(arguments[1]);
  return result;
}
