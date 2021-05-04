#ifndef EASYEASYLISP_HPP
#define EASYEASYLISP_HPP

#include "ast.hpp"
#include "environment.hpp"
#include "value.hpp"

[[nodiscard]] auto eval(const Expr& expr, const EnvPtr& env) -> Value;
[[nodiscard]] auto apply(const Value& func, Values args) -> Value;
void add_definition(const Definition& definition);

#endif // EASYEASYLISP_HPP
