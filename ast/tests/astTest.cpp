#include "ast.hpp"
#include "token.hpp"

#include <gtest/gtest.h>
#include <memory>

TEST(Ast, TestGetString) {
  Token letToken;
  letToken.Type = std::string{TokenTypes::LET};
  letToken.Literal = "let";

  Token myVar;
  myVar.Type = std::string{TokenTypes::IDENT};
  myVar.Literal = "myVar";

  auto myVarIdentifier = std::make_unique<Identifier>(myVar, myVar.Literal);

  Token anotherVar;
  anotherVar.Type = std::string{TokenTypes::IDENT};
  anotherVar.Literal = "anotherVar";

  auto anotherVarIdentifier = std::make_unique<Identifier>(anotherVar, anotherVar.Literal);

  auto letStatement = std::make_unique<LetStatement>(letToken);

  letStatement->name = std::move(myVarIdentifier);
  letStatement->value = std::move(anotherVarIdentifier);

  auto program = std::make_unique<Program>();
  program->statements.push_back(std::move(letStatement));

  ASSERT_EQ(program->getString(), "let myVar = anotherVar;");
}