#include "parser.hpp"

#include "ast.hpp"
#include "lexer.hpp"
#include "token.hpp"

#include <functional>
#include <memory>
#include <string_view>

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

  TokenType_t type = std::string(TokenTypes::IDENT);
  registerPrefix(type, std::bind(&Parser::parseIdentifier, this));
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

std::unique_ptr<Expression> Parser::parseExpression(Precedence precedence) {
  if (!prefixParseFns.count(currentToken.Type)) {
    return nullptr;
  }

  auto &&prefix = prefixParseFns[currentToken.Type];

  auto leftExpression = prefix();

  Identifier *test = dynamic_cast<Identifier *>(leftExpression.get());

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

void Parser::registerPrefix(TokenType_t &tokenType, prefixParseFn fn) { prefixParseFns[tokenType] = fn; }
void Parser::registerInfix(TokenType_t &tokenType, infixParseFn fn) { infixParseFns[tokenType] = fn; }
