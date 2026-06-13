#include <assert.h>
#include <errno.h> // IWYU pragma: keep
#include <stdint.h>
#include <stdlib.h>

#include "impls.h"

int rot13_scalar(const char* input, size_t len, char** result_out)
{
    assert(input && "input null");
    assert(len && "len == 0");
    assert(len <= INT32_MAX && "len > INT32_MAX");

    char* output = malloc(len + 1);
    if(!output)
    {
        return -ENOMEM; // NOLINT(misc-include-cleaner)
    }

    for(int i = 0; i < (int) len; ++i)
    {
        int byte = (input[i] & 0xFF);
        if((byte >= 'a') && (byte <= 'z'))
        {
            byte = 'a' + ((byte - 'a' + 13) % 26);
        }
        else if((byte >= 'A') && (byte <= 'Z'))
        {
            byte = 'A' + ((byte - 'A' + 13) % 26);
        }
        // else the character falls through

        output[i] = (char) byte;
    }
    output[len] = '\0';
    *result_out = output;

    return 0;
}
