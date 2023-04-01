#include "repl.hpp"

#include "compiler.hpp"
#include "evaluator.hpp"
#include "lexer.hpp"
#include "object.hpp"
#include "parser.hpp"
#include "symbolTable.hpp"
#include "token.hpp"
#include "vm.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

std::string_view PROMPT{">> "};

void startInterpreter() {
  Evaluator evaluator{};
  std::string line{};
  auto env = std::make_shared<Environment>();
  while (true) {
    std::cout << PROMPT;
    if (!std::getline(std::cin, line)) {
      return;
    }
    Lexer l{line};
    Parser p{&l};
    auto program = p.parseProgram();

    if (p.getErrors().size() != 0) {
      for (auto &&error : p.getErrors()) {
        std::cout << "\t" << error << "\n";
      }
      continue;
    }
    auto evaluated = evaluator.eval(program.get(), env);

    if (evaluated != nullptr) {
      std::cout << evaluated->inspect() << "\n";
    }
  }
}

void startCompiler() {
  std::string line{};

  std::vector<std::shared_ptr<Object>> constants{};
  auto globals = std::make_shared<std::vector<std::shared_ptr<Object>>>(65536);
  auto symbolTable = std::make_shared<SymbolTable>();

  while (true) {
    std::cout << PROMPT;
    if (!std::getline(std::cin, line)) {
      return;
    }
    Lexer l{line};
    Parser p{&l};
    auto program = p.parseProgram();

    if (p.getErrors().size() != 0) {
      for (auto &&error : p.getErrors()) {
        std::cout << "\t" << error << "\n";
      }
      continue;
    }

    Compiler compiler{constants, symbolTable};
    compiler.compile(program.get());

    constants = compiler.getBytecode().constants;

    VM machine{std::move(compiler.getBytecode().constants), globals, std::move(compiler.getBytecode().instructions)};

    machine.run();

    auto top = machine.lastPoppedStackElem();

    // There would be a situation when top is nullptr, we could evaluate
    // something useless, for example `if (1 > 2) { 10 }`.
    if (top != nullptr) {
      std::cout << top->inspect() << "\n";
    }
  }
}
