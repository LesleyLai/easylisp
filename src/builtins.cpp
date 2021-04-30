#include "environment.hpp"
#include "interpreter.hpp"
#include <cassert>
#include <numeric>

namespace {

void check_args_count(std::string_view proc_name, std::size_t actual,
                      std::size_t expected)
{
  if (actual != expected) {
    throw std::runtime_error{fmt::format(
        "Type error: arity mismatch for {}\n"
        "the expected number of arguments does not match the given number\n"
        "  expected: {}, given: {}",
        proc_name, expected, actual)};
  }
}

void check_args_count_greater_equal(std::string_view proc_name,
                                    std::size_t actual, std::size_t expected)
{
  if (actual < expected) {
    throw std::runtime_error{fmt::format(
        "Type error: arity mismatch for {}\n"
        "the expected number of arguments does not match the given number\n"
        "  expected: at least {}, given: {}",
        proc_name, expected, actual)};
  }
}

template <typename Pred>
void check_arg(const Value& arg, Pred&& pred, std::string_view pred_name)
{
  if (!pred(arg))
    throw std::runtime_error{
        fmt::format("Type error: ({} {}) is false", pred_name, to_string(arg))};
}

void check_arg_is_boolean(const Value& arg)
{
  return check_arg(arg, is_boolean, "bool?");
}

void check_arg_is_number(const Value& arg)
{
  return check_arg(arg, is_number, "number?");
}

void check_arg_is_proc(const Value& arg)
{
  return check_arg(arg, is_procedural, "procedural?");
}

void check_arg_is_list(const Value& arg)
{
  return check_arg(arg, is_list, "list?");
}

void check_arg_is_pair(const Value& arg)
{
  return check_arg(arg, is_pair, "pair?");
}

auto to_vector(const Value& list) -> std::vector<Value>
{
  assert(is_list(list));
  std::vector<Value> values;
  {
    const auto* node_ptr = std::get<ObjectPtr>(list).get();
    while (node_ptr != nullptr) {
      const auto* cons_ptr = dynamic_cast<const Cons*>(node_ptr);
      values.push_back(cons_ptr->car);
      node_ptr = std::get<ObjectPtr>(cons_ptr->cdr).get();
    }
  }
  return values;
}

auto to_lisp_list(const Values& args) -> Value
{
  Value list = nullptr;
  for (const auto& arg : std::views::reverse(args)) {
    list = std::make_shared<Cons>(arg, list, true);
  }
  return list;
}

template <typename BinaryOp> auto builtin_arith_proc(std::string name)
{
  return std::make_shared<BuiltinProc>(name, [name](Values args) {
    check_args_count_greater_equal(name, args.size(), 1);
    check_arg_is_number(args.front());
    auto number = std::get<double>(args.front());
    for (auto i = args.begin() + 1; i != args.end(); ++i) {
      check_arg_is_number(*i);
      number = BinaryOp{}(number, std::get<double>(*i));
    }
    return number;
  });
}

auto builtin_eq() -> Value
{
  return std::make_shared<BuiltinProc>("eq?", [](Values args) -> Value {
    check_args_count("eq?", args.size(), 2);
    return args[0] == args[1];
  });
}

[[nodiscard]] auto lisp_equal(const Value& lhs, const Value& rhs) -> bool
{
  return std::visit(
      [](auto&& lhs_, auto&& rhs_) {
        using T1 = std::remove_cvref_t<decltype(lhs_)>;
        using T2 = std::remove_cvref_t<decltype(rhs_)>;
        if constexpr (!std::is_same_v<T1, T2>) {
          return false;
        } else if constexpr (!std::is_same_v<T1, ObjectPtr>) {
          return lhs_ == rhs_;
        } else {
          if (lhs_ == nullptr && rhs_ == nullptr) return true;
          if (lhs_ == nullptr || rhs_ == nullptr) return false;
          const auto* lhs_cons_ptr = dynamic_cast<const Cons*>(lhs_.get());
          const auto* rhs_cons_ptr = dynamic_cast<const Cons*>(rhs_.get());
          if (!lhs_cons_ptr || !rhs_cons_ptr) return lhs_ == rhs_;

          return lisp_equal(lhs_cons_ptr->car, rhs_cons_ptr->car) &&
                 lisp_equal(lhs_cons_ptr->cdr, rhs_cons_ptr->cdr);
        }
      },
      lhs, rhs);
}

auto builtin_equal() -> Value
{
  return std::make_shared<BuiltinProc>("equal?", [](Values args) -> Value {
    check_args_count("equal?", args.size(), 2);
    return lisp_equal(args[0], args[1]);
  });
}

template <typename ArgT, typename CheckArg, typename BinaryOp>
auto builtin_binary_proc(CheckArg check_arg, const std::string& name,
                         BinaryOp op)
{
  return std::make_shared<BuiltinProc>(name, [=](Values args) {
    check_args_count(name, args.size(), 2);
    check_arg(args[0]);
    check_arg(args[1]);
    return op(std::get<ArgT>(args[0]), std::get<ArgT>(args[1]));
  });
}

template <typename BinaryOp>
auto builtin_compare_proc(const std::string& name, BinaryOp op)
{
  return builtin_binary_proc<double>(check_arg_is_number, name, op);
}

auto builtin_not() -> Value
{
  return std::make_shared<BuiltinProc>("not", [](Values args) -> Value {
    check_args_count("not", args.size(), 1);
    check_arg_is_boolean(args[0]);
    return !std::get<bool>(args[0]);
  });
}

template <typename BinaryOp>
auto builtin_logical_proc(const std::string& name, BinaryOp op)
{
  return builtin_binary_proc<bool>(check_arg_is_boolean, name, op);
}

auto builtin_cons() -> Value
{
  return std::make_shared<BuiltinProc>("cons", [](Values args) -> Value {
    check_args_count("cons", args.size(), 2);
    return std::make_shared<Cons>(args[0], args[1], is_list(args[1]));
  });
}

auto as_cons(const Value& v) -> const Cons&
{
  return dynamic_cast<const Cons&>(*std::get<ObjectPtr>(v));
}

auto builtin_car() -> Value
{
  return std::make_shared<BuiltinProc>("car", [](Values args) -> Value {
    check_args_count("car", args.size(), 1);
    check_arg_is_pair(args[0]);
    return as_cons(args[0]).car;
  });
}

auto builtin_cdr() -> Value
{
  return std::make_shared<BuiltinProc>("cdr", [](Values args) -> Value {
    check_args_count("cdr", args.size(), 1);
    check_arg_is_pair(args[0]);
    return as_cons(args[0]).cdr;
  });
}

auto builtin_list() -> Value
{
  return std::make_shared<BuiltinProc>(
      "list", [](Values args) { return to_lisp_list(args); });
}

auto builtin_range() -> Value
{
  return std::make_shared<BuiltinProc>("range", [](Values args) -> Value {
    check_args_count("range", args.size(), 2);
    check_arg_is_number(args[0]);
    check_arg_is_number(args[1]);

    const int lower = static_cast<int>(std::get<double>(args[0]));
    const int upper = static_cast<int>(std::get<double>(args[1]));
    if (upper < lower) return nullptr;

    Value list = nullptr;
    for (int i = upper - 1; i >= lower; --i) {
      list = std::make_shared<Cons>(static_cast<double>(i), list, true);
    }
    return list;
  });
}

template <typename Pred>
auto builtin_pred(std::string name, Pred&& pred) -> Value
{
  return std::make_shared<BuiltinProc>(
      name, [name, pred = FWD(pred)](Values args) -> Value {
        check_args_count(name, args.size(), 1);
        return pred(args[0]);
      });
}

auto builtin_map() -> Value
{
  return std::make_shared<BuiltinProc>("map", [](Values args) -> Value {
    check_args_count("map", args.size(), 2);
    check_arg_is_proc(args[0]);
    check_arg_is_list(args[1]);

    std::vector<Value> values = to_vector(args[1]);
    for (auto& value : values) { value = ::apply(args[0], std::vector{value}); }
    return to_lisp_list(values);
  });
}

auto builtin_filter() -> Value
{
  return std::make_shared<BuiltinProc>("filter", [](Values args) -> Value {
    check_args_count("filter", args.size(), 2);
    check_arg_is_proc(args[0]);
    check_arg_is_list(args[1]);

    std::vector<Value> values = to_vector(args[1]);
    std::vector<Value> results;
    results.reserve(values.size());
    std::ranges::copy_if(
        values, std::back_inserter(results), [&](const Value& value) {
          const auto result = ::apply(args[0], std::vector{value});
          const bool* satisfied = std::get_if<bool>(&result);
          return !(satisfied != nullptr && !*satisfied);
        });
    return to_lisp_list(results);
  });
}

auto builtin_foldl() -> Value
{
  return std::make_shared<BuiltinProc>("foldl", [](Values args) -> Value {
    check_args_count("foldl", args.size(), 3);
    check_arg_is_proc(args[0]);
    check_arg_is_list(args[2]);

    Value acc = args[1];
    const auto* node_ptr = std::get<ObjectPtr>(args[2]).get();
    while (node_ptr != nullptr) {
      const auto* cons_ptr = dynamic_cast<const Cons*>(node_ptr);
      acc = ::apply(args[0], std::vector{cons_ptr->car, acc});
      node_ptr = std::get<ObjectPtr>(cons_ptr->cdr).get();
    }
    return acc;
  });
}

auto builtin_foldr() -> Value
{
  return std::make_shared<BuiltinProc>("foldr", [](Values args) -> Value {
    check_args_count("foldr", args.size(), 3);
    check_arg_is_proc(args[0]);
    check_arg_is_list(args[2]);

    std::vector<Value> values = to_vector(args[2]);
    return std::accumulate(values.rbegin(), values.rend(), args[1],
                           [&](const Value& acc, const Value& elem) {
                             return ::apply(args[0], std::vector{elem, acc});
                           });
  });
}

} // anonymous namespace

