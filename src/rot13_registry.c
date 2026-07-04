#include <rot13/rot13.h>
#include <string.h>

#include <assert.h>

#include "impls.h"

const struct rot13_impl g_rot13_impls[] = {
    {"scalar", rot13_scalar},
    {"lut",    rot13_lut   },
    {NULL, NULL},
};

#define N_IMPLS (sizeof(g_rot13_impls) / sizeof(g_rot13_impls[0]))
const struct rot13_impl* rot13_find_impl(const char* name)
{
    assert(g_rot13_impls[N_IMPLS - 1].name == NULL && "last entry must be the NULL sentinel");
    for(size_t i = 0; i < N_IMPLS - 1; ++i)
    {
        if(strcmp(g_rot13_impls[i].name, name) == 0)
        {
            return &g_rot13_impls[i];
        }
    }
    return NULL;
}
