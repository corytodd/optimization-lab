## Branchless Range Classification

The scalar and LUT builds both encode "is this a letter, and which case is it"
as a comparison. SIMD has no per-lane branch, so classification has to be
arithmetic. The identity this build leans on:

```
in_range(c, lower, upper) iff (uint8_t)(c - lower) <= (uint8_t)(upper - lower)
```

`c` is the ordinal value of the character. `(uint8_t)(c - lower)` is unsigned
subtraction mod 256. If `c` is inside `[lower, upper]`, the result is small and
positive. If `c` is below `lower`, the subtraction wraps to a large value close
to 256. Either way, one unsigned comparison against `upper - lower` now answers
a two-sided bounds check. There is no unsigned `<=` instruction for bytes in
AVX2, only `==` and signed `>`, so the last step fakes it with `min` and
equality.

`min_epu8(delta, range) == delta` is true only when `delta <= range`, since the
min can only equal `delta` when `delta` was already the smaller value.

`'a'..'m'`, `'n'..'z'`, `'A'..'M'`, and `'N'..'Z'` are each 13 letters wide, so
`upper - lower` is `12` in all four cases, reused for every range check instead
of four separate bounds.

```
  'a'..'m'        'n'..'z'         'A'..'M'        'N'..'Z'
 +---------+     +---------+     +---------+     +---------+
 | +13     |     | -13     |     | +13     |     | -13     |
 +---------+     +---------+     +---------+     +---------+
  lower_am        lower_nz        upper_am        upper_nz
```

| Char | Value | δ vs `'a'`<br>Char - 'a' | δ vs `'n'`<br>Char - 'n' | δ vs `'A'`<br>Char - 'A' | δ vs `'N'`<br>Char - 'N' | Rotation value   |
|------|-------|--------------------------|--------------------------|--------------------------|--------------------------|------------------|
| `H`  | 72    | 231                      | 218                      | **7**                    | 250                      | A..M → +13       |
| `e`  | 101   | **4**                    | 247                      | 36                       | 23                       | a..m → +13       |
| `y`  | 121   | 24                       | **11**                   | 56                       | 43                       | n..z → −13       |
| `,`  | 44    | 203                      | 190                      | 235                      | 222                      | none → unchanged |
| ` `  | 32    | 191                      | 178                      | 223                      | 210                      | none → unchanged |
| `Z`  | 90    | 249                      | 236                      | 25                       | **12**                   | N..Z → −13       |
| `q`  | 113   | 16                       | **3**                    | 48                       | 35                       | n..z → −13       |
| `!`  | 33    | 192                      | 179                      | 224                      | 211                      | none → unchanged |

Each byte lane gets checked against all four ranges independently, in parallel,
across all 32 bytes of the chunk at once. There is no per-lane loop, one
instruction covers all 32 lanes:

```c
__m256i delta_lower_am = _mm256_sub_epi8(chunk, consts->lower_a);
__m256i min_lower_am = _mm256_min_epu8(delta_lower_am, consts->range12);
__m256i in_lower_am = _mm256_cmpeq_epi8(min_lower_am, delta_lower_am);
```

`_mm256_cmpeq_epi8` doesn't return a single bit per lane. There is no packed
boolean representation for byte compares in AVX2. Each lane gets filled with
`0xFF` (match) or `0x00` (no match), a full byte-wide mask. The mask operation
is explained in the next section.
