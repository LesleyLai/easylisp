#ifndef EASYLISP_PARSER_HPP
#define EASYLISP_PARSER_HPP

#include "ast.hpp"

[[nodiscard]] auto parse_toplevel(std::string_view source) -> Toplevel;

#endif // EASYLISP_PARSER_HPP
