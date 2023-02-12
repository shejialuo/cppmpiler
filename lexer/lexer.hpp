#ifndef _LEXER_LEXER_HPP_
#define _LEXER_LEXER_HPP_

#include "token.hpp"
#include <string>

/**
 * @brief Lexer is the basic class
 *
 */
class Lexer {
private:
  std::string input; // input string
  int position;      // current position in input (points to current char)
  int nextPosition;  // current reading position in input (after current char)
  char ch;           // current char under examination
public:
  Lexer() = delete;
  Lexer(std::string i) : input(std::move(i)), position{}, nextPosition{}, ch{} {
    readChar();
  }
  Lexer(const Lexer &) = delete;
  Lexer(Lexer &&) = delete;

  /**
   * @brief read the current char, and move the `position`
   * and `nextPosition`
   *
   */
  void readChar();

  /**
   * @brief read the current char, produce the `Token` struct
   * and call `readChar`.
   *
   * @return Token
   */
  Token nextToken();
};

#endif // _LEXER_LEXER_HPP_
