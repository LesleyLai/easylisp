#ifndef EASYLISP_AST_HPP
#define EASYLISP_AST_HPP

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "config.hpp"

struct NumberExpr;
struct ApplyExpr;
struct VariableExpr;
struct LambdaExpr;
struct LetExpr;
struct BooleanExpr;
struct IfExpr;

/**
 * @brief Visitor interface for expression
 */
struct ExprVisitor {
  ExprVisitor() = default;
  virtual ~ExprVisitor() = default;
  ExprVisitor(const ExprVisitor&) = delete;
  auto operator=(const ExprVisitor&) & -> ExprVisitor& = delete;
  ExprVisitor(ExprVisitor&&) noexcept = default;
  auto operator=(ExprVisitor&&) & noexcept -> ExprVisitor& = default;

  virtual void visit(const NumberExpr&) = 0;
  virtual void visit(const ApplyExpr&) = 0;
  virtual void visit(const VariableExpr&) = 0;
  virtual void visit(const LambdaExpr&) = 0;
  virtual void visit(const LetExpr&) = 0;
  virtual void visit(const BooleanExpr&) = 0;
  virtual void visit(const IfExpr&) = 0;
};

/**
 * @brief Interface for expression
 */
struct Expr {
  Expr() = default;
  virtual ~Expr() = default;
  Expr(const Expr&) = delete;
  auto operator=(const Expr&) & -> Expr& = delete;
  Expr(Expr&&) noexcept = default;
  auto operator=(Expr&&) & noexcept -> Expr& = default;

  /**
   * @brief Invokes a expression visitor
   */
  virtual void accept(ExprVisitor& visitor) const = 0;
};

/**
 * @brief A type alias for pointers to expression
 */
using ExprPtr = std::shared_ptr<Expr>;

/**
 * @brief A definition define a variable as an expression
 */
struct Definition {
  std::string var;
  ExprPtr expr;
};

/**
 * @brief A Toplevel is either an expression or a definition
 */
using Toplevel = std::variant<ExprPtr, Definition>;

#define EXPR_ACCEPT                                                            \
  void accept(ExprVisitor& visitor) const override                             \
  {                                                                            \
    visitor.visit(*this);                                                      \
  }

/**
 * @brief A constant expression that contains a number
 */
struct NumberExpr : Expr {
  double value;

  explicit NumberExpr(double v_) : value{v_} {}

  EXPR_ACCEPT
};

/**
 * @brief A constant expression that contains a boolean value
 */
struct BooleanExpr : Expr {
  bool value;

  explicit BooleanExpr(bool v_) : value{v_} {}

  EXPR_ACCEPT
};

/**
 * @brief An expression that contains a variable
 */
struct VariableExpr : Expr {
  std::string id;

  explicit VariableExpr(std::string id_) : id{MOV(id_)} {}

  EXPR_ACCEPT
};

/**
 * @brief An expression that represents a procedural application
 */
struct ApplyExpr : Expr {
  ExprPtr func;
  std::vector<ExprPtr> arguments;

  explicit ApplyExpr(ExprPtr func_, std::vector<ExprPtr> args_)
      : func{MOV(func_)}, arguments(MOV(args_))
  {
  }

  EXPR_ACCEPT
};

/**
 * @brief An expression that represents a lambda expression
 */
struct LambdaExpr : Expr {
  std::vector<std::string> parameters;
  ExprPtr body;

  explicit LambdaExpr(std::vector<std::string> parameters_, ExprPtr body_)
      : parameters{(MOV(parameters_))}, body(MOV(body_))
  {
  }

  EXPR_ACCEPT
};

/**
 * A binding binds a variable to an expression
 */
struct Binding {
  std::string variable;
  ExprPtr expr;
};

/**
 * @brief An expression that represents a local variable binding
 */
struct LetExpr : Expr {
  std::vector<Binding> bindings;
  ExprPtr body;

  explicit LetExpr(std::vector<Binding> bindings_, ExprPtr body_)
      : bindings{(MOV(bindings_))}, body(MOV(body_))
  {
  }

  EXPR_ACCEPT
};

/**
 * @brief An expression that represents an if expression
 */
struct IfExpr : Expr {
  ExprPtr cond_expr;
  ExprPtr if_expr;
  ExprPtr else_expr;

  explicit IfExpr(ExprPtr cond_expr_, ExprPtr if_expr_, ExprPtr else_expr_)
      : cond_expr{MOV(cond_expr_)}, //
        if_expr{MOV(if_expr_)},     //
        else_expr{MOV(else_expr_)}
  {
  }

  EXPR_ACCEPT
};

#undef EXPR_ACCEPT

/// @brief Converts a Toplevel to string
[[nodiscard]] auto to_string(const Toplevel& toplevel) -> std::string;

/// @brief Converts an expression to string
[[nodiscard]] auto to_string(const Expr& expr) -> std::string;

#endif // EASYLISP_AST_HPP
