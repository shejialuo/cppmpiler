#include "lexer.hpp"
#include "spdlog/spdlog.h"
#include "token.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

struct TestToken {
  TokenType_t expectedType;
  std::string expectedLiteral;

  TestToken(std::string_view &t, std::string l)
      : expectedType{t}, expectedLiteral{l} {}
};

TEST(Lexer, TestNextToken) {
  std::string input{"=+(){},;"};

  std::vector<TestToken> tests{
      {TokenTypes::ASSIGN, "="}, {TokenTypes::PLUS, "+"},
      {TokenTypes::LPAREN, "("}, {TokenTypes::RPAREN, ")"},
      {TokenTypes::LBRACE, "{"}, {TokenTypes::RBRACE, "}"},
      {TokenTypes::COMMA, ","},  {TokenTypes::SEMICOLON, ";"},
      {TokenTypes::_EOF, ""},
  };

  Lexer l{input};

  for (int i = 0; i < tests.size(); ++i) {
    Token token = l.nextToken();

    TestToken &testToken = tests[i];

    if (token.Type != testToken.expectedType) {
      spdlog::error("test[{}] - token type wrong. expected='{}', got='{}'", i,
                    testToken.expectedType, token.Type);
      FAIL();
    }

    if (token.Literal != testToken.expectedLiteral) {
      spdlog::error("test[{}] - token literal wrong. expected='{}', got='{}'",
                    i, testToken.expectedLiteral, token.Literal);
      FAIL();
    }
  }
}