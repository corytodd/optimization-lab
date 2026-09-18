# Embarrassingly Parallel

In Chapter 2, we identified two sensible courses of action:

1. Execute fewer instructions
2. Do more work per instruction

The look up table approach helped us minimize the number of instructions
required to complete the same amount of work. That leaves us with the task of
doing more work per instruction.

The `rot13` algorithm just so happens to be a textbook candidate for SIMD. Every
output byte depends only on the input byte at the same position. Nothing carries
across the calculation loop which relaxes the order in which the instructions
are allowed to execute. Problems shaped like this are called *embarrassingly
parallel*. Parallelizing them takes no restructuring, no synchronization, no
shared state to reason about. This chapter processes 32 bytes per instruction
using AVX2, with no branches at all in the hot path.

You may dare say this was planned all along.

## One Instruction, 32 Bytes

A `__m256i` is a 256-bit SIMD register which allows for 32 lanes of one byte
each. Arithmetic and comparison intrinsics like `_mm256_sub_epi8` or
`_mm256_cmpeq_epi8` operate on all 32 lanes simultaneously, in the same
instruction, the same cycle. Where the scalar and LUT builds spend one loop
iteration per byte, the SIMD build spends one iteration per 32 bytes:

```c
for(; (pos + sizeof(__m256i)) <= len; pos += sizeof(__m256i))
{
    __m256i chunk = _mm256_loadu_si256((const __m256i_u*) (input + pos));
    __m256i result = rot13_shift_chunk(chunk, &consts);
    _mm256_storeu_si256((__m256i_u*) (output + pos), result);
}
```

Loads and stores use the unaligned (`loadu`/`storeu`) forms because `input`/
`output` come from `malloc` at arbitrary byte offsets, with no guarantee of
32-byte alignment. The cast targets are `__m256i_u` (alignment 1), not `__m256i`
(alignment 32) because casting to the aligned type would claim an alignment
guarantee the pointer doesn't actually have, which is what `-Wcast-align` warns
about. `__m256i_u` matches the true alignment and the intrinsic's own declared
parameter type. 

> [!WARNING]
> Pay attention to these warnings or else you run the risk of
> performance penalties, or worse, undefined behavior (UB).

Any bytes left over once `len` isn't a multiple of 32 fall through to a scalar
tail loop that reuses the Chapter 3 LUT.

```c
for(; pos < len; ++pos)
{
    output[pos] = (char) g_rot13_table[(input[pos] & 0xFF)];
}
```
