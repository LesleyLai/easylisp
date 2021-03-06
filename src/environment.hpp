#ifndef EASYLISP_ENVIRONMENT_HPP
#define EASYLISP_ENVIRONMENT_HPP

#include "value.hpp"
#include <memory>
#include <string>
#include <unordered_map>

class Environment {
  std::unordered_map<std::string, Value> bindings_;
  EnvPtr parent_ = nullptr;

public:
  static constexpr struct create_global_t {
  } create_global{};
  explicit Environment(create_global_t);
  explicit Environment(EnvPtr parent) : parent_(MOV(parent)) {}

  [[nodiscard]] auto find(const std::string& var) const -> const Value*;
  void add(std::string variable, Value value);
};

#endif // EASYLISP_ENVIRONMENT_HPP
