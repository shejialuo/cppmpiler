#ifndef _TOKEN_TOKEN_HPP_
#define _TOKEN_TOKEN_HPP_

#include <string>
#include <string_view>

// For simplicity, we define all the `TokenType` to be the `std::string`
using TokenType_t = std::string;

/**
 * @brief TokenTypes is used to wrap the type definition
 *
 */
struct TokenTypes {
  static std::string_view ILLEGAL;
  static std::string_view _EOF;

  // Identifiers + literals
  static std::string_view IDENT;
  static std::string_view INT;

  // Operators
  static std::string_view ASSIGN;
  static std::string_view PLUS;

  // Delimiters
  static std::string_view COMMA;
  static std::string_view SEMICOLON;

  static std::string_view LPAREN;
  static std::string_view RPAREN;
  static std::string_view LBRACE;
  static std::string_view RBRACE;

  // Keywords
  static std::string_view FUNCTION;
  static std::string_view LET;
};

/**
 * @brief Token is a data structure which represents the token.
 * It has two fields, one is its type, another is its literal.
 * They are nearly the same.
 */
struct Token {
  TokenType_t Type;
  std::string Literal;

  Token() = default;
  Token(TokenType_t &t, std::string &l) : Type{t}, Literal{l} {}
  void setToken(std::string_view &t, char ch);
};

#endif // _TOKEN_TOKEN_HPP_
