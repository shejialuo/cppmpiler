#include "ast.hpp"

#include "token.hpp"

#include <string>

void Statement::statementNode() {}
std::string Statement::tokenLiteral() { return ""; }
std::string Statement::getString() { return ""; }

void Expression::expressionNode() {}
std::string Expression::tokenLiteral() { return ""; }
std::string Expression::getString() { return ""; }

std::string Program::tokenLiteral() { return statements.size() > 0 ? statements[0]->tokenLiteral() : ""; }
std::string Program::getString() {
  std::string info{};
  for (auto &&statement : statements) {
    info += statement->getString();
  }
  return info;
}

Identifier::Identifier(const Token &t, std::string &s) : token{t}, value{s} {}
void Identifier::expressionNode() {}
std::string Identifier::tokenLiteral() { return token.Literal; }
std::string Identifier::getString() { return value; }

LetStatement::LetStatement(const Token &t) : token{t} {}
void LetStatement::statementNode() {}
std::string LetStatement::tokenLiteral() { return token.Literal; }
std::string LetStatement::getString() {
  std::string info{tokenLiteral() + " " + name->getString() + " = "};

  if (value != nullptr) {
    info += value->getString();
  }

  info += ";";

  return info;
}

ReturnStatement::ReturnStatement(const Token &t) : token{t} {}
void ReturnStatement::statementNode() {}
std::string ReturnStatement::tokenLiteral() { return token.Literal; }
std::string ReturnStatement::getString() {
  std::string info{tokenLiteral() + " "};

  if (returnValue != nullptr) {
    info += returnValue->getString();
  }

  info += ";";

  return info;
}

ExpressionStatement::ExpressionStatement(const Token &t) : token{t} {}
void ExpressionStatement::statementNode() {}
std::string ExpressionStatement::tokenLiteral() { return token.Literal; }
std::string ExpressionStatement::getString() {
  if (expression != nullptr) {
    return expression->getString();
  }
  return "";
}

IntegerLiteral::IntegerLiteral(const Token &t, int64_t v) : token{t}, value{v} {}
void IntegerLiteral::expressionNode() {}
std::string IntegerLiteral::tokenLiteral() { return token.Literal; }
std::string IntegerLiteral::getString() { return token.Literal; }

PrefixExpression::PrefixExpression(Token &t, std::string &op) : token{t}, _operator{op} {}
void PrefixExpression::expressionNode() {}
std::string PrefixExpression::tokenLiteral() { return token.Literal; }
std::string PrefixExpression::getString() {
  std::string info = "(" + _operator + right->getString() + ")";
  return info;
}
