macro(run_conan)
    if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake")
    endif ()

    include(${CMAKE_BINARY_DIR}/conan.cmake)

    conan_add_remote(NAME bincrafters URL
            https://api.bintray.com/conan/bincrafters/public-conan)

    conan_cmake_run(REQUIRES
            fmt/7.1.3
            fast_float/1.0.0
            catch2/2.13.4
            approvaltests.cpp/10.8.0
            BASIC_SETUP CMAKE_TARGETS
            BUILD missing)
endmacro()