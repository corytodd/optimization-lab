## Results

```bash
./tools/run-perf.sh -o \
  results/simd_perf.txt -- \
  ./build/cmd/rot13-cli \
  -f data/data_1GB.txt \
  --bench --impl simd
```

| Field                        | Baseline       | LUT           | SIMD        | LUT -> SIMD      |
| ---------------------------- | -------------- | ------------- | ----------- | ---------------- |
| `cpu_core/cycles/u`          | 7,490,870,768  | 1,138,221,410 | 428,068,700 | 2.7x fewer       |
| `cpu_core/instructions/u`    | 28,260,177,542 | 6,004,558,237 | 755,869,166 | 7.9x fewer       |
| instructions / byte          | 26.3           | 5.6           | 0.7         | 7.9x fewer       |
| IPC (`instructions/cycles`)  | 3.77           | 5.28          | 1.77        | lower            |
| `cpu_core/branches/u`        | 62,383,554     | 1,004,003,575 | 30,817,924  | 32.6x fewer      |
| `cpu_core/branch-misses/u`   | 196            | 16            | 199         | --               |
| `mem_load_retired.l2_miss/u` | 3,808          | 11,036        | 208,863     | 19x more         |
| `mem_load_retired.l3_miss/u` | 1,200          | 10,397        | 207,376     | 20x more         |
| `dtlb-loads/u`               | 1,682,340,562  | 2,007,116,244 | 30,648,952  | 65x fewer        |
| `page-faults:u`              | 988            | 244,224       | 987         | back to baseline |

## What the Results Tell Us

**Instructions per byte drop to 0.7**, down from the LUT's already-good 5.6 and
the baseline's 26.3. That tracks directly with the chunk width. One
classify-and-shift sequence now costs the same handful of instructions
regardless of whether it covers 1 byte or 32, so widening the chunk divides the
per-byte instruction cost by roughly the same factor.

**IPC drops to 1.77, below the LUT's 5.28.** This looks like a regression but it
isn't. IPC measures instructions retired per cycle, not bytes processed per
cycle. Each vector instruction here does 32x the work of a scalar one, so the
front end doesn't need to issue as many of them to keep the execution units fed.
Cycles fell by 2.7x in step with the 7.9x fall in instructions.
That's the number that maps to wall-clock time, and it moved in the right
direction. Instructions/byte and raw cycles are what to trust here.

**Branches fall 32.6x, from the LUT's one-per-byte loop condition to one-per-32-bytes.**
The remaining branches are still almost perfectly predicted at 199 misses across
30.8M, so we're observing the loop running 32x fewer times, not recovery in our
mispredictions.

**L2 and L3 misses rise sharply (19-20x) even though L1 misses fall.** The
likely explanation is that the LUT build was slow enough that the hardware
prefetcher had plenty of slack to stay ahead of consumption, keeping most
accesses resolved in L1. The SIMD build consumes memory so much faster that
the prefetcher can't stay as far ahead, so a larger share of accesses that
would have been hidden in L1 is now surfaced as L2/L3 traffic. This suggests
that we're approaching the memory-bound floor from Chapter 2. Compute time
shrank far enough that previously invisible memory subsystem behavior is now
part of the picture.

**Page faults return to baseline levels (987, vs the LUT run's 244,224).**
Chapter 3 speculated that the LUT's larger `sys` time might trace to page-fault
handling on the ~1 GB output buffer. But that allocation is identical here, and
page faults did not scale with it. That number was most likely single-run noise,
a reminder that `perf stat`'s hardware counters, like its wall-clock time, are a
single sample and worth treating with the same skepticism.
