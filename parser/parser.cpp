#include "parser.hpp"

#include "ast.hpp"
#include "lexer.hpp"
#include "token.hpp"

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string_view>
#include <unordered_map>

/**
 * @brief define the precedence
 *
 */
enum class Precedence {
  LOWEST = 0,
  EQUALS,       // ==
  LESSGREATER,  // > or <
  SUM,          // +
  PRODUCT,      // *
  PREFIX,       // -X or !x
  CALL,         // myFunction(X)
};

Parser::Parser(Lexer *l) : lexer{l}, prefixParseFns{}, infixParseFns{} {
  // Read two tokens, set `currentToken` and `peekToken`.
  nextToken();
  nextToken();

  precedences = {
      {std::string(TokenTypes::EQ), Precedence::EQUALS},
      {std::string(TokenTypes::NOT_EQ), Precedence::EQUALS},
      {std::string(TokenTypes::LT), Precedence::LESSGREATER},
      {std::string(TokenTypes::GT), Precedence::LESSGREATER},
      {std::string(TokenTypes::PLUS), Precedence::SUM},
      {std::string(TokenTypes::MINUS), Precedence::SUM},
      {std::string(TokenTypes::SLASH), Precedence::PRODUCT},
      {std::string(TokenTypes::ASTERISK), Precedence::PRODUCT},
  };

  using std::placeholders::_1;

  registerPrefix(std::string(TokenTypes::IDENT), std::bind(&Parser::parseIdentifier, this));
  registerPrefix(std::string(TokenTypes::INT), std::bind(&Parser::parseIntegerLiteral, this));
  registerPrefix(std::string(TokenTypes::BANG), std::bind(&Parser::parsePrefixExpression, this));
  registerPrefix(std::string(TokenTypes::MINUS), std::bind(&Parser::parsePrefixExpression, this));
  registerPrefix(std::string(TokenTypes::TRUE), std::bind(&Parser::parseBooleanExpression, this));
  registerPrefix(std::string(TokenTypes::FALSE), std::bind(&Parser::parseBooleanExpression, this));
  registerPrefix(std::string(TokenTypes::LPAREN), std::bind(&Parser::ParseGroupedExpression, this));
  registerPrefix(std::string(TokenTypes::IF), std::bind(&Parser::parseIfExpression, this));

  registerInfix(std::string(TokenTypes::PLUS), std::bind(&Parser::parseInfixExpression, this, _1));
  registerInfix(std::string(TokenTypes::MINUS), std::bind(&Parser::parseInfixExpression, this, _1));
  registerInfix(std::string(TokenTypes::SLASH), std::bind(&Parser::parseInfixExpression, this, _1));
  registerInfix(std::string(TokenTypes::ASTERISK), std::bind(&Parser::parseInfixExpression, this, _1));
  registerInfix(std::string(TokenTypes::EQ), std::bind(&Parser::parseInfixExpression, this, _1));
  registerInfix(std::string(TokenTypes::NOT_EQ), std::bind(&Parser::parseInfixExpression, this, _1));
  registerInfix(std::string(TokenTypes::LT), std::bind(&Parser::parseInfixExpression, this, _1));
  registerInfix(std::string(TokenTypes::GT), std::bind(&Parser::parseInfixExpression, this, _1));
}

void Parser::nextToken() {
  currentToken = peekToken;
  peekToken = lexer->nextToken();
}

std::unique_ptr<Program> Parser::parseProgram() {
  auto program = std::make_unique<Program>();

  while (currentToken.Type != TokenTypes::_EOF) {
    auto statement = parseStatement();
    if (statement != nullptr) {
      program->statements.push_back(std::move(statement));
    }
    nextToken();
  }

  return std::move(program);
}

std::unique_ptr<Statement> Parser::parseStatement() {
  if (currentToken.Type == TokenTypes::LET) {
    return std::move(parseLetStatement());
  } else if (currentToken.Type == TokenTypes::RETURN) {
    return std::move(parseReturnStatement());
  } else {
    return std::move(parseExpressionStatement());
  }
}

std::unique_ptr<LetStatement> Parser::parseLetStatement() {
  auto letStatement = std::make_unique<LetStatement>(currentToken);

  if (!expectPeek(TokenTypes::IDENT)) {
    return nullptr;
  }

  letStatement->name = std::make_unique<Identifier>(currentToken, currentToken.Literal);

  if (!expectPeek(TokenTypes::ASSIGN)) {
    return nullptr;
  }

  // TODO: skip the expressions until encountering a semicolon.
  while (!currentTokenIs(TokenTypes::SEMICOLON)) {
    nextToken();
  }

  return std::move(letStatement);
}

std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {
  auto returnStatement = std::make_unique<ReturnStatement>(currentToken);

  nextToken();

  // TODO: skip the expressions until encountering a semicolon.

  while (!currentTokenIs(TokenTypes::SEMICOLON)) {
    nextToken();
  }

  return std::move(returnStatement);
}

std::unique_ptr<PrefixExpression> Parser::parsePrefixExpression() {
  auto prefixExpression = std::make_unique<PrefixExpression>(currentToken, currentToken.Literal);

  nextToken();

  prefixExpression->right = std::move(parseExpression(Precedence::PREFIX));

  return std::move(prefixExpression);
}

