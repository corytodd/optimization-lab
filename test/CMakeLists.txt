add_executable(test_rot13 test_rot13.c)
target_link_libraries(test_rot13 PRIVATE rot13)

add_test(NAME rot13 COMMAND test_rot13)
