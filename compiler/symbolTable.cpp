#include "symbolTable.hpp"

#include <functional>
#include <optional>

std::string Symbol::globalScope{"GLOBAL"};

Symbol &SymbolTable::define(const std::string &name) {
  Symbol symbol{name, Symbol::globalScope, numDefinitions};
  store[name] = std::move(symbol);
  numDefinitions++;
  return store[name];
}

std::optional<std::reference_wrapper<Symbol>> SymbolTable::resolve(const std::string &name) {
  if (store.find(name) != store.end()) {
    return store[name];
  }
  return {};
}