std::unique_ptr<InfixExpression> Parser::parseInfixExpression(std::unique_ptr<Expression> left) {
  auto infixExpression = std::make_unique<InfixExpression>(currentToken, currentToken.Literal);

  infixExpression->left = std::move(left);
  Precedence precedence = currentPrecedence();
  nextToken();
  infixExpression->right = parseExpression(precedence);

  return std::move(infixExpression);
}

std::unique_ptr<Expression> Parser::parseExpression(Precedence precedence) {
  // The first thing is to check whether there is a `prefixParseFn` associated
  // with the currentTokenType.
  if (!prefixParseFns.count(currentToken.Type)) {
    noPrefixParseFnError(currentToken.Type);
    return nullptr;
  }

  // We call the corresponding registered functions
  auto &&prefix = prefixParseFns[currentToken.Type];

  auto leftExpression = prefix();

  // This is the most important loop, it will recursively handle the
  // nested expressions based on the precedence, this is a nice design.
  while (!peekTokenIs(TokenTypes::SEMICOLON) && precedence < peekPrecedence()) {
    if (!infixParseFns.count(peekToken.Type)) {
      return std::move(leftExpression);
    }

    auto &&infix = infixParseFns[peekToken.Type];

    nextToken();

    leftExpression = std::move(infix(std::move(leftExpression)));
  }

  return std::move(leftExpression);
}

std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
  auto expressionStatement = std::make_unique<ExpressionStatement>(currentToken);

  expressionStatement->expression = std::move(parseExpression(Precedence::LOWEST));

  if (peekTokenIs(TokenTypes::SEMICOLON)) {
    nextToken();
  }

  return std::move(expressionStatement);
}

std::unique_ptr<Expression> Parser::parseIdentifier() {
  auto identifier = std::make_unique<Identifier>(currentToken, currentToken.Literal);
  return std::move(identifier);
}

std::unique_ptr<Expression> Parser::parseIntegerLiteral() {
  int64_t value = std::strtoll(currentToken.Literal.c_str(), nullptr, 10);
  auto integerLiteral = std::make_unique<IntegerLiteral>(currentToken, value);
  return std::move(integerLiteral);
}

std::unique_ptr<Expression> Parser::ParseGroupedExpression() {
  nextToken();

  auto expression = parseExpression(Precedence::LOWEST);

  // The last token must should be ')'.
  if (!expectPeek(TokenTypes::RPAREN)) {
    return nullptr;
  }

  return std::move(expression);
}

std::unique_ptr<Expression> Parser::parseIfExpression() {
  auto ifExpression = std::make_unique<IfExpression>(currentToken);

  if (!expectPeek(TokenTypes::LPAREN)) {
    return nullptr;
  }

  nextToken();
  ifExpression->condition = std::move(parseExpression(Precedence::LOWEST));

  if (!expectPeek(TokenTypes::RPAREN)) {
    return nullptr;
  }

  if (!expectPeek(TokenTypes::LBRACE)) {
    return nullptr;
  }

  ifExpression->consequence = std::move(parseBlockStatement());

  if (peekTokenIs(TokenTypes::ELSE)) {
    nextToken();

    if (!expectPeek(TokenTypes::LBRACE)) {
      return nullptr;
    }

    ifExpression->alternative = std::move(parseBlockStatement());
  }

  return std::move(ifExpression);
}

std::unique_ptr<BlockStatement> Parser::parseBlockStatement() {
  auto blockStatement = std::make_unique<BlockStatement>(currentToken);

  nextToken();

  while (!currentTokenIs(TokenTypes::RBRACE) && !currentTokenIs(TokenTypes::_EOF)) {
    auto statement = parseStatement();
    if (statement != nullptr) {
      blockStatement->statements.push_back(std::move(statement));
    }
    nextToken();
  }

  return blockStatement;
}

std::unique_ptr<BooleanExpression> Parser::parseBooleanExpression() {
  auto booleanExpression = std::make_unique<BooleanExpression>(currentToken, currentTokenIs(TokenTypes::TRUE));
  return std::move(booleanExpression);
}

bool Parser::currentTokenIs(std::string_view &t) { return currentToken.Type == t; }

bool Parser::peekTokenIs(std::string_view &t) { return peekToken.Type == t; }

bool Parser::expectPeek(std::string_view &t) {
  if (peekTokenIs(t)) {
    nextToken();
    return true;
  }
  peekError(t);
  return false;
}

void Parser::peekError(std::string_view &t) {
  std::string message = "expected next token to be " + std::string(t) + " got " + peekToken.Type + " instead";
  errors.push_back(std::move(message));
}

Precedence Parser::currentPrecedence() {
  if (precedences.count(currentToken.Type)) {
    return precedences[currentToken.Type];
  }

  return Precedence::LOWEST;
}

Precedence Parser::peekPrecedence() {
  if (precedences.count(peekToken.Type)) {
    return precedences[peekToken.Type];
  }

  return Precedence::LOWEST;
}

void Parser::noPrefixParseFnError(TokenType_t &tokenType) {
  std::string message = "no prefix parse function for " + tokenType + " found";
  errors.push_back(std::move(message));
}

void Parser::registerPrefix(const TokenType_t &tokenType, prefixParseFn fn) { prefixParseFns[tokenType] = fn; }
void Parser::registerInfix(const TokenType_t &tokenType, infixParseFn fn) { infixParseFns[tokenType] = fn; }
