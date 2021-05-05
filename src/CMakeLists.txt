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
target_include_directories(common PUBLIC "${PROJECT_SOURCE_DIR}/src")

add_executable(easylisp main.cpp)
target_link_libraries(easylisp
        PRIVATE common compiler_options)

add_custom_target(scripts
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/scripts ${PROJECT_BINARY_DIR}/bin
        )
add_dependencies(easylisp scripts)
