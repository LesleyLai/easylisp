#include "ast.hpp"

#include <cassert>
#include <fmt/format.h>

template <> struct fmt::formatter<ExprPtr> {
  constexpr auto parse(format_parse_context& ctx)
  {
    return ctx.end();
  }

  template <typename FormatContext>
  auto format(const ExprPtr& ptr, FormatContext& ctx)
  {
    assert(ptr != nullptr);
    return format_to(ctx.out(), "{}", to_string(*ptr));
  }
};

template <> struct fmt::formatter<Binding> {
  constexpr auto parse(format_parse_context& ctx)
  {
    return ctx.end();
  }

  template <typename FormatContext>
  auto format(const Binding& binding, FormatContext& ctx)
  {
    return format_to(ctx.out(), "({} {})", binding.variable,
                     to_string(*binding.expr));
  }
};

struct ExprPrinter : ExprVisitor {
  std::string result;

  void visit(const NumberExpr& expr) override
  {
    result = fmt::format("(const {})", expr.value);
  }

  void visit(const VariableExpr& expr) override
  {
    result = fmt::format("(var {})", expr.id);
  }

  void visit(const ApplyExpr& expr) override
  {
    result = fmt::format("(app {} {})", to_string(*expr.func),
                         fmt::join(expr.arguments, " "));
  }

  void visit(const LambdaExpr& expr) override
  {
    result = fmt::format("(lambda ({}) {})", fmt::join(expr.parameters, " "),
                         to_string(*expr.body));
  }

  void visit(const LetExpr& expr) override
  {
    result = fmt::format("(let ({}) {})", fmt::join(expr.bindings, " "),
                         to_string(*expr.body));
  }

  void visit(const BooleanExpr& expr) override
  {
    result = fmt::format("(const {})", expr.value);
  }

  void visit(const IfExpr& expr) override
  {
    result = fmt::format("(if {} {} {})", expr.cond_expr, expr.if_expr,
                         expr.else_expr);
  }
};

struct ToplevelPrinter {
  auto operator()(const ExprPtr& expr)
  {
    return fmt::format("{}", expr);
  }

  auto operator()(const Definition& definition)
  {
    return fmt::format("(define {} {})", definition.var, definition.expr);
  }
};

[[nodiscard]] auto to_string(const Expr& expr) -> std::string
{
  ExprPrinter printer;
  expr.accept(printer);
  return printer.result;
}

[[nodiscard]] auto to_string(const Toplevel& toplevel) -> std::string
{
  return std::visit(ToplevelPrinter{}, toplevel);
}