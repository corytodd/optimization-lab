# SPDX-License-Identifier: MIT
# Add a custom build type for benchmarking: optimized with debug info for profiling

if(PROJECT_IS_TOP_LEVEL)
    if(NOT "Benchmark" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Benchmark)
    endif()

    set(BENCHMARK_COMPILE_FLAGS
        -O3
        -DNDEBUG
        -g
        -fno-omit-frame-pointer
    )

    list(JOIN BENCHMARK_COMPILE_FLAGS " " BENCHMARK_COMPILE_FLAGS_STR)

    set(CMAKE_C_FLAGS_BENCHMARK
        "${CMAKE_C_FLAGS_RELEASE} ${BENCHMARK_COMPILE_FLAGS_STR}"
        CACHE STRING "C compiler flags for Benchmark build type" FORCE
    )
    set(CMAKE_EXE_LINKER_FLAGS_BENCHMARK
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE}"
        CACHE STRING "Executable linker flags for Benchmark build type" FORCE
    )
    set(CMAKE_SHARED_LINKER_FLAGS_BENCHMARK
        "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}"
        CACHE STRING "Shared library linker flags for Benchmark build type" FORCE
    )
    set(CMAKE_STATIC_LINKER_FLAGS_BENCHMARK
        "${CMAKE_STATIC_LINKER_FLAGS_RELEASE}"
        CACHE STRING "Static library linker flags for Benchmark build type" FORCE
    )
    set(CMAKE_MODULE_LINKER_FLAGS_BENCHMARK
        "${CMAKE_MODULE_LINKER_FLAGS_RELEASE}"
        CACHE STRING "Module linker flags for Benchmark build type" FORCE
    )
endif()
