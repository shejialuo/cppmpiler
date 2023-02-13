#include "lexer.hpp"

#include "token.hpp"

#include <cctype>
#include <functional>
#include <string>

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

  consecutiveSubstring(isWhitespace);

  switch (ch) {
    case '=':
      if (examineNextChar() == '=') {
        readChar();
        token.Type = TokenTypes::EQ;
        token.Literal = "==";
      } else {
        token.setToken(TokenTypes::ASSIGN, ch);
      }

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
    case '-':
      token.setToken(TokenTypes::MINUS, ch);
      break;
    case '!':
      if (examineNextChar() == '=') {
        readChar();
        token.Type = TokenTypes::NOT_EQ;
        token.Literal = "!=";
      } else {
        token.setToken(TokenTypes::BANG, ch);
      }
      break;
    case '/':
      token.setToken(TokenTypes::SLASH, ch);
      break;
    case '*':
      token.setToken(TokenTypes::ASTERISK, ch);
      break;
    case '<':
      token.setToken(TokenTypes::LT, ch);
      break;
    case '>':
      token.setToken(TokenTypes::GT, ch);
      break;
    case 0:
      token.Literal = "";
      token.Type = TokenTypes::_EOF;
      break;
    default:
      if (isLetter(ch)) {
        token.Literal = consecutiveSubstring(isLetter);
        token.setIdentifiers(token.Literal);
        return token;
      } else if (isDigit(ch)) {
        token.Type = TokenTypes::INT;
        token.Literal = consecutiveSubstring(isDigit);
        return token;
      } else {
        token.setToken(TokenTypes::ILLEGAL, ch);
      }
  }

  readChar();

  return token;
}

std::string Lexer::consecutiveSubstring(std::function<bool(char)> fn) {
  int originalPosition = position;

  while (fn(ch)) {
    readChar();
  }

  if (position >= input.size()) {
    return "";
  }

  return input.substr(originalPosition, position - originalPosition);
}

char Lexer::examineNextChar() {
  if (nextPosition >= input.size()) {
    return 0;
  } else {
    return input[nextPosition];
  }
}

static bool isLetter(char ch) { return std::isalpha(ch) || ch == '_'; }

static bool isDigit(char ch) { return std::isdigit(ch); }

static bool isWhitespace(char ch) { return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'; }
