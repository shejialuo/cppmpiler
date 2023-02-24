#ifndef _AST_AST_HPP_
#define _AST_AST_HPP_

#include "token.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Every node in the AST has to implement
 * this abstract class.
 *
 */
class Node {
public:
  /**
   * @brief the literal value of the token
   *
   * @return std::string
   */
  virtual std::string tokenLiteral() = 0;
  /**
   * @brief Get the string information
   *
   * @return std::string
   */
  virtual std::string getString() = 0;
  virtual ~Node() = default;
};

/**
 * @brief Represent the statements, for example,
 * `let a = 3;` is a statement. It should inherit
 * from Node.
 */
class Statement : public Node {
public:
  std::string tokenLiteral() override;
  std::string getString() override;
  virtual void statementNode();
};

/**
 * @brief Represent the expression, for example,
 * `let a = 3`. `a` is the expression and also,
 * `3` is the expression. It should inherit from
 * Node.
 */
class Expression : public Node {
public:
  std::string tokenLiteral() override;
  std::string getString() override;
  virtual void expressionNode();
};

/**
 * @brief This class is the entry point of the AST.
 * it composes a consecutive statements.
 *
 */
class Program : public Node {
public:
  std::vector<std::unique_ptr<Statement>> statements{};
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief Identifier class. It is an expression.
 * It is inherited from `Expression` class.
 *
 */
class Identifier : public Expression {
public:
  Identifier() = default;
  Identifier(const Token &, std::string &);

  Token token;
  std::string value;
  void expressionNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief LetStatement represents let statement,
 * for example `let a = 3`.
 *
 */
class LetStatement : public Statement {
public:
  LetStatement() = default;
  LetStatement(const Token &);
  Token token;
  std::unique_ptr<Identifier> name;
  std::unique_ptr<Expression> value;
  void statementNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief ReturnStatement represents return statement
 * for example, `return 3`
 */
class ReturnStatement : public Statement {
public:
  ReturnStatement() = default;
  ReturnStatement(const Token &);
  Token token;
  std::unique_ptr<Expression> returnValue;
  void statementNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief Expression could also be a statement.
 * for example: `x + 10`.
 *
 */
class ExpressionStatement : public Statement {
public:
  ExpressionStatement() = default;
  ExpressionStatement(const Token &);

  Token token;
  std::unique_ptr<Expression> expression;

  void statementNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief `BlockStatement` is a block (`{}`) which contains
 * many different statements.
 *
 */
class BlockStatement : public Statement {
public:
  BlockStatement() = default;
  BlockStatement(const Token &);

  Token token;
  std::vector<std::unique_ptr<Statement>> statements;

  void statementNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief `IntegerLiteral` is an expression which represents
 * the `5`.
 *
 */
class IntegerLiteral : public Expression {
public:
  IntegerLiteral() = default;
  IntegerLiteral(const Token &, int64_t);

  Token token;
  int64_t value;
  void expressionNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief `PrefixExpression` is an expression which
 * represents `<prefix operator><expression>;`
 *
 */
class PrefixExpression : public Expression {
public:
  PrefixExpression() = default;
  PrefixExpression(Token &, std::string &);

  Token token;
  std::string _operator;
  std::unique_ptr<Expression> right;

  void expressionNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief `InfixExpression` is an expression which
 * represents `<expression><infix operator><expression>`
 *
 */
class InfixExpression : public Expression {
public:
  InfixExpression() = default;
  InfixExpression(Token &, std::string &);

  Token token;
  std::unique_ptr<Expression> left;
  std::string _operator;
  std::unique_ptr<Expression> right;

  void expressionNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief represent the boolean value
 *
 */
class BooleanExpression : public Expression {
public:
  BooleanExpression() = default;
  BooleanExpression(const Token &, bool);

  Token token;
  bool value;

  void expressionNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief `IfExpression` contains three important aspects:
 * 1. the expression to evaluate to be true or false
 * 2. the true branch
 * 3. the false branch
 *
 */
class IfExpression : public Expression {
public:
  IfExpression() = default;
  IfExpression(const Token &);

  Token token;
  std::unique_ptr<Expression> condition;
  std::unique_ptr<BlockStatement> consequence;
  std::unique_ptr<BlockStatement> alternative;

  void expressionNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief This class represents the functional literal
 *
 */
class FunctionLiteral : public Expression {
public:
  Token token;
  std::vector<std::unique_ptr<Identifier>> parameters;
  std::unique_ptr<BlockStatement> body;

  FunctionLiteral() = default;
  FunctionLiteral(const Token &);

  void expressionNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief This class represents the call for example
 * `add(a + b, 5, 4)`
 *
 */
class CallExpression : public Expression {
public:
  Token token;
  std::unique_ptr<Expression> function;
  std::vector<std::unique_ptr<Expression>> arguments;

  CallExpression() = default;
  CallExpression(const Token &);

  void expressionNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

/**
 * @brief StringLiteral represents string
 *
 */
class StringLiteral : public Expression {
public:
  Token token;
  std::string value;

  StringLiteral() = default;
  StringLiteral(const Token &, const std::string &);

  void expressionNode() override;
  std::string tokenLiteral() override;
  std::string getString() override;
};

#endif  // _AST_AST_HPP_
