#include <catch2/catch.hpp>

#include "ast.hpp"

namespace {

auto var(std::string name)
{
  return std::make_shared<VariableExpr>(MOV(name));
}

auto number(double val)
{
  return std::make_shared<NumberExpr>(val);
}

template <typename... Args> auto apply(ExprPtr&& func, Args&&... args)
{
  std::vector<ExprPtr> arguments;
  (arguments.push_back(FWD(args)), ...);
  return std::make_shared<ApplyExpr>(MOV(func), MOV(arguments));
}

} // anonymous namespace

TEST_CASE("AST Print Test")
{
  const auto expr = apply(var("+"), number(42), number(21));
  REQUIRE(to_string(*expr) == "(app (var +) (const 42) (const 21))");
}
