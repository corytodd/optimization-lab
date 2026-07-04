#include <assert.h>
#include <errno.h>
#include <rot13/rot13.h>
#include <stdint.h>
#include <stdlib.h>

#include "impls.h"

extern const uint8_t g_rot13_table[256];

int rot13_lut(const char* input, size_t len, char** result_out)
{
    assert(input && "input null");
    assert(len && "len == 0");
    assert(len <= INT32_MAX && "len > INT32_MAX");

    char* output = malloc(len + 1);
    if(!output)
    {
        return -ENOMEM;
    }

    for(int i = 0; i < (int) len; ++i)
    {
        int byte = (input[i] & 0xFF);
        output[i] = (char) g_rot13_table[byte];
    }
    output[len] = '\0';
    *result_out = output;

    return 0;
}
