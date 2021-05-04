#include <catch2/catch.hpp>

#include "ApprovalTests.hpp"
#include "fmt/format.h"
#include "interpreter.hpp"
#include "parser.hpp"

namespace {

[[nodiscard]] auto interpret_and_print(std::string_view source) -> std::string
{
  Program program = parse(source);

  std::vector<std::string> results;
  for (const auto& toplevel : program) {
    std::visit( //
        overloaded{
            [&](const ExprPtr& expr) {
              const auto val = eval(*expr, Environment::global());
              results.push_back(to_string(val));
            },
            [](const Definition& definition) { add_definition(definition); },
        },
        toplevel);
  }
  return fmt::format("{}", fmt::join(results, "\n"));
}

void verify_interpret(std::string_view source)
{
  std::string result;
  fmt::format_to(back_inserter(result), "source:\n{}\n\n", source);
  try {
    std::string out = interpret_and_print(source);
    fmt::format_to(back_inserter(result), "output:\n{}\n", out);
  } catch (const std::runtime_error& err) {
    fmt::format_to(back_inserter(result), "error:\n{}\n", err.what());
  }

  ApprovalTests::Approvals::verify(result);
}

} // namespace

TEST_CASE("Evaluator test")
{
  SECTION("plus") { verify_interpret("(+ 1 3 4)"); };

  SECTION("cannot invoke + without arguments") { verify_interpret("(+)"); };

  SECTION("minus") { verify_interpret("(- 1 3 4)"); };

  SECTION("multiply") { verify_interpret("(* 1 3 4)"); };

  SECTION("divide") { verify_interpret("(/ 1 3 4)"); };

  SECTION("undefined variable") { verify_interpret("x"); }

  SECTION("builtin proc +")
  {
    REQUIRE(interpret_and_print("+") == "<builtin proc +>");
  }

  SECTION("Cannot apply to number") { verify_interpret("(12)"); }

  SECTION("Cannot apply to list") { verify_interpret("((list 1 2 3) 4)"); }

  SECTION("lambda expression")
  {
    REQUIRE(interpret_and_print("(lambda (x y) (+ x y))") == "<proc (x y)>");
    REQUIRE(interpret_and_print("((lambda (x y) (+ x y)) 3 4)") == "7");
  }

  SECTION("lambda with unmatching numbers of arguments")
  {
    verify_interpret("((lambda (x y) (+ x y)) 3 4 5)");
  }

  SECTION("closure")
  {
    REQUIRE(interpret_and_print("(((lambda (x) (lambda (y) (+ x y))) 3) 4)") ==
            "7");
  }

  SECTION("let")
  {
    REQUIRE(interpret_and_print("(let ((x 10) (y 20)) (+ x y))") == "30");
  }

  SECTION("true/false")
  {
    REQUIRE(interpret_and_print("true") == "true");
    REQUIRE(interpret_and_print("false") == "false");
  }

  SECTION("evaluate if expression")
  {
    REQUIRE(interpret_and_print("(if (< 10 20) 10 20)") == "10");
    REQUIRE(interpret_and_print("(if (> 10 20) 10 20)") == "20");
  }

  SECTION("if with none-boolean condition is not allowed")
  {
    verify_interpret("(if 1 10 20)");
  }
}

TEST_CASE("List evaluation test")
{
  SECTION("null") { REQUIRE(interpret_and_print("null") == "()"); }

  SECTION("cons")
  {
    REQUIRE(interpret_and_print("(cons 1 2)") == "(1 . 2)");
    REQUIRE(interpret_and_print("(cons 1 null)") == "(1)");
    REQUIRE(interpret_and_print("(cons (cons 1 2) (list 3))") == "((1 . 2) 3)");
  }

  SECTION("cons with 3 arguments is invalid")
  {
    verify_interpret("(cons 1 2 3)");
  }

  SECTION("car")
  {
    REQUIRE(interpret_and_print("(car (cons 1 2))") == "1");
    REQUIRE(interpret_and_print("(car (list 1 2))") == "1");
  }

  SECTION("cons with empty list") { verify_interpret("(car (list))"); }

  SECTION("cons with 2 arguments is invalid")
  {
    verify_interpret("(car (cons 1 2) 2)");
  }

  SECTION("cdr")
  {
    REQUIRE(interpret_and_print("(cdr (cons 1 2))") == "2");
    REQUIRE(interpret_and_print("(cdr (list 1 2))") == "(2)");
  }

  SECTION("cdr with empty list") { verify_interpret("(cdr (list))"); }

  SECTION("cdr with 2 arguments is invalid")
  {
    verify_interpret("(cdr (cons 1 2) 2)");
  }

  SECTION("range")
  {
    REQUIRE(interpret_and_print("(range 0 5)") == "(0 1 2 3 4)");
    REQUIRE(interpret_and_print("(range 5 0)") == "()");
  }

  SECTION("map")
  {
    REQUIRE(
        interpret_and_print("(map (lambda (x) (+ x 1)) (list 1 2 3 4 5))") ==
        "(2 3 4 5 6)");
  }

  SECTION("filter")
  {
    REQUIRE(interpret_and_print(
                "(filter (lambda (x) (> x 3)) (list 1 2 3 4 5))") == "(4 5)");
  }

  SECTION("foldl")
  {
    REQUIRE(interpret_and_print("(foldl + 0 (list 1 2 3 4 5))") == "15");
    REQUIRE(
        interpret_and_print(
            "(foldl (lambda (x acc) (* (+ acc 1) x)) 0 (list 1 2 3 4 5))") ==
        "325");
  }

  SECTION("foldr")
  {
    REQUIRE(interpret_and_print("(foldr + 0 (list 1 2 3 4 5))") == "15");
    REQUIRE(
        interpret_and_print(
            "(foldr (lambda (x acc) (* (+ acc 1) x)) 0 (list 1 2 3 4 5))") ==
        "153");
  }
}

