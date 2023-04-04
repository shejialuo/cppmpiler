#include "symbolTable.hpp"

#include <functional>
#include <optional>

std::string Symbol::globalScope{"GLOBAL"};
std::string Symbol::localScope{"LOCAL"};

Symbol &SymbolTable::define(const std::string &name) {
  Symbol symbol{name, Symbol::globalScope, numDefinitions};

  if (outer != nullptr) {
    symbol.symbolScope = Symbol::localScope;
  }

  store[name] = std::move(symbol);
  numDefinitions++;
  return store[name];
}

std::optional<std::reference_wrapper<Symbol>> SymbolTable::resolve(const std::string &name) {
  if (store.find(name) != store.end()) {
    return store[name];
  } else {
    if (outer != nullptr) {
      return outer->resolve(name);
    }
  }
  return {};
}
