# Compiler specific settings

if (compiler_included)
    return()
endif ()
set(compiler_included true)

# Link this 'library' to use the standard warnings
add_library(compiler_options INTERFACE)


option(EASYLISP_WARNING_AS_ERROR "Treats compiler warnings as errors" ON)
if (MSVC)
    target_compile_options(compiler_options INTERFACE /W4 "/permissive-")
    if (EASYLISP_WARNING_AS_ERROR)
        target_compile_options(compiler_options INTERFACE /WX)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(compiler_options
            INTERFACE -Wall
            -Wextra
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2)
    if (EASYLISP_WARNING_AS_ERROR)
        target_compile_options(compiler_options INTERFACE -Werror)
    endif ()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(compiler_options
                INTERFACE -Wmisleading-indentation
                -Wduplicated-cond
                -Wduplicated-branches
                -Wlogical-op
                -Wuseless-cast
                )
    endif ()
endif ()

option(EASYLISP_ENABLE_PCH "Enable Precompiled Headers" OFF)
if (EASYLISP_ENABLE_PCH)
    target_precompile_headers(compiler_options INTERFACE
            <algorithm>
            <array>
            <vector>
            <string>
            <utility>
            <functional>
            <memory>
            <memory_resource>
            <string_view>
            <cmath>
            <cstddef>
            <type_traits>
            )
endif ()

option(EASYLISP_USE_ASAN "Enable the Address Sanitizers" OFF)
if (EASYLISP_USE_ASAN)
    message("Enable Address Sanitizer")
    target_compile_options(compiler_options INTERFACE
            -fsanitize=address -fno-omit-frame-pointer)
    target_link_libraries(compiler_options INTERFACE
            -fsanitize=address)
endif ()

option(EASYLISP_USE_TSAN "Enable the Thread Sanitizers" OFF)
if (EASYLISP_USE_TSAN)
    message("Enable Thread Sanitizer")
    target_compile_options(compiler_options INTERFACE
            -fsanitize=thread)
    target_link_libraries(compiler_options INTERFACE
            -fsanitize=thread)
endif ()

option(EASYLISP_USE_MSAN "Enable the Memory Sanitizers" OFF)
if (EASYLISP_USE_MSAN)
    message("Enable Memory Sanitizer")
    target_compile_options(compiler_options INTERFACE
            -fsanitize=memory -fno-omit-frame-pointer)
    target_link_libraries(compiler_options INTERFACE
            -fsanitize=memory)
endif ()

option(EASYLISP_USE_UBSAN "Enable the Undefined Behavior Sanitizers" OFF)
if (EASYLISP_USE_UBSAN)
    message("Enable Undefined Behavior Sanitizer")
    target_compile_options(compiler_options INTERFACE
            -fsanitize=undefined)
    target_link_libraries(compiler_options INTERFACE
            -fsanitize=undefined)
endif ()