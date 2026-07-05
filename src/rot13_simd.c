#include <assert.h>
#include <errno.h>
#include <rot13/rot13.h>
#include <stdint.h>
#include <stdlib.h>

#include "impls.h"

#include <immintrin.h>

// LUT is still required for any leftover bytes from inputs
// not aligned to sizeof(__m256i)
extern const uint8_t g_rot13_table[256];

// Vectorized ROT13: rather than a 256-entry LUT lookup per byte, classify
// each byte with branchless unsigned range checks and add/subtract 13.
//   in_range(c, lower, upper) <=> (uint8_t)(c - lower) <= (uint8_t)(upper - lower)
//                             <=> min_epu8(c - lower, upper - lower) == (c - lower)
// 'a'..'m' / 'A'..'M' shift forward by 13
// 'n'..'z' / 'N'..'Z' shift back by 13
// the four ranges are disjoint so the deltas can be OR-combined.
//
// Widths: 
//  AVX2 (__m256i, 32B/iter) is the sweet spot here.
//  SSE2 (__m128i, 16B/iter) would halve throughput for no real portability gain.
//  AVX-512 (__m512i) is not a drop-in widen. _mm512_cmpeq_epi8 returns a
// __mmask64, not a byte vector of 0xFF/0x00 lanes, so the AND/OR-combine of masks
// below wouldn't compile as-is. It'd need to be rewritten around mask ops instead 
// of the current vector bitwise AND/OR.
// More importantly, I don't have AVX-512 hardware!
typedef struct
{
    __m256i lower_a;
    __m256i lower_n;
    __m256i upper_a;
    __m256i upper_n;
    __m256i range12;
    __m256i plus13;
    __m256i minus13;
} rot13_simd_consts_t;

//   'a'..'m'        'n'..'z'         'A'..'M'        'N'..'Z'
//  +---------+     +---------+     +---------+     +---------+
//  | +13     |     | -13     |     | +13     |     | -13     |
//  +---------+     +---------+     +---------+     +---------+
//   lower_am        lower_nz        upper_am        upper_nz
//
// chunk = [ b0 b1 b2 ... b31 ]  (32 lanes, sizeof(__m256i))
// Every step below runs on all 32 lanes at once -- the pipeline is drawn
// for a single lane, but there is no loop: one instruction = 32 bytes.
//        chunk (32 bytes)
//              |
//   +----------+----------+----------+----------+
//   |          |          |          |          |
// -lower_a   -lower_n   -upper_a   -upper_n      (parallel subtracts)
//   |          |          |          |
// <=12?      <=12?      <=12?      <=12?         (min_epu8 == delta trick)
//   |          |          |          |
// in_lower_am in_lower_nz in_upper_am in_upper_nz (disjoint! only one byte will be 0xFF)
//    \___OR___/            \___OR___/
//   add_mask               sub_mask
//   (want +13)             (want -13)
//        \_________OR_________/
//               shift
//                 |
//         chunk + shift = result
static inline __m256i rot13_shift_chunk(__m256i chunk, const rot13_simd_consts_t* consts)
{
    __m256i delta_lower_am = _mm256_sub_epi8(chunk, consts->lower_a);
    __m256i min_lower_am = _mm256_min_epu8(delta_lower_am, consts->range12);
    __m256i in_lower_am = _mm256_cmpeq_epi8(min_lower_am, delta_lower_am); // [0] "not in lower_am", [FF] "in lower_am"

    __m256i delta_lower_nz = _mm256_sub_epi8(chunk, consts->lower_n);
    __m256i min_lower_nz = _mm256_min_epu8(delta_lower_nz, consts->range12);
    __m256i in_lower_nz = _mm256_cmpeq_epi8(min_lower_nz, delta_lower_nz);

    __m256i delta_upper_am = _mm256_sub_epi8(chunk, consts->upper_a);
    __m256i min_upper_am = _mm256_min_epu8(delta_upper_am, consts->range12);
    __m256i in_upper_am = _mm256_cmpeq_epi8(min_upper_am, delta_upper_am);

    __m256i delta_upper_nz = _mm256_sub_epi8(chunk, consts->upper_n);
    __m256i min_upper_nz = _mm256_min_epu8(delta_upper_nz, consts->range12);
    __m256i in_upper_nz = _mm256_cmpeq_epi8(min_upper_nz, delta_upper_nz);

    __m256i add_mask = _mm256_or_si256(in_lower_am, in_upper_am);
    __m256i sub_mask = _mm256_or_si256(in_lower_nz, in_upper_nz);

    __m256i add_shift = _mm256_and_si256(add_mask, consts->plus13);
    __m256i sub_shift = _mm256_and_si256(sub_mask, consts->minus13);
    __m256i shift = _mm256_or_si256(add_shift, sub_shift);

    __m256i result = _mm256_add_epi8(chunk, shift);

    return result;
}

int rot13_simd(const char* input, size_t len, char** result_out)
{
    assert(input && "input null");
    assert(len && "len == 0");
    assert(len <= INT32_MAX && "len > INT32_MAX");

    char* output = malloc(len + 1);
    if(!output)
    {
        return -ENOMEM;
    }

    size_t pos = 0;

    // See rot13_shift_chunk for the in_range/min_epu8 classification trick.
    const rot13_simd_consts_t consts = {
        .lower_a = _mm256_set1_epi8('a'), // broadcast 'a' across all bytes
        .lower_n = _mm256_set1_epi8('n'), // This is like Python:
        .upper_a = _mm256_set1_epi8('A'), //     'A' * 32
        .upper_n = _mm256_set1_epi8('N'),
        .range12 = _mm256_set1_epi8(12),
        .plus13 = _mm256_set1_epi8(13),
        .minus13 = _mm256_set1_epi8(-13),
    };

    for(; (pos + sizeof(__m256i)) <= len; pos += sizeof(__m256i))
    {
        // Cast to __m256i_u (align(1)) rather than __m256i (align(32))
        // so the cast doesn't claim an alignment guarantee we don't have.
        __m256i chunk = _mm256_loadu_si256((const __m256i_u*) (input + pos));

        __m256i result = rot13_shift_chunk(chunk, &consts);

        _mm256_storeu_si256((__m256i_u*) (output + pos), result);
    }

    // Use LUT for any remaining bytes
    for(; pos < len; ++pos)
    {
        int byte = (input[pos] & 0xFF);
        output[pos] = (char) g_rot13_table[byte];
    }
    output[len] = '\0';
    *result_out = output;

    return 0;
}