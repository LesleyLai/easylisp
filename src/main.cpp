#include <fmt/core.h>
#include <iostream>
#include <sstream>

#include "interpreter.hpp"
#include "parser.hpp"

namespace {

void interpret_toplevel(const Toplevel& toplevel)
{
  std::visit( //
      overloaded{
          [](const ExprPtr& expr) {
            const auto val = eval(*expr, Environment::global());
            fmt::print("{}\n", to_string(val));
          },
          [](const Definition& definition) { add_definition(definition); },
      },
      toplevel);
}

void repl()
{
  std::string line;
  while (true) {
    fmt::print(">> ");
    std::getline(std::cin, line);
    if (line.empty()) continue;
    if (line == "(exit)") { std::exit(0); }
    try {
      for (const auto& toplevel : parse(line)) { interpret_toplevel(toplevel); }
    } catch (const std::exception& e) {
      fmt::print("{}\n", e.what());
    }
  }
}

} // anonymous namespace

auto main() -> int { repl(); }
