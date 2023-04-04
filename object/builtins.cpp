#include "builtins.hpp"

#include <string_view>

static std::shared_ptr<Error> newError(const std::string &s);

static std::shared_ptr<Error> newError(const std::string &s) { return std::make_shared<Error>(s); }

constexpr std::string_view INTEGER_OBJ = "INTEGER";
constexpr std::string_view BOOLEAN_OBJ = "BOOLEAN";
constexpr std::string_view RETURN_VALUE_OBJ = "RETURN_VALUE";
constexpr std::string_view ERROR_OBJ = "ERROR";
constexpr std::string_view STRING_OBJ = "STRING";
constexpr std::string_view ARRAY_OBJ = "ARRAY";

std::unordered_map<std::string, std::shared_ptr<Builtin>> Builtins::builtins = {
    {"len", std::make_shared<Builtin>(len)},
    {"first", std::make_shared<Builtin>(first)},
    {"last", std::make_shared<Builtin>(last)},
    {"rest", std::make_shared<Builtin>(rest)},
    {"push", std::make_shared<Builtin>(push)},
};

std::vector<std::string> Builtins::builtinNames = {"len", "first", "last", "rest", "push"};

std::shared_ptr<Object> Builtins::len(std::vector<std::shared_ptr<Object>> &arguments) {
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

std::shared_ptr<Object> Builtins::first(std::vector<std::shared_ptr<Object>> &arguments) {
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

std::shared_ptr<Object> Builtins::last(std::vector<std::shared_ptr<Object>> &arguments) {
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

std::shared_ptr<Object> Builtins::rest(std::vector<std::shared_ptr<Object>> &arguments) {
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

std::shared_ptr<Object> Builtins::push(std::vector<std::shared_ptr<Object>> &arguments) {
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
