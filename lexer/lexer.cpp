#include "lexer.hpp"
#include "token.hpp"

void Lexer::readChar() {
  if (nextPosition >= input.size()) {
    ch = 0;
  } else {
    ch = input[nextPosition];
  }
  position = nextPosition;
  nextPosition++;
}

Token Lexer::nextToken() {
  Token token{};

  switch (ch) {
  case '=':
    token.setToken(TokenTypes::ASSIGN, ch);
    break;
  case ';':
    token.setToken(TokenTypes::SEMICOLON, ch);
    break;
  case '(':
    token.setToken(TokenTypes::LPAREN, ch);
    break;
  case ')':
    token.setToken(TokenTypes::RPAREN, ch);
    break;
  case ',':
    token.setToken(TokenTypes::COMMA, ch);
    break;
  case '+':
    token.setToken(TokenTypes::PLUS, ch);
    break;
  case '{':
    token.setToken(TokenTypes::LBRACE, ch);
    break;
  case '}':
    token.setToken(TokenTypes::RBRACE, ch);
    break;
  case 0:
    token.Literal = "";
    token.Type = TokenTypes::_EOF;
    break;
  }

  readChar();

  return token;
}
