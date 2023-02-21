#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "spdlog/spdlog.h"

#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

template <typename T>
struct TestLetStatementData {
  std::string input;
  std::string expectedIdentifier;
  T expectedVar;

  TestLetStatementData(const std::string &i, const std::string &ei, T ev)
      : input{i}, expectedIdentifier{ei}, expectedVar{ev} {}
};

template <typename T>
struct TestPrefixData {
  std::string input;
  std::string _operator;
  T value;

  TestPrefixData(std::string i, std::string op, T v) : input{i}, _operator{op}, value{v} {}
};

template <typename T1, typename T2>
struct TestInfixData {
  std::string input;
  T1 leftValue;
  std::string _operator;
  T2 rightValue;

  TestInfixData(std::string i, T1 l, std::string op, T2 r) : input{i}, leftValue{l}, _operator{op}, rightValue{r} {}
};

template <typename T>
struct TestReturnData {
  std::string input;
  T expectedValue;

  TestReturnData(std::string i, T v) : input{i}, expectedValue{v} {}
};

struct TestPrecedenceData {
  std::string input;
  std::string expected;

  TestPrecedenceData(const std::string &i, const std::string &e) : input{i}, expected{e} {}
};

bool testLetStatement(Statement *statement, std::string name);
bool checkParseErrors(Parser &parser);
bool testIntegerLiteral(Expression *expression, int64_t value);
bool testBooleanLiteral(Expression *expression, bool value);
bool testIdentifier(Expression *expression, std::string value);

template <typename T>
bool testLetStatementHelper(TestLetStatementData<T> &test);

template <typename T>
bool testLiteralExpression(Expression *expression, T expected);

template <typename T1, typename T2>
bool testInfixExpression(Expression *expression, T1 left, const std::string &op, T2 right);

template <typename T1, typename T2>
bool testInfixExpressionHelper(TestInfixData<T1, T2> &test);

template <typename T>
bool testPrefixExpressionHelper(TestPrefixData<T> &test);

template <typename T>
bool testReturnStatementHelper(TestReturnData<T> &test);

template <typename T>
bool testPrefixExpressionHelper(TestPrefixData<T> &test) {
  Lexer lexer{test.input};
  Parser parser{&lexer};

  auto program = parser.parseProgram();

  if (!checkParseErrors(parser)) {
    return false;
  }

  if (program->statements.size() != 1) {
    spdlog::error("program->statements does not contain 1 statements. got='{}'", program->statements.size());
    return false;
  }

  ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(program->statements[0].get());
  if (expressionStatement == nullptr) {
    spdlog::error("statement is not an ExpressionStatement");
    return false;
  }

  PrefixExpression *prefixExpression = dynamic_cast<PrefixExpression *>(expressionStatement->expression.get());
  if (prefixExpression == nullptr) {
    spdlog::error("expression is not a PrefixExpression");
    return false;
  }

  if (prefixExpression->_operator != test._operator) {
    spdlog::error("prefixExpression->_operator is not '{}'. got='{}'", test._operator, prefixExpression->_operator);
    return false;
  }

  if (!testLiteralExpression<decltype(test.value)>(prefixExpression->right.get(), test.value)) {
    return false;
  }
  return true;
}

