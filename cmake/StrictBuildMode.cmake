# SPDX-License-Identifier: MIT
# Apply strict warning flags to Debug and Release builds unless RELAXED_BUILD_MODE is enabled

if(PROJECT_IS_TOP_LEVEL)
    option(RELAXED_BUILD_MODE "Disable strict warning flags" OFF)

    if(NOT RELAXED_BUILD_MODE)
        set(STRICT_COMPILE_FLAGS "")
        list(APPEND STRICT_COMPILE_FLAGS
            -Wall -Wextra -Wpedantic
            -Werror
            -Wshadow
            -Wconversion
            -Wsign-conversion
            -Wmissing-declarations
            -Wmissing-prototypes
            -Wstrict-prototypes
            -Wundef
            -Wfloat-equal
            -Wcast-align
            -Wformat=2
            -Wformat-security
            -Winit-self
            -Wmissing-format-attribute
            -Wmissing-noreturn
            -Wredundant-decls
            -Wswitch-enum
            -fstack-protector-strong
        )

        set(STRICT_LINK_FLAGS -fstack-protector-strong)

        list(JOIN STRICT_COMPILE_FLAGS " " STRICT_COMPILE_FLAGS_STR)
        list(JOIN STRICT_LINK_FLAGS " " STRICT_LINK_FLAGS_STR)

        foreach(_build_type Debug Release)
            string(TOUPPER ${_build_type} _build_type_upper)

            set(CMAKE_C_FLAGS_${_build_type_upper}
                "${CMAKE_C_FLAGS_${_build_type_upper}} ${STRICT_COMPILE_FLAGS_STR}"
                CACHE STRING "C compiler flags for ${_build_type} build type" FORCE
            )
            set(CMAKE_EXE_LINKER_FLAGS_${_build_type_upper}
                "${CMAKE_EXE_LINKER_FLAGS_${_build_type_upper}} ${STRICT_LINK_FLAGS_STR}"
                CACHE STRING "Executable linker flags for ${_build_type} build type" FORCE
            )
            set(CMAKE_SHARED_LINKER_FLAGS_${_build_type_upper}
                "${CMAKE_SHARED_LINKER_FLAGS_${_build_type_upper}} ${STRICT_LINK_FLAGS_STR}"
                CACHE STRING "Shared library linker flags for ${_build_type} build type" FORCE
            )
        endforeach()
    endif()
endif()
