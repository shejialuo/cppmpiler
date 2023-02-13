#ifndef _AST_AST_HPP_
#define _AST_AST_HPP_

#include "token.hpp"

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
  // TODO: currently we do not consider about Expression.
  virtual void expressionNode() = 0;
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

#endif  // _AST_AST_HPP_
