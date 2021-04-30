#ifndef EASYLISP_SCANNER_HPP
#define EASYLISP_SCANNER_HPP

#include "token.hpp"

/**
 * @brief The scanner iterator scans the source code and creates tokens
 */
class Scanner {
  const char* begin_ = nullptr;
  const char* end_ = nullptr;
  Token current_token_;

public:
  Scanner() = default;
  explicit Scanner(std::string_view source)
      : begin_{source.data()}, end_{source.data() + source.size()}
  {
    advance();
  }

  [[nodiscard]] auto operator->() -> Token*
  {
    return &current_token_;
  }

  [[nodiscard]] auto operator*() -> Token
  {
    return current_token_;
  }

  auto operator++() -> Scanner&
  {
    advance();
    return *this;
  }

private:
  void advance();
  auto is_at_end() -> bool;
  auto peek() -> char;
  auto peek_next() -> char;
  auto peek_forward(int n) -> char;
  void consume_whitespaces();
  void find_identifier();
  auto identifier_type() -> TokenType;
  auto check_keyword(unsigned int start_offset, std::string_view rest,
                     TokenType type) -> TokenType;
};

#endif // EASYLISP_SCANNER_HPP
