#ifndef EASYEASYLISP_HPP
#define EASYEASYLISP_HPP

#include <optional>

#include "ast.hpp"
#include "environment.hpp"
#include "value.hpp"

auto eval(const Expr& expr, const EnvPtr& env) -> Value;
[[nodiscard]] auto apply(const Value& func, Values args) -> Value;

void add_definition(const Definition& definition);
void require_module(const Require& require);

auto interpret_toplevel(const Toplevel& toplevel) -> std::optional<Value>;
void interpret(const Program& program);

#endif // EASYEASYLISP_HPP
