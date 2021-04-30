#include "value.hpp"

#include <fmt/format.h>

template <> struct fmt::formatter<Value> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

  template <typename FormatContext>
  auto format(const Value& v, FormatContext& ctx)
  {
    return format_to(ctx.out(), "{}", to_string(v));
  }
};

struct ValuePrinter {
  auto operator()(bool b) -> std::string { return fmt::format("{}", b); }

  auto operator()(double number) -> std::string
  {
    return fmt::format("{}", number);
  }

  auto operator()(const ObjectPtr& obj) -> std::string
  {
    if (obj == nullptr) { return "()"; }

    struct ObjectPrinter : ObjectVisitor {
      std::string result;

      void visit(const BuiltinProc& proc) override
      {
        result = fmt::format("<builtin proc {}>", proc.name);
      }

      void visit(const Proc& proc) override
      {
        result = fmt::format("<proc ({})>", fmt::join(proc.parameters, " "));
      }

      void visit(const Cons& cons) override
      {
        if (!cons.is_list_) {
          result = fmt::format("({} . {})", cons.car, cons.cdr);
          return;
        }

        std::vector<std::string> elems;
        const Cons* ptr = &cons;
        while (ptr != nullptr) {
          elems.push_back(fmt::format("{}", ptr->car));
          ptr = dynamic_cast<const Cons*>(std::get<ObjectPtr>(ptr->cdr).get());
        }

        result = fmt::format("({})", fmt::join(elems, " "));
      }

    } visitor;

    obj->accept(visitor);
    return visitor.result;
  }
};

auto to_string(const Value& value) -> std::string
{
  return std::visit(ValuePrinter{}, value);
}

auto is_number(const Value& value) -> bool
{
  return std::holds_alternative<double>(value);
}

auto is_boolean(const Value& value) -> bool
{
  return std::holds_alternative<bool>(value);
}

[[nodiscard]] static auto as_object(const Value& value) -> const ObjectPtr*
{
  return std::get_if<ObjectPtr>(&value);
}

auto is_null(const Value& value) -> bool
{
  const auto* obj_pptr = as_object(value);
  return obj_pptr && (*obj_pptr == nullptr);
}

auto is_pair(const Value& value) -> bool
{
  const auto* obj_pptr = as_object(value);
  return obj_pptr && *obj_pptr && (*obj_pptr)->is_pair();
}

auto is_list(const Value& value) -> bool
{
  const auto* obj_pptr = as_object(value);
  return obj_pptr && (!(*obj_pptr) || ((*obj_pptr)->is_list()));
}

auto is_procedural(const Value& value) -> bool
{
  const auto* obj_pptr = as_object(value);
  if (obj_pptr && *obj_pptr) { return (*obj_pptr)->is_procedural(); }
  return false;
}