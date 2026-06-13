# SPDX-License-Identifier: MIT
# Optional targets: format, format-fix, tidy
# Requires clang-format and clang-tidy to be on PATH (versioned names tried first)

find_program(CLANG_FORMAT NAMES clang-format-19 clang-format)
find_program(CLANG_TIDY   NAMES clang-tidy-19   clang-tidy)

file(GLOB_RECURSE ALL_SOURCE_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/cmd/*.c
    ${CMAKE_SOURCE_DIR}/include/*.h
)

file(GLOB_RECURSE ALL_C_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/cmd/*.c
)

if(CLANG_FORMAT)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} --dry-run --Werror ${ALL_SOURCE_FILES}
        COMMENT "Checking clang-format"
        VERBATIM
    )
    add_custom_target(format-fix
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
        COMMENT "Applying clang-format fixes"
        VERBATIM
    )
else()
    message(STATUS "clang-format not found -- format and format-fix targets unavailable")
endif()

if(CLANG_TIDY)
    add_custom_target(tidy
        COMMAND ${CLANG_TIDY}
            -p ${CMAKE_BINARY_DIR}
            --header-filter=${CMAKE_SOURCE_DIR}/include/.*
            --warnings-as-errors=*
            ${ALL_C_FILES}
        COMMENT "Running clang-tidy"
        VERBATIM
    )
else()
    message(STATUS "clang-tidy not found -- tidy target unavailable")
endif()
