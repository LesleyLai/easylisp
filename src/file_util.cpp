#include "file_util.hpp"

#include <sstream>

[[nodiscard]] auto file_to_string(const std::ifstream& file) -> std::string
{
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}