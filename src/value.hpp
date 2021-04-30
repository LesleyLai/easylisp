#ifndef EASYLISP_VALUE_HPP
#define EASYLISP_VALUE_HPP

#include "ast.hpp"
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <fmt/format.h>

class Environment;
using EnvPtr = std::shared_ptr<const Environment>;

struct BuiltinProc;
struct Proc;
struct Cons;

struct ObjectVisitor {
  ObjectVisitor() = default;
  virtual ~ObjectVisitor() = default;
  ObjectVisitor(const ObjectVisitor&) = delete;
  auto operator=(const ObjectVisitor&) & -> ObjectVisitor& = delete;
  ObjectVisitor(ObjectVisitor&&) noexcept = default;
  auto operator=(ObjectVisitor&&) & noexcept -> ObjectVisitor& = default;

  virtual void visit(const BuiltinProc&) = 0;
  virtual void visit(const Proc&) = 0;
  virtual void visit(const Cons&) = 0;
};

struct Object {
  Object() = default;
  virtual ~Object() = default;
  Object(const Object&) = delete;
  auto operator=(const Object&) & -> Object& = delete;
  Object(Object&&) noexcept = default;
  auto operator=(Object&&) & noexcept -> Object& = default;

  virtual void accept(ObjectVisitor& visitor) const = 0;
  [[nodiscard]] virtual auto is_pair() const -> bool { return false; }
  [[nodiscard]] virtual auto is_list() const -> bool { return false; }
  [[nodiscard]] virtual auto is_procedural() const -> bool { return false; }
};
using ObjectPtr = std::shared_ptr<Object>;

/**
 * @brief A polymorphic value type for our language
 */
using Value = std::variant<double, bool, ObjectPtr>;

/**
 * @brief A list of values
 */
using Values = std::span<const Value>;

#define OBJECT_ACCEPT                                                          \
  void accept(ObjectVisitor& visitor) const override { visitor.visit(*this); }

/**
 * @brief BuiltinProc wraps a native function that can be invoked from our
 * language
 */
struct BuiltinProc : Object {
  using NativeFunc = std::function<Value(Values)>;
  std::string name;
  NativeFunc native_func;

  BuiltinProc(std::string name_, NativeFunc native_func_)
      : name{std::move(name_)}, native_func{std::move(native_func_)}
  {}

  [[nodiscard]] auto is_procedural() const -> bool override { return true; }

  OBJECT_ACCEPT
};

/**
 * @brief a lisp procedural
 */
struct Proc : Object {
  std::vector<std::string> parameters;
  ExprPtr body;
  EnvPtr env;

  Proc(std::vector<std::string> parameters_, ExprPtr body_, EnvPtr env_)
      : parameters(std::move(parameters_)), //
        body(std::move(body_)),             //
        env(std::move(env_))
  {}

  [[nodiscard]] auto is_procedural() const -> bool override { return true; }

  OBJECT_ACCEPT
};

/**
 * @brief A pair
 */
struct Cons : Object {
  Value car;
  Value cdr;
  bool is_list_;

  Cons(Value car_, Value cdr_, bool is_list)
      : car(std::move(car_)), cdr(std::move(cdr_)), is_list_{is_list}
  {}

  [[nodiscard]] auto is_pair() const -> bool override { return true; }
  [[nodiscard]] auto is_list() const -> bool override { return is_list_; }

  OBJECT_ACCEPT
};

[[nodiscard]] auto to_string(const Value& value) -> std::string;

[[nodiscard]] auto is_number(const Value& value) -> bool;
[[nodiscard]] auto is_boolean(const Value& value) -> bool;
[[nodiscard]] auto is_null(const Value& value) -> bool;
[[nodiscard]] auto is_pair(const Value& value) -> bool;
[[nodiscard]] auto is_list(const Value& value) -> bool;
[[nodiscard]] auto is_procedural(const Value& value) -> bool;

#endif // EASYLISP_VALUE_HPP