template <typename T1, typename T2>
bool testInfixExpressionHelper(TestInfixData<T1, T2> &test) {
  Lexer lexer{test.input};
  Parser parser{&lexer};
  auto program = parser.parseProgram();

  if (!checkParseErrors(parser)) {
    return false;
  }

  if (program->statements.size() != 1) {
    spdlog::error("program->statements does not contain 1 statements. got='{}'", program->statements.size());
    return false;
  }

  ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(program->statements[0].get());
  if (expressionStatement == nullptr) {
    spdlog::error("statement is not an ExpressionStatement");
    return false;
  }

  InfixExpression *infixExpression = dynamic_cast<InfixExpression *>(expressionStatement->expression.get());
  if (infixExpression == nullptr) {
    spdlog::error("expression is not a PrefixExpression");
    return false;
  }

  if (infixExpression->_operator != test._operator) {
    spdlog::error("infixExpression->_operator is not '{}'. got='{}'", test._operator, infixExpression->_operator);
    return false;
  }

  if (!testLiteralExpression<decltype(test.leftValue)>(infixExpression->left.get(), test.leftValue)) {
    return false;
  }

  if (!testLiteralExpression<decltype(test.rightValue)>(infixExpression->right.get(), test.rightValue)) {
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

bool testBooleanLiteral(Expression *expression, bool value) {
  BooleanExpression *booleanExpression = dynamic_cast<BooleanExpression *>(expression);
  if (booleanExpression == nullptr) {
    spdlog::error("expression is not a BooleanExpression");
    return false;
  }

  if (booleanExpression->value != value) {
    spdlog::error("booleanExpression->value not {}. got={}", value, booleanExpression->value);
    return false;
  }

  std::string val = value ? "true" : "false";

  if (booleanExpression->tokenLiteral() != val) {
    spdlog::error("booleanExpression->tokenLiteral not {}. got={}", value, booleanExpression->tokenLiteral());
    return false;
  }

  return true;
}

bool testIdentifier(Expression *expression, std::string value) {
  Identifier *identifier = dynamic_cast<Identifier *>(expression);

  if (identifier == nullptr) {
    spdlog::error("expression is not an Identifier");
    return false;
  }

  if (identifier->value != value) {
    spdlog::error("identifier->value not '{}', got='{}'", value, identifier->value);
    return false;
  }

  if (identifier->tokenLiteral() != value) {
    spdlog::error("identifier->tokenLiteral not '{}', got='{}'", value, identifier->tokenLiteral());
    return false;
  }

  return true;
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

template <typename T>
bool testLiteralExpression(Expression *expression, T expected) {
  if constexpr (std::is_same_v<int, T>) {
    return testIntegerLiteral(expression, static_cast<int64_t>(expected));
  } else if constexpr (std::is_same_v<int64_t, T>) {
    return testIntegerLiteral(expression, expected);
  } else if constexpr (std::is_same_v<T, std::string>) {
    return testIdentifier(expression, expected);
  } else if constexpr (std::is_same_v<T, bool>) {
    return testBooleanLiteral(expression, expected);
  }
  return false;
}

template <typename T>
bool testLetStatementHelper(TestLetStatementData<T> &test) {
  Lexer lexer{test.input};
  Parser parser{&lexer};

  auto program = parser.parseProgram();

  if (!checkParseErrors(parser)) {
    return false;
  }

  if (program->statements.size() != 1) {
    spdlog::error("program->statements does not contain 1 statement. got={}", program->statements.size());
    return false;
  }

  auto statement = program->statements[0].get();

  if (!testLetStatement(statement, test.expectedIdentifier)) {
    return false;
  }

  LetStatement *letStatement = dynamic_cast<LetStatement *>(statement);

  if (!testLiteralExpression<T>(letStatement->value.get(), test.expectedVar)) {
    return false;
  }

  return true;
}

template <typename T1, typename T2>
bool testInfixExpression(Expression *expression, T1 left, const std::string &op, T2 right) {
  InfixExpression *infixExpression = dynamic_cast<InfixExpression *>(expression);
  if (infixExpression == nullptr) {
    spdlog::error("expression is not an InfixExpression");
    return false;
  }

  if (!testLiteralExpression(infixExpression->left.get(), left)) {
    return false;
  }

  if (infixExpression->_operator != op) {
    spdlog::error("operator is not '{}'. got='{}'", op, infixExpression->_operator);
    return false;
  }

  if (!testLiteralExpression(infixExpression->right.get(), right)) {
    return false;
  }

  return true;
}

template <typename T>
bool testReturnStatementHelper(TestReturnData<T> &test) {
  Lexer lexer{test.input};
  Parser parser{&lexer};

  auto program = parser.parseProgram();
  if (!checkParseErrors(parser)) {
    return false;
  }

  if (program->statements.size() != 1) {
    spdlog::error("program->statements does not contain 1 statements. got='{}'", program->statements.size());
    return false;
  }

  ReturnStatement *returnStatement = dynamic_cast<ReturnStatement *>(program->statements[0].get());

  if (returnStatement == nullptr) {
    spdlog::error("statement is not an returnStatement");
    return false;
  }

  if (returnStatement->tokenLiteral() != "return") {
    spdlog::error("returnStatement.tokenLiteral not 'return', got='{}'", returnStatement->tokenLiteral());
    return false;
  }

  if (!testLiteralExpression(returnStatement->returnValue.get(), test.expectedValue)) {
    return false;
  }

  return true;
}

TEST(Parser, TestLetStatements) {
  TestLetStatementData<int64_t> test1{"let x = 5;", "x", 5};
  TestLetStatementData<bool> test2{"let y = true;", "y", true};
  TestLetStatementData<std::string> test3{"let foobar = y;", "foobar", "y"};

  if (!testLetStatementHelper(test1)) {
    FAIL();
  }

  if (!testLetStatementHelper(test2)) {
    FAIL();
  }

  if (!testLetStatementHelper(test3)) {
    FAIL();
  }
}

TEST(Parser, TestReturnStatements) {
  TestReturnData<int> test1{"return 5;", 5};
  TestReturnData<bool> test2{"return true;", true};
  TestReturnData<std::string> test3{"return foobar;", "foobar"};

  if (!testReturnStatementHelper(test1)) {
    FAIL();
  }
  if (!testReturnStatementHelper(test2)) {
    FAIL();
  }
  if (!testReturnStatementHelper(test3)) {
    FAIL();
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
  std::vector<TestPrefixData<int>> prefixTestsForInt{
      {"!5;", "!", 5},
      {"-15;", "-", 15},
  };

  std::vector<TestPrefixData<std::string>> prefixTestsForString{
      {"!foobar;", "!", "foobar"},
      {"-foobar;", "-", "foobar"},
  };

  std::vector<TestPrefixData<bool>> prefixTestsForBoolean{
      {"!true;", "!", true},
      {"!false;", "!", false},
  };

  for (auto &&prefixTest : prefixTestsForInt) {
    if (!testPrefixExpressionHelper(prefixTest)) {
      FAIL();
    }
  }

  for (auto &&prefixTest : prefixTestsForString) {
    if (!testPrefixExpressionHelper(prefixTest)) {
      FAIL();
    }
  }

  for (auto &&prefixTest : prefixTestsForBoolean) {
    if (!testPrefixExpressionHelper(prefixTest)) {
      FAIL();
    }
  }
}

TEST(Parser, TestParsingInfixExpressions) {
  std::vector<TestInfixData<int64_t, int64_t>> infixTestsForInt{
      {"5 + 5;", 5, "+", 5},
      {"5 - 5;", 5, "-", 5},
      {"5 * 5;", 5, "*", 5},
      {"5 / 5;", 5, "/", 5},
      {"5 > 5;", 5, ">", 5},
      {"5 < 5;", 5, "<", 5},
      {"5 == 5;", 5, "==", 5},
      {"5 != 5;", 5, "!=", 5},
  };

  std::vector<TestInfixData<std::string, std::string>> infixTestsForString{
      {"foobar + barfoo;", "foobar", "+", "barfoo"},
      {"foobar - barfoo;", "foobar", "-", "barfoo"},
      {"foobar * barfoo;", "foobar", "*", "barfoo"},
      {"foobar / barfoo;", "foobar", "/", "barfoo"},
      {"foobar > barfoo;", "foobar", ">", "barfoo"},
      {"foobar < barfoo;", "foobar", "<", "barfoo"},
      {"foobar == barfoo;", "foobar", "==", "barfoo"},
      {"foobar != barfoo;", "foobar", "!=", "barfoo"},

  };

  std::vector<TestInfixData<bool, bool>> infixTestsForBoolean{
      {"true == true;", true, "==", true},
      {"true != false;", true, "!=", false},
      {"false == false;", false, "==", false},
  };

  for (auto &&infixTest : infixTestsForInt) {
    if (!testInfixExpressionHelper(infixTest)) {
      FAIL();
    }
  }

  for (auto &&infixTest : infixTestsForString) {
    if (!testInfixExpressionHelper(infixTest)) {
      FAIL();
    }
  }

  for (auto &&infixTest : infixTestsForBoolean) {
    if (!testInfixExpressionHelper(infixTest)) {
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
      {
          "1 + (2 + 3) + 4;",
          "((1 + (2 + 3)) + 4)",
      },
      {
          "(5 + 5) * 2;",
          "((5 + 5) * 2)",
      },
      {
          "2 / (5 + 5);",
          "(2 / (5 + 5))",
      },
      {
          "-(5 + 5);",
          "(-(5 + 5))",
      },
      {
          "!(true == true);",
          "(!(true == true))",
      },
      {
          "a + add(b * c) + d;",
          "((a + add((b * c))) + d)",
      },
      {
          "add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8));",
          "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))",
      },
      {
          "add(a + b + c * d / f + g);",
          "add((((a + b) + ((c * d) / f)) + g))",
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

TEST(Parser, TestIfExpression) {
  std::string input = "if (x < y) { x }";

  Lexer lexer{input};
  Parser parser{&lexer};
  auto program = parser.parseProgram();
  if (!checkParseErrors(parser)) {
    FAIL();
  }

  if (program->statements.size() != 1) {
    spdlog::error("program body does not contain 1. got=", program->statements.size());
    FAIL();
  }

  ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(program->statements[0].get());
  if (expressionStatement == nullptr) {
    spdlog::error("statement is not an ExpressionStatement");
    FAIL();
  }

  IfExpression *ifExpression = dynamic_cast<IfExpression *>(expressionStatement->expression.get());
  if (ifExpression == nullptr) {
    spdlog::error("expression is not an IfExpression");
    FAIL();
  }

  if (!testInfixExpression<std::string, std::string>(ifExpression->condition.get(), "x", "<", "y")) {
    FAIL();
  }

  if (ifExpression->consequence->statements.size() != 1) {
    spdlog::error("consequence is not 1 statements. got={}\n", ifExpression->consequence->statements.size());
    FAIL();
  }

  ExpressionStatement *es = dynamic_cast<ExpressionStatement *>(ifExpression->consequence->statements[0].get());
  if (es == nullptr) {
    spdlog::error("Consequence statement is not ExpressionStatement");
    FAIL();
  }

  if (!testIdentifier(es->expression.get(), "x")) {
    FAIL();
  }

  if (ifExpression->alternative != nullptr) {
    spdlog::error("alternative is not nullptr");
    FAIL();
  }
}

TEST(Parser, TestIfElseExpression) {
  std::string input = "if (x < y) { x } else { y }";

  Lexer lexer{input};
  Parser parser{&lexer};
  auto program = parser.parseProgram();
  if (!checkParseErrors(parser)) {
    FAIL();
  }

  if (program->statements.size() != 1) {
    spdlog::error("program body does not contain 1. got=", program->statements.size());
    FAIL();
  }

  ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(program->statements[0].get());
  if (expressionStatement == nullptr) {
    spdlog::error("statement is not an ExpressionStatement");
    FAIL();
  }

  IfExpression *ifExpression = dynamic_cast<IfExpression *>(expressionStatement->expression.get());
  if (ifExpression == nullptr) {
    spdlog::error("expression is not an IfExpression");
    FAIL();
  }

  if (!testInfixExpression<std::string, std::string>(ifExpression->condition.get(), "x", "<", "y")) {
    FAIL();
  }

  if (ifExpression->consequence->statements.size() != 1) {
    spdlog::error("consequence is not 1 statements. got={}\n", ifExpression->consequence->statements.size());
    FAIL();
  }

  ExpressionStatement *es = dynamic_cast<ExpressionStatement *>(ifExpression->consequence->statements[0].get());
  if (es == nullptr) {
    spdlog::error("Consequence statement is not ExpressionStatement");
    FAIL();
  }

  if (!testIdentifier(es->expression.get(), "x")) {
    FAIL();
  }

  if (ifExpression->alternative->statements.size() != 1) {
    spdlog::error("Alternative statement does not contain 1 statements. got={}",
                  ifExpression->alternative->statements.size());
    FAIL();
  }

  es = dynamic_cast<ExpressionStatement *>(ifExpression->alternative->statements[0].get());
  if (es == nullptr) {
    spdlog::error("Alternative statement is not ExpressionStatement");
    FAIL();
  }

  if (!testIdentifier(es->expression.get(), "y")) {
    FAIL();
  }
}

TEST(Parser, TestFunctionLiteralParsing) {
  std::string input = "fn(x, y) { x + y; }";

  Lexer lexer{input};
  Parser parser{&lexer};
  auto program = parser.parseProgram();

  if (!checkParseErrors(parser)) {
    FAIL();
  }

  if (program->statements.size() != 1) {
    spdlog::error("program body does not contain 1 statements. got=%d", program->statements.size());
    FAIL();
  }

  ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(program->statements[0].get());
  if (expressionStatement == nullptr) {
    spdlog::error("program->statements[0] is not ExpressionStatement");
    FAIL();
  }

  FunctionLiteral *function = dynamic_cast<FunctionLiteral *>(expressionStatement->expression.get());
  if (function == nullptr) {
    spdlog::error("expression is not FunctionLiteral");
    FAIL();
  }

  if (function->parameters.size() != 2) {
    spdlog::error("function literal parameters wrong. want 2, got={}", function->parameters.size());
    FAIL();
  }

  if (!testLiteralExpression<std::string>(function->parameters[0].get(), "x")) {
    FAIL();
  }

  if (!testLiteralExpression<std::string>(function->parameters[1].get(), "y")) {
    FAIL();
  }

  if (function->body->statements.size() != 1) {
    spdlog::error("function->body->statements has not 1 statements. got={}", function->body->statements.size());
    FAIL();
  }

  ExpressionStatement *body = dynamic_cast<ExpressionStatement *>(function->body->statements[0].get());
  if (body == nullptr) {
    spdlog::error("function body is not ExpressionStatement");
    FAIL();
  }

  if (!testInfixExpression<std::string, std::string>(body->expression.get(), "x", "+", "y")) {
    FAIL();
  }
}

TEST(Parser, TestCallExpressionParsing) {
  std::string input = "add(1, 2 * 3, 4 + 5);";
  Lexer lexer{input};
  Parser parser{&lexer};
  auto program = parser.parseProgram();

  if (!checkParseErrors(parser)) {
    FAIL();
  }

  if (program->statements.size() != 1) {
    spdlog::error("program->statements does not contain 1 statements. got={}", program->statements.size());
    FAIL();
  }

  ExpressionStatement *expressionStatement = dynamic_cast<ExpressionStatement *>(program->statements[0].get());
  if (expressionStatement == nullptr) {
    spdlog::error("It is not an ExpressionStatement");
    FAIL();
  }

  CallExpression *call = dynamic_cast<CallExpression *>(expressionStatement->expression.get());
  if (call == nullptr) {
    spdlog::error("call is not a CallExpression");
    FAIL();
  }

  if (!testIdentifier(call->function.get(), "add")) {
    FAIL();
  }

  if (call->arguments.size() != 3) {
    spdlog::error("wrong length of arguments. got={}", call->arguments.size());
    FAIL();
  }

  if (!testLiteralExpression<int64_t>(call->arguments[0].get(), 1)) {
    FAIL();
  }

  if (!testInfixExpression<int64_t, int64_t>(call->arguments[1].get(), 2, "*", 3)) {
    FAIL();
  }

  if (!testInfixExpression<int64_t, int64_t>(call->arguments[2].get(), 4, "+", 5)) {
    FAIL();
  }
}
