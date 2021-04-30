#include "interpreter.hpp"
#include "environment.hpp"
#include "value.hpp"

#include <fmt/core.h>
#include <stdexcept>

auto eval_args(const std::vector<ExprPtr>& arg_exprs, const EnvPtr& env)
    -> std::vector<Value>
{
  std::vector<Value> args;
  args.reserve(arg_exprs.size());
  for (const ExprPtr& arg : arg_exprs) { args.push_back(eval(*arg, env)); }
  return args;
}

[[nodiscard]] auto apply_expr(const ApplyExpr& expr, const EnvPtr& env) -> Value
{
  return ::apply(eval(*expr.func, env), eval_args(expr.arguments, env));
}

struct ObjectApplier : ObjectVisitor {
  Value result;
  Values args;

  explicit ObjectApplier(Values args_) : args{args_} {}

  void visit(const BuiltinProc& proc) override
  {
    result = proc.native_func(args);
  }

  void visit(const Proc& proc) override
  {
    if (proc.parameters.size() != args.size()) {
      throw std::runtime_error(fmt::format(
          "Type error: arity mismatch\n"
          "the expected number of arguments does not match the given number\n"
          "  expected: {}, given: {}",
          args.size(), proc.parameters.size()));
    }

    auto apply_env = std::make_shared<Environment>(proc.env);
    for (std::size_t i = 0; i < proc.parameters.size(); ++i) {
      apply_env->add(proc.parameters[i], args[i]);
    }
    result = eval(*proc.body, apply_env);
  }

  [[noreturn]] static void apply_to_cons()
  {
    throw std::runtime_error{"Type error: cannot apply to cons cells"};
  }

  void visit(const Cons&) override
  {
    apply_to_cons();
  }
};

[[nodiscard]] auto apply(const Value& func, Values args) -> Value
{
  return std::visit(
      [&](auto&& f) -> Value {
        using T = std::remove_cvref_t<decltype(f)>;
        if constexpr (std::is_same_v<T, ObjectPtr>) {
          ObjectApplier visitor{args};
          f->accept(visitor);
          return visitor.result;
        }

        throw std::runtime_error{
            fmt::format("Type error: cannot apply to {}!", to_string(func))};
      },
      func);
}

struct Evaluator : ExprVisitor {
  Value result;
  const EnvPtr& env;

  explicit Evaluator(const EnvPtr& env_) : env{env_} {}

  void visit(const NumberExpr& number) override
  {
    result = number.value;
  }

  void visit(const ApplyExpr& expr) override
  {
    result = apply_expr(expr, env);
  }

  void visit(const VariableExpr& expr) override
  {
    if (auto* val = env->find(expr.id); val) {
      result = *val;
      return;
    }

    throw std::runtime_error(
        fmt::format("ReferenceError: {} is not defined", expr.id));
  }

  void visit(const LambdaExpr& expr) override
  {
    result = std::make_shared<Proc>(expr.parameters, expr.body, env);
  }

  void visit(const LetExpr& expr) override
  {
    std::vector<Value> binding_vals;
    for (const Binding& binding : expr.bindings) {
      binding_vals.push_back(eval(*binding.expr, env));
    }
    auto body_env = std::make_shared<Environment>(env);
    for (std::size_t i = 0; i < binding_vals.size(); ++i) {
      body_env->add(expr.bindings[i].variable, binding_vals[i]);
    }
    result = eval(*expr.body, body_env);
  }

  void visit(const BooleanExpr& expr) override
  {
    result = expr.value;
  }

  void visit(const IfExpr& expr) override
  {
    auto cond_val = eval(*expr.cond_expr, env);
    const bool* cond = std::get_if<bool>(&cond_val);
    if (!cond) {
      throw std::runtime_error{
          fmt::format("Type error: {} is not a boolean. The condition of an if "
                      "expression must be a boolean.",
                      to_string(cond_val))};
    }

    auto& branch = *cond ? *expr.if_expr : *expr.else_expr;
    result = eval(branch, env);
  }
};

auto eval(const Expr& expr, const EnvPtr& env) -> Value
{
  Evaluator evaluator{env};
  expr.accept(evaluator);
  return evaluator.result;
}

struct Interpreter {
  void operator()(const ExprPtr& expr)
  {
    const auto val = eval(*expr, Environment::global());
    fmt::print("{}\n", to_string(val));
  }

  void operator()(const Definition& definition)
  {
    Environment::global()->add(definition.var,
                               eval(*definition.expr, Environment::global()));
  }
};

void interpret(const Toplevel& toplevel)
{
  std::visit(Interpreter{}, toplevel);
}