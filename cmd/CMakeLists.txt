set(BIN rot13-cli)

set(SOURCES
    main.c
)

add_executable(${BIN} ${SOURCES})
target_link_libraries(${BIN} PRIVATE ${PROJECT_NAME}::rot13)

target_compile_options(${BIN} PRIVATE
    -fPIE
    -fstack-clash-protection
    # _FORTIFY_SOURCE requires optimization; skip in Debug to avoid warnings
    $<$<NOT:$<CONFIG:Debug>>:-D_FORTIFY_SOURCE=2>
)

target_link_options(${BIN} PRIVATE
    -pie
    -Wl,-z,relro
    -Wl,-z,now
    -Wl,-z,noexecstack
)

target_compile_definitions(${BIN} PRIVATE
)
