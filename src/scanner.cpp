#include "scanner.hpp"

#include <fast_float/fast_float.h>

#include <algorithm>

namespace {
[[nodiscard]] auto is_space(char c) -> bool
{
  return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

[[nodiscard]] auto not_valid_identifier_char(char c) -> bool
{
  return is_space(c) || c == '(' || c == ')' || c == '\'';
}
} // anonymous namespace

void Scanner::advance()
{
  consume_whitespaces();

  if (is_at_end()) {
    current_token_ = Token{TokenType::eof};
    return;
  }

  switch (peek()) {
  case '(':
    current_token_ = Token{TokenType::left_paren};
    ++begin_;
    return;
  case ')':
    current_token_ = Token{TokenType::right_paren};
    ++begin_;
    return;
  }

  Number number = 0;
  if (auto [p, ec] = fast_float::from_chars(begin_, end_, number);
      ec == std::errc()) {
    current_token_ = Token{.type = TokenType::number, .data{.number = number}};
    begin_ = p;
    return;
  }

  find_identifier();
}

auto Scanner::check_keyword(unsigned int start_offset, std::string_view rest,
                            TokenType type) -> TokenType
{
  const char* start = begin_ + start_offset;
  if (std::ssize(rest) <= end_ - start &&
      std::string_view{start, rest.size()} == rest) {
    return type;
  }

  return TokenType::identifier;
}

[[nodiscard]] auto Scanner::identifier_type() -> TokenType
{
  switch (peek()) {
  case 'i':
    return check_keyword(1, "f", TokenType::keyword_if);
  case 'l':
    switch (peek_next()) {
    case 'a':
      return check_keyword(2, "mbda", TokenType::keyword_lambda);
    case 'e':
      return check_keyword(2, "t", TokenType::keyword_let);
    default:
      break;
    }
    break;
  default:
    break;
  }
  return TokenType::identifier;
}

void Scanner::find_identifier()
{
  const char* ident_end = std::find_if(begin_, end_, not_valid_identifier_char);
  current_token_ =
      Token{.type = identifier_type(),
            .data = {.lexeme = std::string_view{begin_, ident_end}}};
  begin_ = ident_end;
}

void Scanner::consume_whitespaces()
{
  while (true) {
    const char c = peek();

    if (is_space(c)) {
      ++begin_;
      continue;
    }
    if (c == ';') {
      while (peek() != '\n' && !is_at_end()) { ++begin_; }
      continue;
    }
    break;
  }
}
[[nodiscard]] auto Scanner::is_at_end() -> bool { return begin_ == end_; }
[[nodiscard]] auto Scanner::peek() -> char { return peek_forward(0); }
[[nodiscard]] auto Scanner::peek_next() -> char { return peek_forward(1); }
[[nodiscard]] auto Scanner::peek_forward(int n) -> char
{
  return (end_ - begin_ > n) ? begin_[n] : '\0';
}
