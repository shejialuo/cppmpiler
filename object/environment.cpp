#include "environment.hpp"

#include "object.hpp"

#include <memory>

Environment::Environment() {}

std::unique_ptr<Object> Environment::get(const std::string &name) {
  // Here, I will allocate each object with unique_ptr, although this
  // is bad for performance, but I can make no memory leak. However,
  // we really should use shared_ptr here. But I don't want the dependency
  // be terrible.
  if (store.count(name)) {
    auto object = store[name].get();

    Integer *integer = dynamic_cast<Integer *>(object);
    if (integer != nullptr) {
      return std::make_unique<Integer>(integer->value);
    }

    Boolean *boolean = dynamic_cast<Boolean *>(object);
    if (boolean != nullptr) {
      return std::make_unique<Boolean>(boolean->value);
    }
  }
  return nullptr;
}

void Environment::set(const std::string &name, std::unique_ptr<Object> &&val) { store[name] = std::move(val); }