Environment::Environment(Environment::create_global_t)
{
  bindings_.emplace("boolean?", builtin_pred("boolean?", is_boolean));
  bindings_.emplace("true", true);
  bindings_.emplace("false", false);

  bindings_.emplace("number?", builtin_pred("number?", is_number));
  bindings_.emplace("+", builtin_arith_proc<std::plus<>>("+"));
  bindings_.emplace("-", builtin_arith_proc<std::minus<>>("-"));
  bindings_.emplace("*", builtin_arith_proc<std::multiplies<>>("*"));
  bindings_.emplace("/", builtin_arith_proc<std::divides<>>("/"));

  bindings_.emplace("eq?", builtin_eq());
  bindings_.emplace("equal?", builtin_equal());
  bindings_.emplace("<", builtin_compare_proc("<", std::less<>{}));
  bindings_.emplace("<=", builtin_compare_proc("<=", std::less_equal<>{}));
  bindings_.emplace(">", builtin_compare_proc(">", std::greater<>{}));
  bindings_.emplace(">=", builtin_compare_proc(">=", std::greater_equal<>{}));

  bindings_.emplace("not", builtin_not());
  bindings_.emplace("and", builtin_logical_proc("and", std::logical_and<>{}));
  bindings_.emplace("or", builtin_logical_proc("or", std::logical_or<>{}));

  bindings_.emplace("null?", builtin_pred("null?", is_null));
  bindings_.emplace("null", nullptr);
  bindings_.emplace("pair?", builtin_pred("pair?", is_pair));
  bindings_.emplace("list?", builtin_pred("list?", is_list));
  bindings_.emplace("cons", builtin_cons());
  bindings_.emplace("car", builtin_car());
  bindings_.emplace("cdr", builtin_cdr());
  bindings_.emplace("list", builtin_list());
  bindings_.emplace("range", builtin_range());
  bindings_.emplace("map", builtin_map());
  bindings_.emplace("filter", builtin_filter());
  bindings_.emplace("foldl", builtin_foldl());
  bindings_.emplace("foldr", builtin_foldr());

  bindings_.emplace("procedural?", builtin_pred("procedural?", is_procedural));
}