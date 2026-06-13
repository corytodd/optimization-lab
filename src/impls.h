#pragma once

#include <stddef.h>

int rot13_scalar(const char* input, size_t len, char** result_out);
int rot13_lut(const char* input, size_t len, char** result_out);
