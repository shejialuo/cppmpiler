#include "ast.hpp"
#include "object.hpp"

#include <memory>

Environment::Environment(std::unique_ptr<Environment> *o) : outer(o) {}

std::shared_ptr<Object> Environment::get(const std::string &name) {
  if (store.count(name)) {
    auto object = store[name];
    return store[name];
  } else if (outer != nullptr) {
    return (*outer)->get(name);
  }
  return nullptr;
}

void Environment::set(const std::string &name, std::shared_ptr<Object> &&val) { store[name] = val; }
