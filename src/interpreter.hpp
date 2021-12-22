#ifndef EASYEASYLISP_HPP
#define EASYEASYLISP_HPP

#include <optional>

#include "ast.hpp"
#include "environment.hpp"
#include "value.hpp"

[[nodiscard]] auto eval(const Expr& expr, const EnvPtr& env) -> Value;
[[nodiscard]] auto apply(const Value& func, Values args) -> Value;

class Interpreter {
  std::shared_ptr<Environment> global_env_ =
      std::make_shared<Environment>(Environment::create_global);

public:
  void add_definition(const Definition& definition);
  void require_module(const Require& require);

  auto interpret_toplevel(const Toplevel& toplevel) -> std::optional<Value>;
  void interpret(const Program& program);
};

#endif // EASYEASYLISP_HPP
