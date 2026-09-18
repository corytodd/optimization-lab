## OR-Combining Disjoint Ranges

No byte can be in two of the four ranges at once. A byte is never simultaneously
a lowercase and an uppercase letter, nor in both halves of the same case's
alphabet. Because the four masks are mutually exclusive, they can be safely
combined with a plain bitwise OR, no select or blend required:

```c
__m256i add_mask = _mm256_or_si256(in_lower_am, in_upper_am);   // want +13
__m256i sub_mask = _mm256_or_si256(in_lower_nz, in_upper_nz);   // want -13

__m256i add_shift = _mm256_and_si256(add_mask, consts->plus13);
__m256i sub_shift = _mm256_and_si256(sub_mask, consts->minus13);
__m256i shift     = _mm256_or_si256(add_shift, sub_shift);

return _mm256_add_epi8(chunk, shift);
```

`add_mask & plus13` reads as "keep `+13` where the mask is all-ones, zero it out
otherwise". The same 0xFF/0x00-per-lane trick from the classification step
doubles as a per-lane select. Three ORs total:

- two to collapse the four range masks down to "add" and "subtract,"
- one more to merge those two signed shift amounts, since a byte can only ever
  match one of the two

See [rot13_simd.c](../../../src/rot13_simd.c) for the full listing, including the
reasoning for why AVX2 (not SSE2, not AVX-512) is the right width for this
problem, in a comment above the constants struct.
