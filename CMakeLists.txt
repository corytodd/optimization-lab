cmake_minimum_required(VERSION 3.22.1)
project(optizaton-demo
    VERSION 1.0.0
    LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Benchmark CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(StrictBuildMode)
include(ClangTools)
include(BuildTypeSanitizer)
include(BuildTypeCoverage)
include(BuildTypeBenchmark)

enable_testing()

add_subdirectory(cmd)
add_subdirectory(src)
add_subdirectory(test)
