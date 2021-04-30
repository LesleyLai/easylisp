#include <string_view>

#include "scanner.hpp"

#include <catch2/catch.hpp>

[[nodiscard]] static auto operator==(Token lhs, Token rhs) -> bool
{
  if (lhs.type != rhs.type) { return false; }
  switch (lhs.type) {
  case TokenType::number:
    return lhs.data.number == rhs.data.number;

  case TokenType::identifier:
    return lhs.data.lexeme == rhs.data.lexeme;

  default:
    return true;
  }
}

TEST_CASE("Scanner empty string test")
{
  Scanner scanner{""};
  REQUIRE(scanner->type == TokenType::eof);
}

TEST_CASE("Scanner Test")
{
  auto itr = Scanner{"(+ 42 0.5)"};
  Token first = *itr;
  REQUIRE(first.type == TokenType::left_paren);

  REQUIRE(first == first);

  Token second = *(++itr);
  REQUIRE(second.type == TokenType::identifier);
  REQUIRE(second.data.lexeme == "+");

  REQUIRE(second == second);
  REQUIRE(second != first);

  Token third = *(++itr);
  REQUIRE(third.type == TokenType::number);
  REQUIRE(third.data.number == 42);

  REQUIRE(third == third);
  REQUIRE(second != third);

  Token fourth = *(++itr);
  REQUIRE(fourth.type == TokenType::number);
  REQUIRE(fourth.data.number == 0.5);

  Token fifth = *(++itr);
  REQUIRE(fifth.type == TokenType::right_paren);

  ++itr;
  REQUIRE(itr->type == TokenType::eof);
}