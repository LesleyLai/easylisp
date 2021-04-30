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
  struct create_global_t {};
  explicit Environment(create_global_t);
  explicit Environment(EnvPtr parent) : parent_(MOV(parent)) {}

  [[nodiscard]] static auto global() -> std::shared_ptr<Environment>&;
  [[nodiscard]] auto find(const std::string& var) const -> const Value*;
  void add(std::string variable, Value value);
};

#endif // EASYLISP_ENVIRONMENT_HPP
