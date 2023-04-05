#include "object.hpp"

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

constexpr std::string_view INTEGER_OBJ = "INTEGER";
constexpr std::string_view BOOLEAN_OBJ = "BOOLEAN";
constexpr std::string_view RETURN_VALUE_OBJ = "RETURN_VALUE";
constexpr std::string_view FUNCTION_OBJ = "FUNCTION";
constexpr std::string_view ERROR_OBJ = "ERROR";
constexpr std::string_view STRING_OBJ = "STRING";
constexpr std::string_view BUILTIN_OBJ = "BUILTIN";
constexpr std::string_view ARRAY_OBJ = "ARRAY";
constexpr std::string_view COMPILED_FUNCTION_OBJ = "COMPILED_FUNCTION";
constexpr std::string_view CLOSURE_OBJ = "CLOSURE";

Integer::Integer(int64_t v) : value{v} {}
std::string Integer::inspect() { return std::to_string(value); }
ObjectType Integer::type() { return std::string(INTEGER_OBJ); }

Boolean::Boolean(bool v) : value{v} {}
std::string Boolean::inspect() { return value ? "true" : "false"; }
ObjectType Boolean::type() { return std::string(BOOLEAN_OBJ); }

std::string ReturnValue::inspect() { return value->inspect(); }
ObjectType ReturnValue::type() { return std::string(RETURN_VALUE_OBJ); }

Error::Error(const std::string &m) : message{m} {}
std::string Error::inspect() { return "ERROR: " + message; }
ObjectType Error::type() { return std::string(ERROR_OBJ); }

String::String(const std::string &s) : value{s} {}
std::string String::inspect() { return value; }
ObjectType String::type() { return std::string(STRING_OBJ); }

Function::Function(std::vector<std::unique_ptr<Identifier>> &&p,
                   std::unique_ptr<BlockStatement> &&b,
                   std::shared_ptr<Environment> e) {
  parameters = std::move(p);
  body = std::move(b);

  // set the current environment
  env = e;
}
std::string Function::inspect() {
  std::string info{};
  info += "fn(";

  if (!parameters.empty()) {
    int i = 0;
    for (; i < parameters.size() - 1; ++i) {
      info += parameters[i]->getString();
      info += ", ";
    }

    info += parameters[i]->getString();
  }
  info += ") {\n";
  info += body->getString();
  info += "\n}";

  return info;
}
ObjectType Function::type() { return std::string(FUNCTION_OBJ); }

CompiledFunction::CompiledFunction(Instructions &&i, int numLocals_) : numLocals{numLocals_} {
  instructions = std::move(i);
}

std::string CompiledFunction::inspect() {
  const void *address = static_cast<const void *>(this);

  std::stringstream ss;
  ss << address;

  std::string info = "CompiledFunction" + ss.str();

  return info;
}
ObjectType CompiledFunction::type() { return std::string(COMPILED_FUNCTION_OBJ); }

ObjectType Closure::type() { return std::string(CLOSURE_OBJ); }
std::string Closure::inspect() {
  const void *address = static_cast<const void *>(this);

  std::stringstream ss;
  ss << address;

  std::string info = "Closure" + ss.str();

  return info;
}

Builtin::Builtin(BuiltinFunction f) : fn{f} {}
std::string Builtin::inspect() { return "builtin function"; }
ObjectType Builtin::type() { return std::string(BUILTIN_OBJ); }

std::string Array::inspect() {
  if (elements.empty()) {
    return "[]";
  }

  std::string info = "[";

  int i = 0;
  for (; i < elements.size() - 1; ++i) {
    info += elements[i]->inspect() + ", ";
  }

  info += elements[i]->inspect() + "]";
  return info;
}
ObjectType Array::type() { return std::string(ARRAY_OBJ); }
