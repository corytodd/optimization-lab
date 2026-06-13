#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE (1024UL * 1024 * 1024)
#define RUNS 5

__attribute__((noinline)) static void do_copy(char* dst, const char* src, size_t n)
{
    memcpy(dst, src, n);
    __asm__ volatile("" : : "r"(dst) : "memory");
}

int main(void)
{
    char* src = malloc(SIZE);
    char* dst = malloc(SIZE);
    memset(src, 0xAB, SIZE);
    memset(dst, 0x00, SIZE);

    double best = 1e9;
    for(int i = 0; i < RUNS; i++)
    {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        do_copy(dst, src, SIZE);
        clock_gettime(CLOCK_MONOTONIC, &t1);

        double secs = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
        if(secs < best)
        {
            best = secs;
        }
    }

    double bw = (2.0 * SIZE / 1e9) / best;
    printf("best memcpy 1 GB:          %.3f s\n", best);
    printf("read+write bandwidth:      %.1f GB/s\n", bw);
    printf("rot13 speed-of-light:      %.1f ms  (1 GB input, 1 read + 1 write)\n",
           (2.0 * SIZE / 1e9) / bw * 1000.0);
    free(src);
    free(dst);
}
