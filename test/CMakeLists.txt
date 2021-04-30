set(TEST_TARGET_NAME ${PROJECT_NAME}_test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${TEST_TARGET_NAME} main.cpp scanner_test.cpp ast_test.cpp parser_test.cpp interpreter_test.cpp env_test.cpp)

target_link_libraries(${TEST_TARGET_NAME} PRIVATE common compiler_options
        CONAN_PKG::catch2 CONAN_PKG::approvaltests.cpp)

enable_testing()

add_test(NAME ${TEST_TARGET_NAME} COMMAND "${CMAKE_BINARY_DIR}/bin/${TEST_TARGET_NAME}")
