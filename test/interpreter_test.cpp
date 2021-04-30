#include <catch2/catch.hpp>

#include "ApprovalTests.hpp"
#include "fmt/format.h"
#include "interpreter.hpp"
#include "parser.hpp"

namespace {

[[nodiscard]] auto eval_and_print(std::string_view source) -> std::string
{
  return to_string(
      eval(*get<ExprPtr>(parse_toplevel(source)), Environment::global()));
}

[[nodiscard]] auto test_eval(std::string_view source) -> std::string
{
  std::string result;
  fmt::format_to(back_inserter(result), "source:\n{}\n\n", source);
  try {
    std::string out = eval_and_print(source);
    fmt::format_to(back_inserter(result), "output:\n{}\n", out);
  } catch (const std::runtime_error& err) {
    fmt::format_to(back_inserter(result), "error:\n{}\n", err.what());
  }
  return result;
}

void verify_eval(std::string_view source)
{
  ApprovalTests::Approvals::verify(test_eval(source));
}

} // namespace

TEST_CASE("Evaluator test")
{
  using ApprovalTests::Approvals;

  SECTION("plus") { verify_eval("(+ 1 3 4)"); };

  SECTION("cannot invoke + without arguments") { verify_eval("(+)"); };

  SECTION("minus") { verify_eval("(- 1 3 4)"); };

  SECTION("multiply") { verify_eval("(* 1 3 4)"); };

  SECTION("divide") { verify_eval("(/ 1 3 4)"); };

  SECTION("undefined variable") { verify_eval("x"); }

  SECTION("builtin proc +")
  {
    REQUIRE(eval_and_print("+") == "<builtin proc +>");
  }

  SECTION("Cannot apply to number") { verify_eval("(12)"); }

  SECTION("Cannot apply to list") { verify_eval("((list 1 2 3) 4)"); }

  SECTION("lambda expression")
  {
    REQUIRE(eval_and_print("(lambda (x y) (+ x y))") == "<proc (x y)>");
    REQUIRE(eval_and_print("((lambda (x y) (+ x y)) 3 4)") == "7");
  }

  SECTION("lambda with unmatching numbers of arguments")
  {
    Approvals::verify(test_eval("((lambda (x y) (+ x y)) 3 4 5)"));
  }

  SECTION("closure")
  {
    REQUIRE(eval_and_print("(((lambda (x) (lambda (y) (+ x y))) 3) 4)") == "7");
  }

  SECTION("let")
  {
    REQUIRE(eval_and_print("(let ((x 10) (y 20)) (+ x y))") == "30");
  }

  SECTION("true/false")
  {
    REQUIRE(eval_and_print("true") == "true");
    REQUIRE(eval_and_print("false") == "false");
  }

  SECTION("evaluate if expression")
  {
    REQUIRE(eval_and_print("(if (< 10 20) 10 20)") == "10");
    REQUIRE(eval_and_print("(if (> 10 20) 10 20)") == "20");
  }

  SECTION("if with none-boolean condition is not allowed")
  {
    Approvals::verify(test_eval("(if 1 10 20)"));
  }
}

TEST_CASE("List evaluation test")
{
  SECTION("null") { REQUIRE(eval_and_print("null") == "()"); }

  SECTION("cons")
  {
    REQUIRE(eval_and_print("(cons 1 2)") == "(1 . 2)");
    REQUIRE(eval_and_print("(cons 1 null)") == "(1)");
    REQUIRE(eval_and_print("(cons (cons 1 2) (list 3))") == "((1 . 2) 3)");
  }

  SECTION("cons with 3 arguments is invalid") { verify_eval("(cons 1 2 3)"); }

  SECTION("car")
  {
    REQUIRE(eval_and_print("(car (cons 1 2))") == "1");
    REQUIRE(eval_and_print("(car (list 1 2))") == "1");
  }

  SECTION("cons with empty list") { verify_eval("(car (list))"); }

  SECTION("cons with 2 arguments is invalid")
  {
    verify_eval("(car (cons 1 2) 2)");
  }

  SECTION("cdr")
  {
    REQUIRE(eval_and_print("(cdr (cons 1 2))") == "2");
    REQUIRE(eval_and_print("(cdr (list 1 2))") == "(2)");
  }

  SECTION("cdr with empty list") { verify_eval("(cdr (list))"); }

  SECTION("cdr with 2 arguments is invalid")
  {
    verify_eval("(cdr (cons 1 2) 2)");
  }

  SECTION("range")
  {
    REQUIRE(eval_and_print("(range 0 5)") == "(0 1 2 3 4)");
    REQUIRE(eval_and_print("(range 5 0)") == "()");
  }

  SECTION("map")
  {
    REQUIRE(eval_and_print("(map (lambda (x) (+ x 1)) (list 1 2 3 4 5))") ==
            "(2 3 4 5 6)");
  }

  SECTION("filter")
  {
    REQUIRE(eval_and_print("(filter (lambda (x) (> x 3)) (list 1 2 3 4 5))") ==
            "(4 5)");
  }

  SECTION("foldl")
  {
    REQUIRE(eval_and_print("(foldl + 0 (list 1 2 3 4 5))") == "15");
    REQUIRE(
        eval_and_print(
            "(foldl (lambda (x acc) (* (+ acc 1) x)) 0 (list 1 2 3 4 5))") ==
        "325");
  }

  SECTION("foldr")
  {
    REQUIRE(eval_and_print("(foldr + 0 (list 1 2 3 4 5))") == "15");
    REQUIRE(
        eval_and_print(
            "(foldr (lambda (x acc) (* (+ acc 1) x)) 0 (list 1 2 3 4 5))") ==
        "153");
  }
}

