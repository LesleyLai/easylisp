#ifndef EASYLISP_TEST_AST_PRINTER_HPP
#define EASYLISP_TEST_AST_PRINTER_HPP

#include <fmt/format.h>

#include "ast.hpp"

[[nodiscard]] auto to_string(const Expr& expr) -> std::string;

template <> struct fmt::formatter<ExprPtr> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

  template <typename FormatContext>
  auto format(const ExprPtr& ptr, FormatContext& ctx)
  {
    assert(ptr != nullptr);
    return format_to(ctx.out(), "{}", to_string(*ptr));
  }
};

template <> struct fmt::formatter<Binding> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

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
    result =
        fmt::format("(app {} {})", expr.func, fmt::join(expr.arguments, " "));
  }

  void visit(const LambdaExpr& expr) override
  {
    result = fmt::format("(lambda ({}) {})", fmt::join(expr.parameters, " "),
                         expr.body);
  }

  void visit(const LetExpr& expr) override
  {
    result =
        fmt::format("(let ({}) {})", fmt::join(expr.bindings, " "), expr.body);
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

[[nodiscard]] inline auto to_string(const Expr& expr) -> std::string
{
  ExprPrinter printer;
  expr.accept(printer);
  return printer.result;
}

template <> struct fmt::formatter<Toplevel> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

  template <typename FormatContext>
  auto format(const Toplevel& toplevel, FormatContext& ctx)
  {
    return std::visit( //
        overloaded{[&](const ExprPtr& expr) {
                     return fmt::format_to(ctx.out(), "{}", expr);
                   },
                   [&](const Definition& definition) {
                     return fmt::format_to(ctx.out(), "(define {} {})",
                                           definition.var, definition.expr);
                   },
                   [&](const Require& require_clause) {
                     return fmt::format_to(ctx.out(), "(require {})",
                                           require_clause.module_name);
                   }},
        toplevel);
  }
};

#endif // EASYLISP_TEST_AST_PRINTER_HPP
