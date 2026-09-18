## What the Baseline Tells Us

**We see high IPC (`28.26B instructions / 7.49B cycles ~= 3.77`)**, this means
the core's execution units are well-fed and retirement is smooth. Retirement is
when the CPU commits a pipelined instruction the system. The alternative is a
discard due to incorrect, speculative execution. Since our issued count is
approximately equal to retired count, we have proof that the CPU is not wasting
cycles on mispredictions.

```
  issued:   ADD R1, R2    ; speculatively dispatched
  issued:   MUL R3, R4    ; speculatively dispatched
  issued:   CMP R5, 0     ; branch condition
  issued:   MOV R6, [bad] ; on wrong path -- never retires
            ^-- branch mispredicted, pipeline flushed
  retired:  ADD R1, R2    ; committed
  retired:  MUL R3, R4    ; committed
  retired:  CMP R5, 0     ; committed
            MOV R6, [bad] ; discarded, uops_issued but not uops_retired
```

A common prescription is to reduce front-end pressure by merging micro-ops.
For example, consolidating the two branch arms into a [cmov][cmov]. That does
not help here because the predictor is already near-perfect with 196 misses
across 62 million branches, so there is no misprediction penalty to recover.
High IPC with fast wall-clock time is healthy. High IPC with slow wall-clock
time, as we see here, just means the CPU is efficiently executing a large amount
of work.

```c
// two branch arms
if (byte >= 'a' && byte <= 'z') {
    byte = 'a' + (byte - 'a' + 13) % 26;
}
else if (byte >= 'A' && byte <= 'Z') {
    byte = 'A' + (byte - 'A' + 13) % 26;
}

// equivalent with cmov has one path, no branch
int lower = 'a' + (byte - 'a' + 13) % 26;
int upper = 'A' + (byte - 'A' + 13) % 26;
int is_lower = (byte >= 'a' && byte <= 'z');
int is_upper = (byte >= 'A' && byte <= 'Z');
byte = is_lower ? lower : (is_upper ? upper : byte);
```

The `cmov` form eliminates the branch entirely at the cost of always computing
both arms. That trade is only worth making when mispredictions are frequent.
Here they are not, so the branch version and the `cmov` version perform
identically and the compiler will often generate `cmov` anyway at `-O3`.

**We see ~28 billion micro-ops to process 1 GB of input**, this means roughly 28
instructions per byte. This is a great place to start scrutinizing. The scalar loop
does far too much work per byte with two range comparisons, two conditional
branches, subtract, modulo, add. No amount of branch or pipeline tuning changes
that ratio. The fix is to process more bytes per instruction or execute less
instructions.

**We see negligible cache misses (1,200 L3 misses across 1 GB)**, so this means
the hardware prefetcher is keeping up with the sequential scan. A common
prescription for high miss counts is to add software prefetch hints via
`__builtin_prefetch`. That would be wasted effort here as the prefetcher is
already doing its job. Verifying this before reaching for prefetch hints is why
we read the counters first.

```c
for (int i = 0; i < (int)len; ++i)
{
    // no value in this case because the hardware prefetcher already handles sequential access.
    // Adding this hint buys nothing and adds an instruction to every iteration
    __builtin_prefetch(&input[i + 64], 0, 0);

    int byte = (input[i] & 0xFF);
    // ...
}
```

**We see `sys` time at 0.297 s (14% of elapsed)**, this means `read()` syscalls
are contributing to the execution time. The file is being copied from the
kernel's page cache into our `malloc` buffer. One prescription for this would be
to replace `fread` with `mmap`. This eliminates the copy from the input file to
the process input. This only works because we're reading from a file. If `stdin`
were instead the source, this would be a less meaningful change.

[cmov]: https://www.felixcloutier.com/x86/cmovcc
