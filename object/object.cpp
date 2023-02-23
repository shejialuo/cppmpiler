#include "object.hpp"

#include <cstdint>
#include <string>
#include <string_view>

constexpr std::string_view INTEGER_OBJ = "INTEGER";
constexpr std::string_view BOOLEAN_OBJ = "BOOLEAN";
constexpr std::string_view RETURN_VALUE_OBJ = "RETURN_VALUE";
constexpr std::string_view FUNCTION_OBJ = "FUNCTION";
constexpr std::string_view ERROR_OBJ = "ERROR";

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

Function::Function(std::vector<std::unique_ptr<Identifier>> &p,
                   std::unique_ptr<BlockStatement> &&b,
                   std::unique_ptr<Environment> &e) {
  parameters = std::move(p);
  body = std::move(b);

  // Set the outer
  env = std::make_unique<Environment>(&e);
}
std::string Function::inspect() {
  std::string info{};
  info += "fn(";

  int i = 0;
  for (; i < parameters.size() - 1; ++i) {
    info += parameters[i]->getString();
    info += ", ";
  }

  info += parameters[i]->getString();
  info += ") {\n";
  info += body->getString();
  info += "\n}";

  return info;
}
ObjectType Function::type() { return std::string(FUNCTION_OBJ); }