TEST_CASE("Type predicate test")
{
  REQUIRE(interpret_and_print("(number? 1)") == "true");
  REQUIRE(interpret_and_print("(number? true)") == "false");

  REQUIRE(interpret_and_print("(boolean? true)") == "true");
  REQUIRE(interpret_and_print("(boolean? 1)") == "false");

  REQUIRE(interpret_and_print("(null? null)") == "true");
  REQUIRE(interpret_and_print("(null? (list))") == "true");
  REQUIRE(interpret_and_print("(null? (cons null null))") == "false");

  REQUIRE(interpret_and_print("(pair? 1)") == "false");
  REQUIRE(interpret_and_print("(pair? (list))") == "false");
  REQUIRE(interpret_and_print("(pair? (list 1))") == "true");
  REQUIRE(interpret_and_print("(pair? (cons 1 2))") == "true");

  REQUIRE(interpret_and_print("(list? 1)") == "false");
  REQUIRE(interpret_and_print("(list? (list))") == "true");
  REQUIRE(interpret_and_print("(list? (list 1))") == "true");
  REQUIRE(interpret_and_print("(list? (cons 1 2))") == "false");

  REQUIRE(interpret_and_print("(procedural? 1)") == "false");
  REQUIRE(interpret_and_print("(procedural? (cons 1 2))") == "false");
  REQUIRE(interpret_and_print("(procedural? (lambda (x) x))") == "true");
  REQUIRE(interpret_and_print("(procedural? +)") == "true");
}

TEST_CASE("Equality test")
{
  SECTION("eq?")
  {
    REQUIRE(interpret_and_print("(eq? 1 1)") == "true");
    REQUIRE(interpret_and_print("(eq? 1 2)") == "false");
    REQUIRE(interpret_and_print("(eq? (cons 1 2) (cons 1 2))") == "false");
    REQUIRE(interpret_and_print("(let ((x (cons 1 2))) (eq? x x))") == "true");
  }

  SECTION("eq? with no arguments") { verify_interpret("(eq?)"); }

  SECTION("eq?")
  {
    REQUIRE(interpret_and_print("(eq? 1 1)") == "true");
    REQUIRE(interpret_and_print("(eq? 1 2)") == "false");
    REQUIRE(interpret_and_print("(eq? (cons 1 2) (cons 1 2))") == "false");
    REQUIRE(interpret_and_print("(let ((x (cons 1 2))) (eq? x x))") == "true");
  }

  SECTION("equal?")
  {
    REQUIRE(interpret_and_print("(equal? 1 1)") == "true");
    REQUIRE(interpret_and_print("(equal? 1 2)") == "false");
    REQUIRE(interpret_and_print("(equal? (cons 1 2) (cons 1 2))") == "true");
    REQUIRE(interpret_and_print("(let ((x (cons 1 2))) (equal? x x))") ==
            "true");
  }

  SECTION("equal? with no arguments") { verify_interpret("(equal?)"); }
}

TEST_CASE("Logic operator test")
{
  REQUIRE(interpret_and_print("(not true)") == "false");
  REQUIRE(interpret_and_print("(not false)") == "true");

  REQUIRE(interpret_and_print("(and true true)") == "true");
  REQUIRE(interpret_and_print("(and true false)") == "false");
  REQUIRE(interpret_and_print("(and false false)") == "false");

  REQUIRE(interpret_and_print("(or true true)") == "true");
  REQUIRE(interpret_and_print("(or true false)") == "true");
  REQUIRE(interpret_and_print("(or false false)") == "false");
}

TEST_CASE("Definition test")
{
  SECTION("define a variable")
  {
    REQUIRE(interpret_and_print("(define x 100) (+ x 1)") == "101");
  }

  SECTION("variable shadowing in definition")
  {
    REQUIRE(interpret_and_print("(define x 100)"
                                "(define x 42)"
                                "(+ x 1)") == "43");
  }

  SECTION("Recursion with definition")
  {
    REQUIRE(interpret_and_print(
                "(define fact (lambda (x) (if (< x 2) x (* x (fact (- x 1))))))"
                "(fact 10)") == "3628800");
  }
}