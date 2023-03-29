#include "repl.hpp"

#include "compiler.hpp"
#include "evaluator.hpp"
#include "lexer.hpp"
#include "parser.hpp"
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

    Compiler compiler{};
    compiler.compile(program.get());

    VM machine{std::move(compiler.getBytecode().constants), std::move(compiler.getBytecode().instructions)};

    machine.run();

    auto top = machine.stackTop();
    std::cout << top->inspect() << "\n";
  }
}
