add_library(common
        token.hpp
        scanner.hpp
        scanner.cpp
        ast.cpp
        ast.hpp
        parser.cpp
        parser.hpp
        value.cpp
        value.hpp
        interpreter.cpp
        interpreter.hpp
        environment.cpp
        environment.hpp
        config.hpp builtins.cpp)
target_link_libraries(common PRIVATE compiler_options CONAN_PKG::fast_float PUBLIC CONAN_PKG::fmt)
target_include_directories(common PUBLIC "${CMAKE_SOURCE_DIR}/src")

add_executable(easylisp main.cpp)
target_link_libraries(easylisp
        PRIVATE common compiler_options)
