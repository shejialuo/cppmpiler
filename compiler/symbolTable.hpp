#ifndef _COMPILER_SYMBOL_TABLE_HPP_
#define _COMPILER_SYMBOL_TABLE_HPP_

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct Symbol {
  static std::string globalScope;
  static std::string localScope;
  static std::string builtinScope;
  static std::string freeScope;

  std::string name;
  std::string symbolScope;
  int index;

  Symbol() = default;
  Symbol(const std::string &n, const std::string &s, int i) : name{n}, symbolScope{s}, index{i} {}

  bool operator==(const Symbol &other) {
    return name == other.name && symbolScope == other.symbolScope && index == other.index;
  }

  bool operator!=(const Symbol &other) { return !operator==(other); }
};

class SymbolTable {
private:
  std::shared_ptr<SymbolTable> outer{};
  std::unordered_map<std::string, Symbol> store{};
  int numDefinitions{};
  std::vector<Symbol> freeSymbols{};

public:
  SymbolTable() = default;
  SymbolTable(std::shared_ptr<SymbolTable> &o) : outer{o} {}

  inline std::shared_ptr<SymbolTable> &getOuter() { return outer; }
  inline int getNumDefinition() { return numDefinitions; }
  inline std::vector<Symbol> &getFreeSymbols() { return freeSymbols; }

  Symbol &define(const std::string &name);
  Symbol &defineBuiltin(int index, const std::string &name);
  Symbol &defineFree(const Symbol &freeSymbol);
  std::optional<std::reference_wrapper<Symbol>> resolve(const std::string &name);
};

#endif  // _COMPILER_SYMBOL_TABLE_HPP_
