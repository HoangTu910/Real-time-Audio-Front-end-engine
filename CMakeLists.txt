cmake_minimum_required(VERSION 3.14)
project(RTAFE
    VERSION   0.1.0
    LANGUAGES C
    DESCRIPTION "Real-Time Audio Front-End engine for SoC"
)

option(RTAFE_BUILD_TESTS    "Build unit tests"                    ON)
option(RTAFE_BUILD_EXAMPLES "Build integration examples"          ON)
option(RTAFE_ENABLE_SIMD    "Enable platform SIMD optimisations"  OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CompilerWarnings.cmake)

add_subdirectory(src)

if(RTAFE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(RTAFE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
