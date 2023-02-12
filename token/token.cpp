#include "token.hpp"

#include <string>
#include <string_view>
#include <unordered_map>

std::unordered_map<std::string, TokenType_t> Keywords{{"fn", std::string(TokenTypes::FUNCTION)},
                                                      {"let", std::string(TokenTypes::LET)}};

std::string_view TokenTypes::ILLEGAL{"ILLEGAL"};
std::string_view TokenTypes::_EOF{"EOF"};
std::string_view TokenTypes::IDENT{"IDENT"};
std::string_view TokenTypes::INT{"INT"};
std::string_view TokenTypes::ASSIGN{"="};
std::string_view TokenTypes::PLUS{"+"};
std::string_view TokenTypes::COMMA{","};
std::string_view TokenTypes::SEMICOLON{";"};
std::string_view TokenTypes::LPAREN{"("};
std::string_view TokenTypes::RPAREN{")"};
std::string_view TokenTypes::LBRACE{"{"};
std::string_view TokenTypes::RBRACE{"}"};
std::string_view TokenTypes::FUNCTION{"FUNCTION"};
std::string_view TokenTypes::LET{"LET"};
std::string_view TokenTypes::MINUS{"-"};
std::string_view TokenTypes::BANG{"!"};
std::string_view TokenTypes::ASTERISK{"*"};
std::string_view TokenTypes::SLASH{"/"};
std::string_view TokenTypes::LT{"<"};
std::string_view TokenTypes::GT{">"};

void Token::setToken(std::string_view &t, char ch) {
  Type = t;
  Literal = std::string{ch};
}

TokenType_t Token::lookupIdentifiers(std::string &identifiers) {
  if (Keywords.count(identifiers)) {
    return Keywords[identifiers];
  } else {
    return std::string{TokenTypes::IDENT};
  }
}