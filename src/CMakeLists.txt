set(LIB rot13)

set(SOURCES
    rot13_registry.c
    rot13_scalar.c
    rot13_lut.c
    rot13_table.c
)

add_library(${LIB} ${SOURCES})
add_library(${PROJECT_NAME}::${LIB} ALIAS ${LIB})

target_include_directories(${LIB} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)
