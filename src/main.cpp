#include <fmt/core.h>
#include <fstream>
#include <iostream>

#include "file_util.hpp"
#include "interpreter.hpp"
#include "parser.hpp"

namespace {
void repl()
{
  std::string line;
  Interpreter interpreter;

  while (true) {
    fmt::print(">> ");
    std::getline(std::cin, line);
    if (std::cin.fail() || std::cin.eof()) {
      std::cin.clear();
      break;
    }

    if (line == "(exit)") { std::exit(0); }
    try {
      for (const auto& toplevel : parse(line)) {
        if (const auto value_opt = interpreter.interpret_toplevel(toplevel);
            value_opt != std::nullopt) {
          fmt::print("{}\n", to_string(*value_opt));
        }
      }
    } catch (const std::exception& e) {
      fmt::print("{}\n", e.what());
    }
  }
}

void run_file(const char* filename)
{
  std::ifstream file{filename};

  if (!file.is_open()) {
    fmt::print(stderr, "Cannot open file {}\n", filename);
    std::exit(1);
  }

  const auto source = file_to_string(file);
  Interpreter interpreter;
  try {
    interpreter.interpret(parse(source));
  } catch (const std::exception& e) {
    fmt::print("{}\n", e.what());
  }
}

} // anonymous namespace

auto main(int argc, const char* argv[]) -> int
try {
  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    run_file(argv[1]);
  } else {
    fmt::print(stderr, "Usage: easylisp [filename]\n");
    std::exit(2);
  }
} catch (const std::exception& e) {
  fmt::print("Uncaught exception:\n{}\n", e.what());
}
