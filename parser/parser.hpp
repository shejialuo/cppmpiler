#ifndef _PARSER_PARSER_HPP_
#define _PARSER_PARSER_HPP_

#include "ast.hpp"
#include "lexer.hpp"
#include "token.hpp"

#include <functional>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

using prefixParseFn = std::function<std::unique_ptr<Expression>()>;
using infixParseFn = std::function<std::unique_ptr<Expression>(std::unique_ptr<Expression>)>;

enum class Precedence;

class Parser {
private:
  Lexer *lexer;
  Token currentToken;                 // current token
  Token peekToken;                    // next token
  std::vector<std::string> errors{};  // error information

  std::unordered_map<TokenType_t, prefixParseFn> prefixParseFns;
  std::unordered_map<TokenType_t, infixParseFn> infixParseFns;

  std::unordered_map<TokenType_t, Precedence> precedences;

public:
  Parser() = delete;
  Parser(Lexer *l);

  /**
   * @brief get the next token
   *
   */
  void nextToken();

  /**
   * @brief Parse the program
   *
   * @return unique_ptr<Program>
   */
  std::unique_ptr<Program> parseProgram();

  /**
   * @brief Parse the statement
   *
   * @return unique_ptr<Statement>
   */
  std::unique_ptr<Statement> parseStatement();

  /**
   * @brief Parse the let statement
   *
   * @return unique_ptr<LetStatement>
   */
  std::unique_ptr<LetStatement> parseLetStatement();

  /**
   * @brief Parse the return statement
   *
   * @return std::unique_ptr<ReturnStatement>
   */
  std::unique_ptr<ReturnStatement> parseReturnStatement();

  /**
   * @brief Parse the expression
   *
   * @return std::unique_ptr<Expression>
   */
  std::unique_ptr<Expression> parseExpression(Precedence precedence);

  /**
   * @brief Parse the expression statement
   *
   * @return std::unique_ptr<ExpressionStatement>
   */
  std::unique_ptr<ExpressionStatement> parseExpressionStatement();

  /**
   * @brief This function need to be registered in the `prefixParseFns`.
   *
   * @return std::unique_ptr<Expression>
   */
  std::unique_ptr<Expression> parseIdentifier();

  /**
   * @brief This function need to be registered in the `prefixParseFns`.
   *
   * @return std::unique_ptr<Expression>
   */
  std::unique_ptr<Expression> parseIntegerLiteral();

  /**
   * @brief This function is need to be registered in the `prefixParseFns`.
   * It is used to parse the `()`, we just need to change the next
   * precedence to be the lowest.
   *
   * @return std::unique_ptr<Expression>
   */
  std::unique_ptr<Expression> ParseGroupedExpression();

  /**
   * @brief  This function need to be registered in the `prefixParseFns`.
   *
   * @return std::unique_ptr<Expression>
   */
  std::unique_ptr<Expression> parseIfExpression();

  /**
   * @brief parse the block statement `{}`.
   *
   * @return std::unique_ptr<BlockStatement>
   */
  std::unique_ptr<BlockStatement> parseBlockStatement();

  /**
   * @brief This function is used to parse the prefix expression.
   * need to be registered in `prefixParseFns`.
   *
   * @return std::unique_ptr<PrefixExpression>
   */
  std::unique_ptr<PrefixExpression> parsePrefixExpression();

  /**
   * @brief This function is used to parse the infix expression.
   * need to be registered in `infixParseFns`.
   *
   * @return std::unique_ptr<InfixExpression>
   */
  std::unique_ptr<InfixExpression> parseInfixExpression(std::unique_ptr<Expression> left);

  /**
   * @brief This function is used to parse the boolean expression.
   * need to be registered in `prefixParseFns`.
   *
   * @return std::unique_ptr<BooleanExpression>
   */
  std::unique_ptr<BooleanExpression> parseBooleanExpression();

  /**
   * @brief This function is use to parse the function.
   * need to be registered in `prefixParseFns`.
   *
   * @return std::unique_ptr<FunctionLiteral>
   */
  std::unique_ptr<FunctionLiteral> parseFunctionLiteral();

  /**
   * @brief Parse the function parameters
   *
   * @return std::vector<std::unique_ptr<Identifier>>
   */
  std::vector<std::unique_ptr<Identifier>> parseFunctionParameters();

  /**
   * @brief Parse the Call Expression such as `add(x,y);`
   *
   * @return std::unique_ptr<CallExpression>
   */
  std::unique_ptr<CallExpression> parseCallExpression(std::unique_ptr<Expression> function);

  /**
   * @brief Parse the call arguments
   *
   * @return std::vector<std::unique_ptr<Expression>>
   */
  std::vector<std::unique_ptr<Expression>> parseCallArguments();

  /**
   * @brief Parse the string literal
   *
   * @return std::unique_ptr<StringLiteral>
   */
  std::unique_ptr<StringLiteral> parseStringLiteral();

  /**
   * @brief A helper function to tell whether
   * `currentToken == t`
   *
   */
  bool currentTokenIs(std::string_view &t);

  /**
   * @brief A helper function to tell whether
   * `peekToken == t`
   *
   */
  bool peekTokenIs(std::string_view &t);

  /**
   * @brief If `peekTokenIs(t)` is true, call
   * `nextToken`
   *
   */
  bool expectPeek(std::string_view &t);

  /**
   * @brief Get the errors object
   *
   * @return std::vector<std::string>&
   */
  std::vector<std::string> &getErrors() { return errors; }

  /**
   * @brief Auxiliary functions to add errors
   *
   */
  void peekError(std::string_view &t);

  /**
   * @brief Get current operator's precedence
   *
   * @return Precedence
   */
  Precedence currentPrecedence();

  /**
   * @brief Get next operator's precedence
   *
   * @return Precedence
   */
  Precedence peekPrecedence();

  void noPrefixParseFnError(TokenType_t &tokenType);

  void registerPrefix(const TokenType_t &tokenType, prefixParseFn fn);

  void registerInfix(const TokenType_t &tokenType, infixParseFn fn);
};

#endif  // _PARSER_PARSER_HPP_
