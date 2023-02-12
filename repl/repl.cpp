#include "lexer.hpp"
#include "token.hpp"

#include <iostream>
#include <string>
#include <string_view>

std::string_view PROMPT{">> "};

void start() {
  std::string line{};
  while (true) {
    std::cout << PROMPT;
    if (!std::getline(std::cin, line)) {
      return;
    }
    Lexer l{line};

    for (Token token = l.nextToken(); token.Type != TokenTypes::_EOF; token = l.nextToken()) {
      std::cout << token << std::endl;
    }
  }
}
