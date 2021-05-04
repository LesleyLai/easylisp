#ifndef EASYLISP_PARSER_HPP
#define EASYLISP_PARSER_HPP

#include "ast.hpp"

[[nodiscard]] auto parse(std::string_view source) -> Program;

#endif // EASYLISP_PARSER_HPP
