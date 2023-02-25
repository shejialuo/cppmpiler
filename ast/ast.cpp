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

BlockStatement::BlockStatement(const Token &t) : token{t} {}
void BlockStatement::statementNode() {}
std::string BlockStatement::tokenLiteral() { return token.Literal; }
std::string BlockStatement::getString() {
  std::string info{};

  for (auto &&statement : statements) {
    info += statement->getString();
  }

  return info;
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

InfixExpression::InfixExpression(Token &t, std::string &op) : token{t}, _operator{op} {}
void InfixExpression::expressionNode() {}
std::string InfixExpression::tokenLiteral() { return token.Literal; }
std::string InfixExpression::getString() {
  std::string info = "(" + left->getString() + " " + _operator + " " + right->getString() + ")";
  return info;
}

BooleanExpression::BooleanExpression(const Token &t, bool v) : token{t}, value{v} {}

void BooleanExpression::expressionNode() {}
std::string BooleanExpression::tokenLiteral() { return token.Literal; }
std::string BooleanExpression::getString() { return token.Literal; }

IfExpression::IfExpression(const Token &t) : token{t} {}
void IfExpression::expressionNode() {}
std::string IfExpression::tokenLiteral() { return token.Literal; }
std::string IfExpression::getString() {
  std::string info = "if" + condition->getString() + " " + consequence->getString();
  if (alternative != nullptr) {
    info += "else " + alternative->getString();
  }
  return info;
}

FunctionLiteral::FunctionLiteral(const Token &t) : token{t} {}
void FunctionLiteral::expressionNode() {}
std::string FunctionLiteral::tokenLiteral() { return token.Literal; }
std::string FunctionLiteral::getString() {
  std::string info = tokenLiteral() + "(";

  int i = 0;
  for (; i < parameters.size() - 1; ++i) {
    info += parameters[i]->getString() + ", ";
  }

  info += parameters[i]->getString() + ")";

  return info;
}

CallExpression::CallExpression(const Token &t) : token{t} {}
void CallExpression::expressionNode() {}
std::string CallExpression::tokenLiteral() { return token.Literal; }
std::string CallExpression::getString() {
  std::string info{};

  info += function->getString();

  info += "(";

  int i = 0;
  for (; i < arguments.size() - 1; ++i) {
    info += arguments[i]->getString() + ", ";
  }

  info += arguments[i]->getString() + ")";

  return info;
}

StringLiteral::StringLiteral(const Token &t, const std::string &s) : token{t}, value{s} {}
void StringLiteral::expressionNode() {}
std::string StringLiteral::tokenLiteral() { return token.Literal; }
std::string StringLiteral::getString() { return token.Literal; }

ArrayLiteral::ArrayLiteral(const Token &t) : token{t} {}
void ArrayLiteral::expressionNode() {}
std::string ArrayLiteral::tokenLiteral() { return token.Literal; }
std::string ArrayLiteral::getString() {
  std::string info{};

  info += "[";

  int i = 0;
  for (; i < elements.size() - 1; ++i) {
    info += elements[i]->getString() + ", ";
  }

  info += elements[i]->getString() + "]";

  return info;
}

IndexExpression::IndexExpression(const Token &t) : token{t} {}
void IndexExpression::expressionNode() {}
std::string IndexExpression::tokenLiteral() { return token.Literal; }
std::string IndexExpression::getString() {
  std::string info = "(" + left->getString() + "[" + index->getString() + "])";

  return info;
}