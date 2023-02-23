#include "evaluator.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

std::string_view PROMPT{">> "};

void start() {
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
