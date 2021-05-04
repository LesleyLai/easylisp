#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "interpreter.hpp"
#include "parser.hpp"

namespace {

void interpret_program(const Program& program)
{
  for (const auto& toplevel : program) {
    std::visit( //
        overloaded{
            [](const ExprPtr& expr) { eval(*expr, Environment::global()); },
            [](const Definition& definition) { add_definition(definition); },
        },
        toplevel);
  }
}

void interpret_source(std::string_view source)
{
  interpret_program(parse(source));
}

void interpret_toplevel_print_result(const Toplevel& toplevel)
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
      for (const auto& toplevel : parse(line)) {
        interpret_toplevel_print_result(toplevel);
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

  std::stringstream ss;
  // read file's buffer contents into streams
  ss << file.rdbuf();
  const auto source = ss.str();

  try {
    interpret_program(parse(source));
  } catch (const std::exception& e) {
    fmt::print("{}\n", e.what());
  }
}

} // anonymous namespace

auto main(int argc, const char* argv[]) -> int
{
  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    run_file(argv[1]);
  } else {
    fmt::print(stderr, "Usage: easylisp [filename]\n");
    std::exit(2);
  }
}
