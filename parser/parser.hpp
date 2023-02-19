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
using infixParseFn = std::function<std::unique_ptr<Expression>(Expression *)>;

enum class Precedence;

class Parser {
private:
  Lexer *lexer;
  Token currentToken;                 // current token
  Token peekToken;                    // next token
  std::vector<std::string> errors{};  // error information

  std::unordered_map<TokenType_t, prefixParseFn> prefixParseFns;
  std::unordered_map<TokenType_t, infixParseFn> infixParseFns;

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
   * @brief This function is used to parse the prefix expression.
   * need to be registered in `prefixParseFns`.
   *
   * @return std::unique_ptr<PrefixExpression>
   */
  std::unique_ptr<PrefixExpression> parsePrefixExpression();

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

  void noPrefixParseFnError(TokenType_t &tokenType);

  void registerPrefix(TokenType_t &tokenType, prefixParseFn fn);

  void registerInfix(TokenType_t &tokenType, infixParseFn fn);
};

#endif  // _PARSER_PARSER_HPP_
