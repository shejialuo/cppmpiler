#ifndef _COMPILER_SYMBOL_TABLE_HPP_
#define _COMPILER_SYMBOL_TABLE_HPP_

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

struct Symbol {
  static std::string globalScope;
  static std::string localScope;

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

public:
  SymbolTable() = default;
  SymbolTable(std::shared_ptr<SymbolTable> &o) : outer{o} {}

  inline std::shared_ptr<SymbolTable> &getOuter() { return outer; }
  inline int getNumDefinition() { return numDefinitions; }

  Symbol &define(const std::string &name);
  std::optional<std::reference_wrapper<Symbol>> resolve(const std::string &name);
};

#endif  // _COMPILER_SYMBOL_TABLE_HPP_
