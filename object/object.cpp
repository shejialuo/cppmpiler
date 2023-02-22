#include "object.hpp"

#include <cstdint>
#include <string>
#include <string_view>

constexpr std::string_view INTEGER_OBJ = "INTEGER";
constexpr std::string_view BOOLEAN_OBJ = "BOOLEAN";
constexpr std::string_view RETURN_VALUE_OBJ = "RETURN_VALUE";

Integer::Integer(int64_t v) : value{v} {}
std::string Integer::inspect() { return std::to_string(value); }
ObjectType Integer::type() { return std::string(INTEGER_OBJ); }

Boolean::Boolean(bool v) : value{v} {}
std::string Boolean::inspect() { return value ? "true" : "false"; }
ObjectType Boolean::type() { return std::string(BOOLEAN_OBJ); }

std::string ReturnValue::inspect() { return value->inspect(); }
ObjectType ReturnValue::type() { return std::string(RETURN_VALUE_OBJ); }
