#include <fmt/core.h>
#include <iostream>
#include <sstream>

#include "interpreter.hpp"
#include "parser.hpp"

auto main() -> int
{
  std::string line;
  while (true) {
    fmt::print(">> ");
    std::getline(std::cin, line);
    if (line.empty()) continue;
    if (line == "(exit)") { std::exit(0); }
    try {
      interpret(parse_toplevel(line));
    } catch (const std::exception& e) {
      fmt::print("{}\n", e.what());
    }
  }
}
