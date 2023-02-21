#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

std::string_view PROMPT{">> "};

void start() {
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

    std::cout << program->getString() << std::endl;
  }
}
