#include <rot13/rot13.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PASS(name) (void) printf("PASS: %s\n", (name))
#define FAIL(name, msg)                                           \
    do                                                            \
    {                                                             \
        (void) fprintf(stderr, "FAIL: %s — %s\n", (name), (msg)); \
        return 1;                                                 \
    } while(0)

#define EXPECT_STR(fn, test_name, input, expected)                                              \
    do                                                                                          \
    {                                                                                           \
        char* actual = NULL;                                                                    \
        if((fn) ((input), strlen(input), &actual) != 0)                                         \
        {                                                                                       \
            FAIL((test_name), "rot13 fn returned error");                                       \
        }                                                                                       \
        if(strcmp(actual, (expected)) != 0)                                                     \
        {                                                                                       \
            (void) fprintf(stderr, "FAIL: %s - got \"%s\", want \"%s\"\n", (test_name), actual, \
                           (expected));                                                         \
            free(actual);                                                                       \
            return 1;                                                                           \
        }                                                                                       \
        free(actual);                                                                           \
        PASS(test_name);                                                                        \
    } while(0)

static int test_lowercase(rot13_fn fn)
{
    EXPECT_STR(fn, "lowercase", "hello", "uryyb");
    return 0;
}

static int test_uppercase(rot13_fn fn)
{
    EXPECT_STR(fn, "uppercase", "HELLO", "URYYB");
    return 0;
}

static int test_mixed_case(rot13_fn fn)
{
    EXPECT_STR(fn, "mixed_case", "Hello, World!", "Uryyb, Jbeyq!");
    return 0;
}

static int test_non_alpha_passthrough(rot13_fn fn)
{
    EXPECT_STR(fn, "non_alpha_passthrough", "abc 123 !@#", "nop 123 !@#");
    return 0;
}

static int test_self_inverse(rot13_fn fn)
{
    const char* input = "The Quick Brown Fox";
    char* first = NULL;
    char* second = NULL;

    if(fn(input, strlen(input), &first) != 0)
    {
        FAIL("self_inverse", "first call returned error");
    }
    if(fn(first, strlen(first), &second) != 0)
    {
        free(first);
        FAIL("self_inverse", "second call returned error");
    }
    free(first);

    if(strcmp(second, input) != 0)
    {
        (void) fprintf(stderr, "FAIL: self_inverse -- got \"%s\", want \"%s\"\n", second, input);
        free(second);
        return 1;
    }
    free(second);
    PASS("self_inverse");
    return 0;
}

static int test_alphabet_boundary(rot13_fn fn)
{
    EXPECT_STR(fn, "alphabet_boundary_lower", "mnopqrstuvwxyzabcdefghijkl",
               "zabcdefghijklmnopqrstuvwxy");
    EXPECT_STR(fn, "alphabet_boundary_upper", "MNOPQRSTUVWXYZABCDEFGHIJKL",
               "ZABCDEFGHIJKLMNOPQRSTUVWXY");
    return 0;
}

static int run_suite(rot13_fn fn)
{
    int failed = 0;
    failed += test_lowercase(fn);
    failed += test_uppercase(fn);
    failed += test_mixed_case(fn);
    failed += test_non_alpha_passthrough(fn);
    failed += test_self_inverse(fn);
    failed += test_alphabet_boundary(fn);
    return failed;
}

int main(void)
{
    int failed = 0;
    for(const struct rot13_impl* impl = g_rot13_impls; impl->name; ++impl)
    {
        (void) printf("-- impl: %s\n", impl->name);
        failed += run_suite(impl->fn);
    }

    if(failed > 0)
    {
        (void) fprintf(stderr, "\n%d test(s) failed.\n", failed);
        return 1;
    }

    (void) printf("\nAll tests passed.\n");
    return 0;
}
