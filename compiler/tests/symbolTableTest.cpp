#include "symbolTable.hpp"

#include <gtest/gtest.h>
#include <string>
#include <unordered_map>
#include <vector>

TEST(SymbolTable, TestDefine) {
  SymbolTable global{};

  std::unordered_map<std::string, Symbol> expected{
      {"a", {"a", Symbol::globalScope, 0}},
      {"b", {"b", Symbol::globalScope, 1}},
  };

  Symbol &a = global.define("a");
  if (a != expected["a"]) {
    FAIL();
  }

  Symbol &b = global.define("b");

  if (b != expected["b"]) {
    FAIL();
  }
}

TEST(SymbolTable, TestResolveGlobal) {
  SymbolTable global{};
  global.define("a");
  global.define("b");

  std::vector<Symbol> expected{
      {"a", Symbol::globalScope, 0},
      {"b", Symbol::globalScope, 1},
  };

  for (auto &&symbol : expected) {
    auto result = global.resolve(symbol.name);
    if (!result.has_value()) {
      FAIL();
    }

    if (result.value().get() != symbol) {
      FAIL();
    }
  }
}
