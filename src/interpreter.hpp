#ifndef EASYEASYLISP_HPP
#define EASYEASYLISP_HPP

#include "ast.hpp"
#include "environment.hpp"
#include "value.hpp"

#include <span>

auto eval(const Expr& expr, const EnvPtr& env) -> Value;

[[nodiscard]] auto apply(const Value& func, Values args) -> Value;

void interpret(const Toplevel& toplevel);

#endif // EASYEASYLISP_HPP
