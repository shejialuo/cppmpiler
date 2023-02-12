#ifndef _LEXER_LEXER_HPP_
#define _LEXER_LEXER_HPP_

#include "token.hpp"

#include <functional>
#include <string>

/**
 * @brief Lexer is the basic class
 *
 */
class Lexer {
private:
  std::string input;  // input string
  int position;       // current position in input (points to current char)
  int nextPosition;   // current reading position in input (after current char)
  char ch;            // current char under examination
public:
  Lexer() = delete;
  Lexer(std::string i) : input(std::move(i)), position{}, nextPosition{}, ch{} { readChar(); }
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

  /**
   * @brief get consecutive substring s, which all of the char c in s
   * satisfies fn(c) == true.
   *
   * @param fn a criterion function
   * @return std::string
   */
  std::string consecutiveSubstring(std::function<bool(char)> fn);
};

/**
 * @brief auxiliary functions to tell whether `ch` is a letter or '_'
 *
 */
static bool isLetter(char ch);

/**
 * @brief auxiliary functions to tell whether `ch` is a digit
 *
 */
static bool isDigit(char ch);

/**
 * @brief auxiliary functions to tell where `ch` is a generic whitespace
 *
 */
static bool isWhitespace(char ch);

#endif  // _LEXER_LEXER_HPP_
