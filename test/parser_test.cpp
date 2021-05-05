#include <catch2/catch.hpp>

#include <fmt/format.h>

#include "ApprovalTests.hpp"

#include "ast_printer.hpp"
#include "parser.hpp"

namespace {

[[nodiscard]] auto test_parse_program(std::string_view src) -> std::string
{
  std::string output = fmt::format("source:\n{}\n\n===============\n\n", src);
  try {
    Program program = parse(src);
    return output + fmt::format("{}", fmt::join(program, "\n"));
  } catch (const std::runtime_error& error) {
    return output + error.what();
  }
}

[[nodiscard]] auto verify_parse_program(std::string_view src)
{
  ApprovalTests::Approvals::verify(test_parse_program(src));
}

} // anonymous namespace

TEST_CASE("Parse empty lines")
{
  verify_parse_program("\n"
                       "\n"
                       "\n");
}

TEST_CASE("Parse bad syntax")
{
  SECTION("Single right paren") { verify_parse_program(")"); }
}

TEST_CASE("Parse plus")
{
  SECTION("Valid plus") { verify_parse_program("(+ 1 3 4)"); }

  SECTION("Missing right paren") { verify_parse_program("(+ 1 3 4"); }
}

TEST_CASE("Parse lambda")
{
  SECTION("Valid lambda") { verify_parse_program("(lambda (x y) (+ x y))"); }

  SECTION("Invalid parameters")
  {
    verify_parse_program("(lambda (42 11) (+ x 11))");
  }
}

TEST_CASE("Parser let")
{
  SECTION("Valid let")
  {
    verify_parse_program("(let ((x 10) (y 20)) (+ x y))");
  }
}

TEST_CASE("Parse if expression")
{
  SECTION("Valid if expression") { verify_parse_program("(if (> x y) x y)"); }

  SECTION("Missing right paren") { verify_parse_program("(if (> x y)"); }

  SECTION("Missing else") { verify_parse_program("(if (> x y) 10"); }

  SECTION("third branch") { verify_parse_program("(if (> x y) 10 20 30"); }
}

TEST_CASE("Parse definition")
{
  SECTION("Valid definition") { verify_parse_program("(define x 42)"); }

  SECTION("Definition without an expression")
  {
    verify_parse_program("(define x)");
  }

  SECTION("Definition with 2 expressions")
  {
    verify_parse_program("(define x 2 2)");
  }
}
