#ifndef _TOKEN_TOKEN_HPP_
#define _TOKEN_TOKEN_HPP_

#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

// For simplicity, we define all the `TokenType` to be the `std::string`
using TokenType_t = std::string;

/**
 * @brief TokenTypes is used to wrap the type definition
 *
 */
struct TokenTypes {
  static std::string_view ILLEGAL;
  static std::string_view _EOF;

  // Identifiers + literals + Strings + Arrays
  static std::string_view IDENT;
  static std::string_view INT;
  static std::string_view STRING;
  static std::string_view LBRACKET;
  static std::string_view RBRACKET;

  // Operators
  static std::string_view ASSIGN;
  static std::string_view PLUS;
  static std::string_view MINUS;
  static std::string_view BANG;
  static std::string_view ASTERISK;
  static std::string_view SLASH;
  static std::string_view EQ;
  static std::string_view NOT_EQ;

  static std::string_view LT;
  static std::string_view GT;

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
  static std::string_view TRUE;
  static std::string_view FALSE;
  static std::string_view IF;
  static std::string_view ELSE;
  static std::string_view RETURN;
};

/**
 * @brief Token is a data structure which represents the token.
 * It has two fields, one is its type, another is its literal.
 * They are nearly the same.
 */
struct Token {
  TokenType_t Type;
  std::string Literal;

  static std::unordered_map<std::string, TokenType_t> keywords;

  Token() = default;
  Token(TokenType_t &t, std::string &l) : Type{t}, Literal{l} {}
  Token(const Token &);
  void setToken(std::string_view &t, char ch);

  /**
   * @brief auxiliary functions to set the keywords
   *
   */
  void setIdentifiers(std::string &identifiers);
};

std::ostream &operator<<(std::ostream &os, Token &token);

#endif  // _TOKEN_TOKEN_HPP_
