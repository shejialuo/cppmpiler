#include "symbolTable.hpp"

#include <functional>
#include <optional>

std::string Symbol::globalScope{"GLOBAL"};
std::string Symbol::localScope{"LOCAL"};
std::string Symbol::builtinScope{"BUILTIN"};
std::string Symbol::freeScope{"FREE"};

Symbol &SymbolTable::define(const std::string &name) {
  Symbol symbol{name, Symbol::globalScope, numDefinitions};

  if (outer != nullptr) {
    symbol.symbolScope = Symbol::localScope;
  }

  store[name] = std::move(symbol);
  numDefinitions++;
  return store[name];
}

Symbol &SymbolTable::defineBuiltin(int index, const std::string &name) {
  Symbol symbol{name, Symbol::builtinScope, index};
  store[name] = std::move(symbol);
  return store[name];
}

Symbol &SymbolTable::defineFree(const Symbol &freeSymbol) {
  freeSymbols.push_back(freeSymbol);

  Symbol symbol{freeSymbol.name, Symbol::freeScope, static_cast<int>(freeSymbols.size() - 1)};

  store[freeSymbol.name] = symbol;
  return store[freeSymbol.name];
}

std::optional<std::reference_wrapper<Symbol>> SymbolTable::resolve(const std::string &name) {
  if (store.find(name) != store.end()) {
    return store[name];
  } else {
    if (outer != nullptr) {
      auto obj = outer->resolve(name);
      if (obj.has_value()) {
        if (obj.value().get().symbolScope == Symbol::globalScope ||
            obj.value().get().symbolScope == Symbol::builtinScope) {
          return obj;
        }

        auto &freeSymbol = defineFree(obj.value().get());
        return freeSymbol;
      }
    }
  }
  return {};
}
