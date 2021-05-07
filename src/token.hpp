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
  keyword_define,
  keyword_if,
  keyword_lambda,
  keyword_let,
  keyword_require,
};

/// @brief A type alias that defines the number type for our language
using Number = double;

/**
 * @brief A token holds information about a slice of source code
 */
struct Token {
  TokenType type = TokenType::eof;
  std::string_view lexeme;
  union Data {
    Number number = 0;
  } data = {};
};

#endif // EASYLISP_TOKEN_HPP
