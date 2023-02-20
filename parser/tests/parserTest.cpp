#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "spdlog/spdlog.h"

#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

bool testLetStatement(Statement *statement, std::string name);
bool checkParseErrors(Parser &parser);
bool testIntegerLiteral(Expression *expression, int64_t value);

struct TestData {
  std::string expectedIdentifier;

  TestData(std::string i) : expectedIdentifier{i} {}
};

struct TestPrefixData {
  std::string input;
  std::string _operator;
  int64_t value;

  TestPrefixData(std::string i, std::string op, int64_t v) : input{i}, _operator{op}, value{v} {}
};

struct TestInfixData {
  std::string input;
  int64_t leftValue;
  std::string _operator;
  int64_t rightValue;

  TestInfixData(std::string i, int64_t l, std::string op, int64_t r)
      : input{i}, leftValue{l}, _operator{op}, rightValue{r} {}
};

struct TestPrecedenceData {
  std::string input;
  std::string expected;

  TestPrecedenceData(const std::string &i, const std::string &e) : input{i}, expected{e} {}
};

TEST(Parser, TestLetStatements) {
  std::string input = "let x = 5; \
                       let y = 10; \
                       let foobar = 838383;";
  Lexer lexer{input};
  Parser parser{&lexer};

  std::unique_ptr<Program> program = parser.parseProgram();
  if (!checkParseErrors(parser)) {
    FAIL();
  }

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

TEST(Parser, TestReturnStatements) {
  std::string input = "return 5;\
                       return 10; \
                       return 993322;";

  Lexer lexer{input};
  Parser parser{&lexer};

  auto program = parser.parseProgram();
  if (!checkParseErrors(parser)) {
    FAIL();
  }

  if (program->statements.size() != 3) {
    spdlog::error("program->statements does not contain 3 statements. got='{}'", program->statements.size());
    FAIL();
  }

  for (auto &&statement : program->statements) {
    ReturnStatement *returnStatement = dynamic_cast<ReturnStatement *>(statement.get());
    if (returnStatement == nullptr) {
      spdlog::error("statement is not an returnStatement");
      FAIL();
    }

    if (returnStatement->tokenLiteral() != "return") {
      spdlog::error("letStatement.name not 'return', got='{}'", returnStatement->tokenLiteral());
      FAIL();
    }
  }
}

TEST(Parser, TestIdentifierExpression) {
  std::string input{"foobar;"};

  Lexer lexer{input};
  Parser parser{&lexer};
  auto program = parser.parseProgram();

  if (!checkParseErrors(parser)) {
    FAIL();
  }

  if (program->statements.size() != 1) {
    spdlog::error("program has not enough statements. got='{}'", program->statements.size());
    FAIL();
  }

  ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(program->statements[0].get());

  if (expressionStatement == nullptr) {
    spdlog::error("statement is not an ExpressionStatement");
    FAIL();
  }

  Identifier *identifier = dynamic_cast<Identifier *>(expressionStatement->expression.get());
  if (identifier == nullptr) {
    spdlog::error("expression is not an Identifier");
    FAIL();
  }

  if (identifier->value != "foobar") {
    spdlog::error("identifier->value not 'foobar'. got='{}'", identifier->value);
    FAIL();
  }

  if (identifier->tokenLiteral() != "foobar") {
    spdlog::error("identifier->tokenLiteral not 'foobar'. got='{}'", identifier->tokenLiteral());
    FAIL();
  }
}

TEST(Parser, TestIntegerLiteralExpression) {
  std::string input{"5;"};

  Lexer lexer{input};
  Parser parser{&lexer};

  auto program = parser.parseProgram();
  if (!checkParseErrors(parser)) {
    FAIL();
  }

  if (program->statements.size() != 1) {
    spdlog::error("program has not enough statements. got='{}'", program->statements.size());
    FAIL();
  }

  ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(program->statements[0].get());

  if (expressionStatement == nullptr) {
    spdlog::error("statement is not an ExpressionStatement");
    FAIL();
  }

  IntegerLiteral *integer = dynamic_cast<IntegerLiteral *>(expressionStatement->expression.get());
  if (integer == nullptr) {
    spdlog::error("expression is not an IntegerLiteral");
    FAIL();
  }

  if (integer->value != 5) {
    spdlog::error("identifier->value not 5. got='{}'", integer->value);
    FAIL();
  }

  if (integer->tokenLiteral() != "5") {
    spdlog::error("identifier->tokenLiteral not '5'. got='{}'", integer->tokenLiteral());
    FAIL();
  }
}

TEST(Parser, TestParsingPrefixExpressions) {
  TestPrefixData prefixTests[] = {
      {"!5;", "!", 5},
      {"-15;", "-", 15},
  };

  for (auto &&prefixTest : prefixTests) {
    Lexer lexer{prefixTest.input};
    Parser parser{&lexer};

    auto program = parser.parseProgram();

    if (!checkParseErrors(parser)) {
      FAIL();
    }

    if (program->statements.size() != 1) {
      spdlog::error("program->statements does not contain 1 statements. got='{}'", program->statements.size());
      FAIL();
    }

    ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(program->statements[0].get());
    if (expressionStatement == nullptr) {
      spdlog::error("statement is not an ExpressionStatement");
      FAIL();
    }

    PrefixExpression *prefixExpression = dynamic_cast<PrefixExpression *>(expressionStatement->expression.get());
    if (prefixExpression == nullptr) {
      spdlog::error("expression is not a PrefixExpression");
      FAIL();
    }

    if (prefixExpression->_operator != prefixTest._operator) {
      spdlog::error(
          "prefixExpression->_operator is not '{}'. got='{}'", prefixTest._operator, prefixExpression->_operator);
      FAIL();
    }

    if (!testIntegerLiteral(prefixExpression->right.get(), prefixTest.value)) {
      FAIL();
    }
  }
}

TEST(Parser, TestParsingInfixExpressions) {
  TestInfixData infixTests[] = {
      {"5 + 5;", 5, "+", 5},
      {"5 - 5;", 5, "-", 5},
      {"5 * 5;", 5, "*", 5},
      {"5 / 5;", 5, "/", 5},
      {"5 > 5;", 5, ">", 5},
      {"5 < 5;", 5, "<", 5},
      {"5 == 5;", 5, "==", 5},
      {"5 != 5;", 5, "!=", 5},
  };

  for (auto &&infixTest : infixTests) {
    Lexer lexer{infixTest.input};
    Parser parser{&lexer};
    auto program = parser.parseProgram();

    if (!checkParseErrors(parser)) {
      FAIL();
    }

    if (program->statements.size() != 1) {
      spdlog::error("program->statements does not contain 1 statements. got='{}'", program->statements.size());
      FAIL();
    }

    ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(program->statements[0].get());
    if (expressionStatement == nullptr) {
      spdlog::error("statement is not an ExpressionStatement");
      FAIL();
    }

    InfixExpression *infixExpression = dynamic_cast<InfixExpression *>(expressionStatement->expression.get());
    if (infixExpression == nullptr) {
      spdlog::error("expression is not a PrefixExpression");
      FAIL();
    }

    if (!testIntegerLiteral(infixExpression->left.get(), infixTest.leftValue)) {
      FAIL();
    }

    if (infixExpression->_operator != infixTest._operator) {
      spdlog::error(
          "infixExpression->_operator is not '{}'. got='{}'", infixTest._operator, infixExpression->_operator);
      FAIL();
    }

    if (!testIntegerLiteral(infixExpression->right.get(), infixTest.rightValue)) {
      FAIL();
    }
  }
}

TEST(Parser, TestOperatorPrecedenceParsing) {
  TestPrecedenceData tests[] = {
      {
          "-a * b;",
          "((-a) * b)",
      },
      {
          "!-a;",
          "(!(-a))",
      },
      {
          "a + b + c;",
          "((a + b) + c)",
      },
      {
          "a + b - c;",
          "((a + b) - c)",
      },
      {
          "a * b * c;",
          "((a * b) * c)",
      },
      {
          "a * b / c;",
          "((a * b) / c)",
      },
      {
          "a + b / c;",
          "(a + (b / c))",
      },
      {
          "a + b * c + d / e - f;",
          "(((a + (b * c)) + (d / e)) - f)",
      },
      {
          "3 + 4; -5 * 5;",
          "(3 + 4)((-5) * 5)",
      },
      {
          "5 > 4 == 3 < 4;",
          "((5 > 4) == (3 < 4))",
      },
      {
          "5 < 4 != 3 > 4;",
          "((5 < 4) != (3 > 4))",
      },
      {
          "3 + 4 * 5 == 3 * 1 + 4 * 5;",
          "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
      },
      {
          "true;",
          "true",
      },
      {
          "false;",
          "false",
      },
      {
          "3 > 5 == false;",
          "((3 > 5) == false)",
      },
      {
          "3 < 5 == true;",
          "((3 < 5) == true)",
      },

  };

  for (auto &&test : tests) {
    Lexer lexer{test.input};
    Parser parser{&lexer};

    auto program = parser.parseProgram();

    if (!checkParseErrors(parser)) {
      FAIL();
    }

    std::string actual = program->getString();
    if (actual != test.expected) {
      spdlog::error("expected='{}', got='{}'", test.expected, actual);
      FAIL();
    }
  }
}

bool testIntegerLiteral(Expression *expression, int64_t value) {
  IntegerLiteral *integerLiteral = dynamic_cast<IntegerLiteral *>(expression);
  if (integerLiteral == nullptr) {
    spdlog::error("it is not an IntegerLiteral");
    return false;
  }
  if (integerLiteral->value != value) {
    spdlog::error("integerLiteral->value is not '{}'. got='{}'", value, integerLiteral->value);
    return false;
  }

  if (integerLiteral->tokenLiteral() != std::to_string(value)) {
    spdlog::error("integerLiteral->tokenLiteral is not '{}'. got='{}'", value, integerLiteral->tokenLiteral());
    return false;
  }

  return true;
}

bool checkParseErrors(Parser &parser) {
  if (parser.getErrors().size() == 0) {
    return true;
  } else {
    for (auto &&message : parser.getErrors()) {
      spdlog::error("parser error: {}", message);
    }
    return false;
  }
}
