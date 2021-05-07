#ifndef EASYLISP_FILE_UTIL_HPP
#define EASYLISP_FILE_UTIL_HPP

#include <fstream>
#include <string>

[[nodiscard]] auto file_to_string(const std::ifstream& file) -> std::string;

#endif // EASYLISP_FILE_UTIL_HPP
