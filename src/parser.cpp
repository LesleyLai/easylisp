#include "parser.hpp"
#include "scanner.hpp"

#include <algorithm>
#include <stdexcept>

#include <fmt/core.h>

namespace {

class Parser {
  Scanner itr_;

public:
  explicit Parser(std::string_view source) : itr_{source} {}

  auto is_at_end() -> bool { return itr_->type == TokenType::eof; }

  [[nodiscard]] auto parse_toplevel() -> Toplevel
  {
    if (match(TokenType::left_paren)) {
      auto look_ahead_itr = itr_;
      ++look_ahead_itr;

      if (match_itr(TokenType::keyword_define, look_ahead_itr)) {
        itr_ = look_ahead_itr;
        ++itr_;
        return parse_definition();
      }
    }

    return parse_expr();
  }

private:
  auto match(TokenType type) -> bool
  {
    if (is_at_end()) { return false; }

    return itr_->type == type;
  }

  auto match_itr(TokenType type, Scanner itr) -> bool
  {
    if (is_at_end()) { return false; }
    return itr->type == type;
  }

  auto parse_definition() -> Definition
  {
    auto binding = parse_binding();
    return Definition{MOV(binding.variable), MOV(binding.expr)};
  }

  auto parse_expr() -> ExprPtr
  {
    switch (itr_->type) {
    case TokenType::number: {
      auto expr = std::make_shared<NumberExpr>(itr_->data.number);
      ++itr_;
      return expr;
    }
    case TokenType::identifier: {
      auto expr = std::make_shared<VariableExpr>(std::string{itr_->lexeme});
      ++itr_;
      return expr;
    }
    case TokenType::left_paren: {
      ++itr_;
      return parse_parenthesis();
    }
    case TokenType::eof:
      throw std::runtime_error{
          "Syntax error: unexpected end of file when parsing expression"};
    default:
      throw std::runtime_error{fmt::format(
          "Syntax error: unexpected token {} when parsing expression",
          itr_->lexeme)};
    }
  }

  auto parse_parenthesis() -> ExprPtr
  {
    switch (itr_->type) {
    case TokenType::keyword_lambda:
      ++itr_;
      return parse_lambda();
    case TokenType::keyword_let:
      ++itr_;
      return parse_let();
    case TokenType::keyword_if:
      ++itr_;
      return parse_if();
    default:
      return parse_apply();
    }
  }

  auto parse_if() -> ExprPtr
  {
    auto cond_expr = parse_expr();
    auto if_expr = parse_expr();
    auto else_expr = parse_expr();
    consume_right_param();
    return std::make_shared<IfExpr>(MOV(cond_expr), MOV(if_expr),
                                    MOV(else_expr));
  }

  auto parse_apply() -> ExprPtr
  {
    auto func = parse_expr();
    std::vector<ExprPtr> args;
    while (!is_at_end() && itr_->type != TokenType::right_paren) {
      args.push_back(parse_expr());
    }

    consume_right_param();

    return std::make_shared<ApplyExpr>(MOV(func), MOV(args));
  }

  auto parse_lambda() -> ExprPtr
  {
    std::vector<std::string> parameters;
    consume_one(TokenType::left_paren, "Syntax error: expect parameter list");

    while (!is_at_end() && itr_->type != TokenType::right_paren) {
      if (match(TokenType::identifier)) {
        parameters.emplace_back(itr_->lexeme);
        ++itr_;
      } else {
        throw std::runtime_error(
            "Syntax error: parameter needs to be a valid identifier");
      }
    }
    consume_right_param();

    auto body = parse_expr();
    consume_right_param();
    return std::make_shared<LambdaExpr>(parameters, MOV(body));
  }

  auto parse_let() -> ExprPtr
  {
    std::vector<Binding> bindings;
    consume_one(TokenType::left_paren, "Syntax error: expect variable list");

    while (!is_at_end() && itr_->type != TokenType::right_paren) {
      consume_one(TokenType::left_paren, "Syntax error: expect binding");
      bindings.push_back(parse_binding());
    }
    consume_right_param();

    auto body = parse_expr();
    consume_right_param();
    return std::make_shared<LetExpr>(MOV(bindings), MOV(body));
  }

  auto parse_binding() -> Binding
  {
    if (itr_->type != TokenType::identifier) {
      throw std::runtime_error("Syntax error: expect variable name");
    }
    std::string variable{itr_->lexeme};
    ++itr_;
    ExprPtr expr = parse_expr();
    consume_right_param();
    return Binding{variable, MOV(expr)};
  }

  void consume_one(TokenType token_type, const char* error_message)
  {
    if (!match(token_type)) { throw std::runtime_error{error_message}; }
    ++itr_;
  }

  void consume_right_param()
  {
    consume_one(TokenType::right_paren, "Syntax error: expect )");
  }
};

} // anonymous namespace

[[nodiscard]] auto parse(std::string_view source) -> Program
{
  Parser parser{source};
  Program program;
  while (!parser.is_at_end()) { program.push_back(parser.parse_toplevel()); }
  return program;
}