TEST_CASE("Type predicate test")
{
  REQUIRE(eval_and_print("(number? 1)") == "true");
  REQUIRE(eval_and_print("(number? true)") == "false");

  REQUIRE(eval_and_print("(boolean? true)") == "true");
  REQUIRE(eval_and_print("(boolean? 1)") == "false");

  REQUIRE(eval_and_print("(null? null)") == "true");
  REQUIRE(eval_and_print("(null? (list))") == "true");
  REQUIRE(eval_and_print("(null? (cons null null))") == "false");

  REQUIRE(eval_and_print("(pair? 1)") == "false");
  REQUIRE(eval_and_print("(pair? (list))") == "false");
  REQUIRE(eval_and_print("(pair? (list 1))") == "true");
  REQUIRE(eval_and_print("(pair? (cons 1 2))") == "true");

  REQUIRE(eval_and_print("(list? 1)") == "false");
  REQUIRE(eval_and_print("(list? (list))") == "true");
  REQUIRE(eval_and_print("(list? (list 1))") == "true");
  REQUIRE(eval_and_print("(list? (cons 1 2))") == "false");

  REQUIRE(eval_and_print("(procedural? 1)") == "false");
  REQUIRE(eval_and_print("(procedural? (cons 1 2))") == "false");
  REQUIRE(eval_and_print("(procedural? (lambda (x) x))") == "true");
  REQUIRE(eval_and_print("(procedural? +)") == "true");
}

TEST_CASE("Equality test")
{
  SECTION("eq?")
  {
    REQUIRE(eval_and_print("(eq? 1 1)") == "true");
    REQUIRE(eval_and_print("(eq? 1 2)") == "false");
    REQUIRE(eval_and_print("(eq? (cons 1 2) (cons 1 2))") == "false");
    REQUIRE(eval_and_print("(let ((x (cons 1 2))) (eq? x x))") == "true");
  }

  SECTION("eq? with no arguments") { verify_eval("(eq?)"); }

  SECTION("eq?")
  {
    REQUIRE(eval_and_print("(eq? 1 1)") == "true");
    REQUIRE(eval_and_print("(eq? 1 2)") == "false");
    REQUIRE(eval_and_print("(eq? (cons 1 2) (cons 1 2))") == "false");
    REQUIRE(eval_and_print("(let ((x (cons 1 2))) (eq? x x))") == "true");
  }

  SECTION("equal?")
  {
    REQUIRE(eval_and_print("(equal? 1 1)") == "true");
    REQUIRE(eval_and_print("(equal? 1 2)") == "false");
    REQUIRE(eval_and_print("(equal? (cons 1 2) (cons 1 2))") == "true");
    REQUIRE(eval_and_print("(let ((x (cons 1 2))) (equal? x x))") == "true");
  }

  SECTION("equal? with no arguments") { verify_eval("(equal?)"); }
}

TEST_CASE("Logic operator test")
{
  REQUIRE(eval_and_print("(not true)") == "false");
  REQUIRE(eval_and_print("(not false)") == "true");

  REQUIRE(eval_and_print("(and true true)") == "true");
  REQUIRE(eval_and_print("(and true false)") == "false");
  REQUIRE(eval_and_print("(and false false)") == "false");

  REQUIRE(eval_and_print("(or true true)") == "true");
  REQUIRE(eval_and_print("(or true false)") == "true");
  REQUIRE(eval_and_print("(or false false)") == "false");
}

TEST_CASE("Definition test")
{
  SECTION("define a variable")
  {
    interpret(parse_toplevel("(define x 100)"));
    REQUIRE(eval_and_print("(+ x 1)") == "101");
  }

  SECTION("variable shadowing in definition")
  {
    interpret(parse_toplevel("(define x 100)"));
    interpret(parse_toplevel("(define x 42)"));
    REQUIRE(eval_and_print("(+ x 1)") == "43");
  }

  SECTION("Recursion with definition")
  {
    interpret(parse_toplevel(
        "(define fact (lambda (x) (if (< x 2) x (* x (fact (- x 1))))))"));
    REQUIRE(eval_and_print("(fact 10)") == "3628800");
  }
}