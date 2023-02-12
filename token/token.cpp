#include "token.hpp"

#include <string>
#include <string_view>

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

void Token::setToken(std::string_view &t, char ch) {
  Type = t;
  Literal = std::string{ch};
}
