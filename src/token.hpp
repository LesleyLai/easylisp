#ifndef EASYLISP_TOKEN_HPP
#define EASYLISP_TOKEN_HPP

#include <cstdint>
#include <string_view>

/// @brief The type of a token
enum class TokenType {
  eof,
  left_paren,
  right_paren,
  number,
  identifier,
  keyword_if,
  keyword_lambda,
  keyword_let,
};

/// @brief A type alias that defines the number type for our language
using Number = double;

/**
 * @brief A token holds information about a slice of source code
 */
struct Token {
  TokenType type = TokenType::eof;
  union Data {
    Number number = 0;
    std::string_view lexeme;
  } data = {};
};

#endif // EASYLISP_TOKEN_HPP
