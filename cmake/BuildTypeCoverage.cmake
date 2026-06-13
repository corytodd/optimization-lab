# SPDX-License-Identifier: MIT
# Add a custom build type that enables code coverage features from your compiler

if(PROJECT_IS_TOP_LEVEL)
    if(NOT "Coverage" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Coverage)
    endif()

    # --coverage on the linker line links libgcov
    set(COVERAGE_LD_FLAGS --coverage)

    set(COVERAGE_COMPILE_FLAGS
        -g
        -O0
        --coverage
        -fno-inline
    )

    list(JOIN COVERAGE_COMPILE_FLAGS " " COVERAGE_COMPILE_FLAGS_STR)
    list(JOIN COVERAGE_LD_FLAGS " " COVERAGE_LD_FLAGS_STR)

    set(CMAKE_C_FLAGS_COVERAGE
        "${CMAKE_C_FLAGS_DEBUG} ${COVERAGE_COMPILE_FLAGS_STR}"
        CACHE STRING "C compiler flags for Coverage build type" FORCE
    )
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
        "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${COVERAGE_LD_FLAGS_STR}"
        CACHE STRING "Executable linker flags for Coverage build type" FORCE
    )
    set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
        "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${COVERAGE_LD_FLAGS_STR}"
        CACHE STRING "Shared library linker flags for Coverage build type" FORCE
    )
    set(CMAKE_STATIC_LINKER_FLAGS_COVERAGE
        "${CMAKE_STATIC_LINKER_FLAGS_DEBUG}"
        CACHE STRING "Static library linker flags for Coverage build type" FORCE
    )
    set(CMAKE_MODULE_LINKER_FLAGS_COVERAGE
        "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} ${COVERAGE_LD_FLAGS_STR}"
        CACHE STRING "Module linker flags for Coverage build type" FORCE
    )

    set_property(GLOBAL APPEND PROPERTY DEBUG_CONFIGURATIONS Coverage)
endif()
