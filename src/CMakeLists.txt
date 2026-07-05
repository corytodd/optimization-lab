set(LIB rot13)

include(CheckCCompilerFlag)
check_c_compiler_flag(-mavx2 ROT13_HAVE_AVX2)

if(NOT ROT13_HAVE_AVX2)
    message(FATAL_ERROR "Compiler does not support -mavx2")
endif()

set(SOURCES
    rot13_registry.c
    rot13_scalar.c
    rot13_lut.c
    rot13_table.c
    rot13_simd.c
)

add_library(${LIB} ${SOURCES})
add_library(${PROJECT_NAME}::${LIB} ALIAS ${LIB})

target_compile_definitions(${LIB} PRIVATE ROT13_SIMD_AVX2)
target_compile_options(${LIB} PRIVATE -mavx2)

target_include_directories(${LIB} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)
