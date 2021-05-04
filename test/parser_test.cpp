#include <catch2/catch.hpp>

#include <fmt/format.h>

#include "ApprovalTests.hpp"

#include "ast_printer.hpp"
#include "parser.hpp"

namespace {

[[nodiscard]] auto test_parse_toplevel(std::string_view src) -> std::string
{
  std::string output = fmt::format("source:\n{}\n\n===============\n\n", src);
  try {
    Program program = parse(src);
    return output + fmt::format("{}", fmt::join(program, "\n"));
  } catch (const std::runtime_error& error) {
    return output + error.what();
  }
}

[[nodiscard]] auto verify_toplevel(std::string_view src)
{
  ApprovalTests::Approvals::verify(test_parse_toplevel(src));
}

} // anonymous namespace

TEST_CASE("Parse bad syntax")
{
  SECTION("Single right paren") { verify_toplevel(")"); }
}

TEST_CASE("Parse plus")
{
  SECTION("Valid plus") { verify_toplevel("(+ 1 3 4)"); }

  SECTION("Missing right paren") { verify_toplevel("(+ 1 3 4"); }
}

TEST_CASE("Parse lambda")
{
  SECTION("Valid lambda") { verify_toplevel("(lambda (x y) (+ x y))"); }

  SECTION("Invalid parameters")
  {
    verify_toplevel("(lambda (42 11) (+ x 11))");
  }
}

TEST_CASE("Parser let")
{
  SECTION("Valid let") { verify_toplevel("(let ((x 10) (y 20)) (+ x y))"); }
}

TEST_CASE("Parse if expression")
{
  SECTION("Valid if expression") { verify_toplevel("(if (> x y) x y)"); }

  SECTION("Missing right paren") { verify_toplevel("(if (> x y)"); }

  SECTION("Missing else") { verify_toplevel("(if (> x y) 10"); }

  SECTION("third branch") { verify_toplevel("(if (> x y) 10 20 30"); }
}

TEST_CASE("Parse definition")
{
  SECTION("Valid definition") { verify_toplevel("(define x 42)"); }

  SECTION("Definition without an expression") { verify_toplevel("(define x)"); }

  SECTION("Definition with 2 expressions")
  {
    verify_toplevel("(define x 2 2)");
  }
}