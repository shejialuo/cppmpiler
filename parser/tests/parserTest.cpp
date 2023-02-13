#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "spdlog/spdlog.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

bool testLetStatement(Statement *statement, std::string name);

struct TestData {
  std::string expectedIdentifier;

  TestData(std::string i) : expectedIdentifier{i} {}
};

TEST(Parser, TestLetStatements) {
  std::string input = "let x = 5; \
                       let y = 10; \
                       let foobar = 838383;";
  Lexer lexer{input};
  Parser parser{&lexer};

  std::unique_ptr<Program> program = parser.parseProgram();
  if (program == nullptr) {
    spdlog::error("parseProgram() returned null");
    FAIL();
  }
  if (program->statements.size() != 3) {
    spdlog::error("program->statements does not contain 3 statements. got='{}'", program->statements.size());
  }

  std::vector<TestData> tests{
      {"x"},
      {"y"},
      {"foobar"},
  };

  for (int i = 0; i < tests.size(); ++i) {
    auto statement = program->statements[i].get();
    if (!testLetStatement(statement, tests[i].expectedIdentifier)) {
      FAIL();
    }
  }
}

bool testLetStatement(Statement *statement, std::string name) {
  if (statement->tokenLiteral() != "let") {
    spdlog::error("statement.tokenLiteral not 'let' got='{}'", statement->tokenLiteral());
    return false;
  }

  LetStatement *letStatement = dynamic_cast<LetStatement *>(statement);
  if (letStatement == nullptr) {
    spdlog::error("statement is not an letStatement");
    return false;
  }

  if (letStatement->name->value != name) {
    spdlog::error("letStatement->name->value not '{}'. got='{}'", name, letStatement->name->value);
    return false;
  }

  if (letStatement->name->tokenLiteral() != name) {
    spdlog::error("letStatement.name not '{}', got='{}'", name, letStatement->name->tokenLiteral());
    return false;
  }
  return true;
}
