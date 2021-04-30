#include <catch2/catch.hpp>

#include "environment.hpp"

TEST_CASE("Global Environment")
{
  Environment global_env{Environment::create_global_t{}};

  SECTION("+")
  {
    const auto* plus = global_env.find("+");
    REQUIRE(plus != nullptr);
    REQUIRE(
        dynamic_cast<const BuiltinProc&>(*std::get<ObjectPtr>(*plus)).name ==
        "+");
  }

  SECTION("None existing function")
  {
    REQUIRE(global_env.find("fff") == nullptr);
  }
}

TEST_CASE("Local Environment")
{
  Environment env{Environment::global()};
  env.add("x", Value{42.0});

  SECTION("Get local variable")
  {
    auto* x = env.find("x");
    REQUIRE(x != nullptr);
    REQUIRE(std::get<double>(*x) == 42.0);
  }

  SECTION("Get variable from parent scope")
  {
    auto* plus = env.find("+");
    REQUIRE(plus != nullptr);
    REQUIRE(
        dynamic_cast<const BuiltinProc&>(*std::get<ObjectPtr>(*plus)).name ==
        "+");
  }
}