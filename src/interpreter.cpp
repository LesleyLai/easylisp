#include "interpreter.hpp"
#include "environment.hpp"
#include "file_util.hpp"
#include "parser.hpp"
#include "value.hpp"

#include <fstream>
#include <stdexcept>

namespace {

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

} // anonymous namespace

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

  void visit(const Cons&) override { apply_to_cons(); }
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
        } else {
          throw std::runtime_error{
              fmt::format("Type error: Cannot apply to {}!", to_string(func))};
        }
      },
      func);
}

struct Evaluator : ExprVisitor {
  Value result;
  const EnvPtr& env;

  explicit Evaluator(const EnvPtr& env_) : env{env_} {}

  void visit(const NumberExpr& number) override { result = number.value; }

  void visit(const ApplyExpr& expr) override { result = apply_expr(expr, env); }

  void visit(const VariableExpr& expr) override
  {
    if (const auto* val = env->find(expr.id); val) {
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
    binding_vals.reserve(expr.bindings.size());
    std::ranges::transform(
        expr.bindings, std::back_inserter(binding_vals),
        [&](const Binding& binding) { return eval(*binding.expr, env); });
    auto body_env = std::make_shared<Environment>(env);
    for (std::size_t i = 0; i < binding_vals.size(); ++i) {
      body_env->add(expr.bindings[i].variable, binding_vals[i]);
    }
    result = eval(*expr.body, body_env);
  }

  void visit(const BooleanExpr& expr) override { result = expr.value; }

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

void Interpreter::add_definition(const Definition& definition)
{
  global_env_->add(definition.var, eval(*definition.expr, global_env_));
}

void Interpreter::require_module(const Require& require)
{
  std::ifstream file{fmt::format("{}.easylisp", require.module_name)};
  if (!file.is_open())
    throw std::runtime_error{fmt::format("Runtime error: Cannot open module {}",
                                         require.module_name)};
  interpret(parse(file_to_string(file)));
}

auto Interpreter::interpret_toplevel(const Toplevel& toplevel)
    -> std::optional<Value>
{
  return std::visit( //
      overloaded{[this](const ExprPtr& expr) {
                   return std::optional{eval(*expr, global_env_)};
                 },
                 [this](const Definition& definition) -> std::optional<Value> {
                   add_definition(definition);
                   return std::nullopt;
                 },
                 [this](const Require& require) -> std::optional<Value> {
                   require_module(require);
                   return std::nullopt;
                 }},
      toplevel);
}

void Interpreter::interpret(const Program& program)
{
  for (const auto& toplevel : program) { interpret_toplevel(toplevel); }
}