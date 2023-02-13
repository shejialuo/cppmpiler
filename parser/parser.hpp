#ifndef _PARSER_PARSER_HPP_
#define _PARSER_PARSER_HPP_

#include "ast.hpp"
#include "lexer.hpp"
#include "token.hpp"

#include <memory>
#include <string_view>
#include <vector>

class Parser {
private:
  Lexer *lexer;
  Token currentToken;                 // current token
  Token peekToken;                    // next token
  std::vector<std::string> errors{};  // error information

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
  std::unique_ptr<ReturnStatement> parserReturnStatement();

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
};

#endif  // _PARSER_PARSER_HPP_
