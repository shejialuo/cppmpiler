#include "ast.hpp"

#include "token.hpp"

#include <string>

void Statement::statementNode() {}
std::string Statement::tokenLiteral() { return ""; }

std::string Program::tokenLiteral() { return statements.size() > 0 ? statements[0]->tokenLiteral() : ""; }

Identifier::Identifier(const Token &t, std::string &s) : token{t}, value{s} {}
void Identifier::expressionNode() {}
std::string Identifier::tokenLiteral() { return token.Literal; }

LetStatement::LetStatement(const Token &t) : token{t} {}
void LetStatement::statementNode() {}
std::string LetStatement::tokenLiteral() { return token.Literal; }
