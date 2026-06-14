# Low Hanging Fruit

Start by looking at the perf output to get a feel for where time is being spent.

## Reading the `perf stat` Output

The metadata header written by `run-perf.sh` captures the environment -- OS, CPU, pinned cores,
compiler, and governor -- so every result is reproducible and comparable. Below it, `perf stat`
reports hardware performance counters. Here is what each field means using `baseline_perf.txt`
as the reference:

| Field | Baseline value | What it measures |
|---|---|---|
| `cpu_core/cycles/u` | 7,490,870,768 | CPU clock cycles consumed in user space. Divide by your clock frequency to get CPU-time; compare across runs to track overall work. |
| `cpu_core/instructions/u` | 28,260,177,542 | Retired instructions in user space. The ratio `instructions/cycles` is IPC (instructions per cycle); higher is better. |
| `cpu_core/mem_load_retired.l1_miss/u` | 41,104 | Load instructions that missed L1 cache. Near zero here because the working set is streamed sequentially and the prefetcher handles it. |
| `cpu_core/mem_load_retired.l2_miss/u` | 3,808 | Load instructions that missed both L1 and L2. Also near zero for the same reason. |
| `cpu_core/mem_load_retired.l3_miss/u` | 1,200 | Load instructions that missed all three cache levels and went to DRAM. Low here despite 1 GB input because the hardware prefetcher fills cache lines ahead of the sequential scan. |
| `cpu_core/dtlb-load-misses/u` | 580 | [TLB][tlb] misses on load addresses. Low because the input is a single contiguous allocation; the OS maps it with a small number of large pages. |
| `cpu_core/dtlb-loads/u` | 1,682,340,562 | Total [TLB][tlb] lookups for loads. The miss rate `dtlb-load-misses / dtlb-loads` is effectively 0%, confirming translation is not a bottleneck. |
| `page-faults:u` | 988 | User-space [page faults][pagefault] (soft faults on first touch). Proportional to the allocation size; not a per-iteration cost once the run is underway. |
| `cpu_core/branches/u` | 62,383,554 | Conditional and unconditional branches executed. Roughly one branch per ~16 bytes of input, the inner loop condition. |
| `cpu_core/branch-misses/u` | 196 | Branches where the predictor was wrong. Near zero: the loop body is perfectly predictable. |
| `cpu_core/uops_issued.any/u` | 28,187,546,428 | Micro-ops issued by the front end. Should be close to `uops_retired.slots`; a large gap would indicate the back end is stalling. |
| `cpu_core/uops_retired.slots/u` | 28,216,153,170 | Micro-ops that completed execution. Matches `uops_issued` closely, meaning almost no speculative work was discarded. |
| `time elapsed` | 2.153 s | Wall-clock time including startup. Noisy for single-shot runs; use `hyperfine` for reliable timing. |
| `user` | 1.835 s | Time spent in user-space code. The bulk of elapsed time is algorithm and I/O buffering. |
| `sys` | 0.297 s | Time spent in kernel code on behalf of the process. Dominated by `read()` syscalls bringing 1 GB off disk into page cache. |

The `<not counted>` rows are `cpu_atom` events on the E-core PMU. Those cores are excluded by the
`taskset 0-3` pinning, so their counters never increment. The `<not supported>` rows
(`stalled-cycles-frontend/backend`) require additional MSR access that this kernel configuration
does not expose.

## What the Baseline Tells Us

**We see high IPC (`28.26B instructions / 7.49B cycles ~= 3.77`)**, so this means the core's
execution units are well-fed and retirement is smooth. A common prescription is to reduce
front-end pressure by merging micro-ops; for example, consolidating the two branch arms into
a [cmov][cmov]. That does not help here because the predictor is already near-perfect (196 misses
across 62 million branches), so there is no misprediction penalty to recover. High IPC with
fast wall-clock time is healthy; high IPC with slow wall-clock time, as we see here, just means
the CPU is efficiently executing the wrong amount of work.

**We see ~28 billion micro-ops to process 1 GB of input**, so this means roughly 28
instructions per byte. That is the real problem: the scalar loop does far too much work per
byte (two range comparisons, two conditional branches, subtract, modulo, add) and no
amount of branch or pipeline tuning changes that ratio. The fix is to process more bytes per
instruction, which is what SIMD does.

**We see negligible cache misses (1,200 L3 misses across 1 GB)**, so this means the hardware
prefetcher is keeping up with the sequential scan. A standard prescription for high miss counts
is to add software prefetch hints (`__builtin_prefetch`). That would be wasted effort here as
the prefetcher is already doing its job. Verifying this before reaching for prefetch hints is
exactly the point of reading the counters first.

**We see `sys` time at 0.297 s (14% of elapsed)**, so this means `read()` syscalls are a
secondary but real cost; the file is being copied from the kernel's page cache into our
`malloc` buffer. Replacing `fread` with `mmap` eliminates that copy. Whether it moves the
needle enough to matter depends on how much we close the gap on the compute side first.

[tlb]: https://en.wikipedia.org/wiki/Translation_lookaside_buffer
[cmov]: https://www.felixcloutier.com/x86/cmovcc
[pagefault]: https://en.wikipedia.org/wiki/Page_fault
