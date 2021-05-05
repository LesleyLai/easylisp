#include <string_view>

#include "scanner.hpp"

#include <catch2/catch.hpp>

[[nodiscard]] static auto operator==(Token lhs, Token rhs) -> bool
{
  if (lhs.type != rhs.type || lhs.lexeme != rhs.lexeme) { return false; }
  if (lhs.type == TokenType::number) {
    return lhs.data.number == rhs.data.number;
  }
  return true;
}

TEST_CASE("Scanner empty string test")
{
  Scanner scanner{""};
  REQUIRE(scanner->type == TokenType::eof);
}

TEST_CASE("Scanner Test")
{
  auto itr = Scanner{"(+ 42 0.5) ;; comments"};
  Token expected[] = {
      {.type = TokenType::left_paren, .lexeme = "("},
      {.type = TokenType::identifier, .lexeme = "+"},
      {.type = TokenType::number, .lexeme = "42", .data = {.number = 42}},
      {.type = TokenType::number, .lexeme = "0.5", .data = {.number = 0.5}},
      {.type = TokenType::right_paren, .lexeme = ")"},
  };

  std::vector<Token> results;
  while (itr->type != TokenType::eof) {
    results.push_back(*itr);
    ++itr;
  }
  REQUIRE(std::ranges::equal(results, expected));
}