#pragma once

#include <stddef.h>

/**
 * @brief Apply rot13 to @p input writing a null-terminated result into @p result_out
 *
 * @param input Buffer to process; non-null
 * @param len   Length of @p input in bytes; 0 < @p len <= INT32_MAX
 * @param[out] result_out Receives output buffer; non-null, caller must free
 *
 * @return 0 on success, <0 on error
 */
typedef int (*rot13_fn)(const char* input, size_t len, char** result_out);

struct rot13_impl
{
    const char* name;
    rot13_fn fn;
};

/// @brief NULL-name-terminated table of all registered implementations
extern const struct rot13_impl g_rot13_impls[];

/**
 * @brief Return implementation by @p name
 *
 * @param name Register implementation name
 * @return Implementation or NULL if not found
 */
const struct rot13_impl* rot13_find_impl(const char* name);
