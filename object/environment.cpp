#include "ast.hpp"
#include "object.hpp"

#include <memory>

Environment::Environment(std::unique_ptr<Environment> *o) : outer(o) {}

std::unique_ptr<Object> Environment::get(const std::string &name) {
  if (store.count(name)) {
    auto object = store[name].get();
    // return &object;

    Integer *integer = dynamic_cast<Integer *>(object);
    if (integer != nullptr) {
      return std::make_unique<Integer>(integer->value);
    }

    Boolean *boolean = dynamic_cast<Boolean *>(object);
    if (boolean != nullptr) {
      return std::make_unique<Boolean>(boolean->value);
    }

    Function *function = dynamic_cast<Function *>(object);
    if (function != nullptr) {
      auto result = std::make_unique<Function>();
    }

  } else if (outer != nullptr) {
    return (*outer)->get(name);
  }
  return nullptr;
}

void Environment::set(const std::string &name, std::unique_ptr<Object> &&val) { store[name] = std::move(val); }
