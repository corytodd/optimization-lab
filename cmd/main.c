#include <rot13/rot13.h>

#include <errno.h>
// clang-tidy's include-cleaner maps getopt symbols to glibc-internal
// bits/getopt_*.h, which must never be included directly; <getopt.h>
// is the correct public header.
// NOLINTNEXTLINE(misc-include-cleaner)
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_CHUNK 4096

struct args
{
    const char* filepath;
    const char* impl;
    bool bench;
};

// NOLINTBEGIN(misc-include-cleaner)
static const struct option LONG_OPTIONS[] = {
    {"file", required_argument, NULL, 'f'},
    {"impl", required_argument, NULL, 'i'},
    {"bench", no_argument, NULL, 'b'},
    {NULL, 0, NULL, 0},
};
// NOLINTEND(misc-include-cleaner)

static struct args parse_args(int argc, char** argv)
{
    struct args result = {.impl = "scalar"};
    int opt;
    // NOLINTNEXTLINE(misc-include-cleaner)
    while((opt = getopt_long(argc, argv, "f:i:b", LONG_OPTIONS, NULL)) != -1)
    {
        switch(opt)
        {
        case 'f':
            result.filepath = optarg; // NOLINT(misc-include-cleaner)
            break;
        case 'i':
            result.impl = optarg; // NOLINT(misc-include-cleaner)
            break;
        case 'b':
            result.bench = true;
            break;
        default:
            break;
        }
    }
    return result;
}

static char* read_all(FILE* file, size_t* out_len)
{
    size_t cap = READ_CHUNK;
    size_t len = 0;
    char* buf = malloc(cap);
    if(!buf)
    {
        return NULL;
    }

    for(;;)
    {
        if(len == cap)
        {
            cap *= 2;
            char* tmp = realloc(buf, cap);
            if(!tmp)
            {
                free(buf);
                return NULL;
            }
            buf = tmp;
        }

        size_t nread = fread(buf + len, 1, cap - len, file);
        len += nread;
        if(feof(file) || ferror(file))
        {
            break;
        }
    }

    if(ferror(file))
    {
        free(buf);
        return NULL;
    }

    *out_len = len;
    return buf;
}

struct input
{
    const char* data;
    size_t len;
    char* buf; // heap-allocated backing store; NULL if data points into argv
};

static void release_input(struct input* inp)
{
    free(inp->buf);
}

static int acquire_input(int argc, char** argv, const char* filepath, struct input* out)
{
    if(filepath)
    {
        FILE* file = fopen(filepath, "r");
        if(!file)
        {
            (void) fprintf(stderr, "cannot open %s: %s\n", filepath, strerror(errno));
            return 1;
        }
        out->buf = read_all(file, &out->len);
        (void) fclose(file);
        if(!out->buf)
        {
            (void) fprintf(stderr, "failed to read %s\n", filepath);
            return 1;
        }
        out->data = out->buf;
        return 0;
    }

    // NOLINTNEXTLINE(misc-include-cleaner)
    if(optind < argc)
    {
        out->data = argv[optind];
        out->len = strlen(argv[optind]);
        return 0;
    }

    out->buf = read_all(stdin, &out->len);
    if(!out->buf)
    {
        (void) fprintf(stderr, "failed to read stdin\n");
        return 1;
    }
    out->data = out->buf;
    return 0;
}

int main(int argc, char** argv)
{
    struct args args = parse_args(argc, argv);

    const struct rot13_impl* impl = rot13_find_impl(args.impl);
    if(!impl)
    {
        (void) fprintf(stderr, "unknown impl '%s'. available:", args.impl);
        for(const struct rot13_impl* p = g_rot13_impls; p->name; ++p)
        {
            (void) fprintf(stderr, " %s", p->name);
        }
        (void) fprintf(stderr, "\n");
        return 1;
    }

    struct input inp = {0};
    if(acquire_input(argc, argv, args.filepath, &inp) != 0)
    {
        return 1;
    }

    char* result = NULL;
    int ret = impl->fn(inp.data, inp.len, &result);
    release_input(&inp);
    if(ret != 0)
    {
        (void) fprintf(stderr, "process failed: %d (%s)\n", ret, strerror(errno));
        return ret;
    }

    if(!args.bench)
    {
        (void) fputs(result, stdout);
    }
    free(result);

    return 0;
}
