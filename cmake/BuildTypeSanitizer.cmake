# SPDX-License-Identifier: MIT
# Add a custom build type that enables sanitizer features from your compiler

if(PROJECT_IS_TOP_LEVEL)
    if(NOT "Sanitizer" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Sanitizer)
    endif()

    # Flags that must appear on both compile and link lines
    set(SANITIZER_LD_FLAGS
        -fsanitize=address
        -fsanitize=undefined
        -fsanitize=leak
    )

    # Compile-only flags that improve sanitizer accuracy
    set(SANITIZER_COMPILE_FLAGS
        ${SANITIZER_LD_FLAGS}
        -fno-common
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
        -O1
        -g
    )

    list(JOIN SANITIZER_COMPILE_FLAGS " " SANITIZER_COMPILE_FLAGS_STR)
    list(JOIN SANITIZER_LD_FLAGS " " SANITIZER_LD_FLAGS_STR)

    set(CMAKE_C_FLAGS_SANITIZER
        "${CMAKE_C_FLAGS_DEBUG} ${SANITIZER_COMPILE_FLAGS_STR}"
        CACHE STRING "C compiler flags for Sanitizer build type" FORCE
    )
    set(CMAKE_EXE_LINKER_FLAGS_SANITIZER
        "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${SANITIZER_LD_FLAGS_STR}"
        CACHE STRING "Executable linker flags for Sanitizer build type" FORCE
    )
    set(CMAKE_SHARED_LINKER_FLAGS_SANITIZER
        "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${SANITIZER_LD_FLAGS_STR}"
        CACHE STRING "Shared library linker flags for Sanitizer build type" FORCE
    )
    set(CMAKE_STATIC_LINKER_FLAGS_SANITIZER
        "${CMAKE_STATIC_LINKER_FLAGS_DEBUG}"
        CACHE STRING "Static library linker flags for Sanitizer build type" FORCE
    )
    set(CMAKE_MODULE_LINKER_FLAGS_SANITIZER
        "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} ${SANITIZER_LD_FLAGS_STR}"
        CACHE STRING "Module linker flags for Sanitizer build type" FORCE
    )

    set_property(GLOBAL APPEND PROPERTY DEBUG_CONFIGURATIONS Sanitizer)
endif()
