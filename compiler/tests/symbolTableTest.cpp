#include "symbolTable.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

TEST(SymbolTable, TestDefine) {
  auto global = std::make_shared<SymbolTable>();

  std::unordered_map<std::string, Symbol> expected{
      {"a", {"a", Symbol::globalScope, 0}},
      {"b", {"b", Symbol::globalScope, 1}},
      {"c", {"c", Symbol::localScope, 0}},
      {"d", {"d", Symbol::localScope, 1}},
      {"e", {"e", Symbol::localScope, 0}},
      {"f", {"f", Symbol::localScope, 1}},
  };

  Symbol &a = global->define("a");
  if (a != expected["a"]) {
    FAIL();
  }

  Symbol &b = global->define("b");

  if (b != expected["b"]) {
    FAIL();
  }

  auto firstLocal = std::make_shared<SymbolTable>(global);

  Symbol &c = firstLocal->define("c");
  if (c != expected["c"]) {
    FAIL();
  }

  Symbol &d = firstLocal->define("d");
  if (d != expected["d"]) {
    FAIL();
  }

  auto secondLocal = std::make_shared<SymbolTable>(firstLocal);

  Symbol &e = secondLocal->define("e");
  if (e != expected["e"]) {
    FAIL();
  }

  Symbol &f = secondLocal->define("f");
  if (f != expected["f"]) {
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

TEST(SymbolTable, TestResolveLocal) {
  auto global = std::make_shared<SymbolTable>();

  global->define("a");
  global->define("b");

  auto local = std::make_shared<SymbolTable>(global);
  local->define("c");
  local->define("d");

  std::vector<Symbol> expected{
      {"a", Symbol::globalScope, 0},
      {"b", Symbol::globalScope, 1},
      {"c", Symbol::localScope, 0},
      {"d", Symbol::localScope, 1},
  };

  for (auto &&symbol : expected) {
    auto result = local->resolve(symbol.name);
    if (!result.has_value()) {
      FAIL();
    }

    if (result.value().get() != symbol) {
      FAIL();
    }
  }
}

TEST(SymbolTable, TestDefineResolveBuiltins) {
  auto global = std::make_shared<SymbolTable>();
  auto firstLocal = std::make_shared<SymbolTable>(global);
  auto secondLocal = std::make_shared<SymbolTable>(firstLocal);

  std::vector<Symbol> expected{
      {"a", Symbol::builtinScope, 0},
      {"c", Symbol::builtinScope, 1},
      {"e", Symbol::builtinScope, 2},
      {"f", Symbol::builtinScope, 3},
  };

  for (int i = 0; i < expected.size(); i++) {
    global->defineBuiltin(i, expected[i].name);
  }

  std::vector<std::shared_ptr<SymbolTable>> tests{global, firstLocal, secondLocal};

  for (auto &&test : tests) {
    for (auto &&symbol : expected) {
      auto result = test->resolve(symbol.name);
      if (!result.has_value()) {
        FAIL();
      }

      if (result.value().get() != symbol) {
        FAIL();
      }
    }
  }
}

TEST(SymbolTable, TestResolveFree) {
  auto global = std::make_shared<SymbolTable>();
  auto firstLocal = std::make_shared<SymbolTable>(global);
  auto secondLocal = std::make_shared<SymbolTable>(firstLocal);

  global->define("a");
  global->define("b");

  firstLocal->define("c");
  firstLocal->define("d");

  secondLocal->define("e");
  secondLocal->define("f");

  struct TestData {
    std::shared_ptr<SymbolTable> table;
    std::vector<Symbol> expectedSymbols;
    std::vector<Symbol> expectedFreeSymbols;

    TestData() = default;
  };

  std::vector<TestData> tests{
      {
          firstLocal,
          {
              {"a", Symbol::globalScope, 0},
              {"b", Symbol::globalScope, 1},
              {"c", Symbol::localScope, 0},
              {"d", Symbol::localScope, 1},
          },
          {},
      },
      {
          secondLocal,
          {
              {"a", Symbol::globalScope, 0},
              {"b", Symbol::globalScope, 1},
              {"c", Symbol::freeScope, 0},
              {"d", Symbol::freeScope, 1},
              {"e", Symbol::localScope, 0},
              {"f", Symbol::localScope, 1},
          },
          {
              {"c", Symbol::localScope, 0},
              {"d", Symbol::localScope, 1},
          },
      },
  };

  for (auto &&test : tests) {
    for (auto &&symbol : test.expectedSymbols) {
      auto result = test.table->resolve(symbol.name);
      if (!result.has_value()) {
        FAIL();
      }

      if (result.value().get() != symbol) {
        FAIL();
      }
    }

    for (int i = 0; i < test.expectedFreeSymbols.size(); i++) {
      auto &&result = test.table->getFreeSymbols()[i];
      if (result != test.expectedFreeSymbols[i]) {
        FAIL();
      }
    }
  }
}
