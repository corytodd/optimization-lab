## Measure Once then Measure Again

Before touching your application, measure the actual performance. There are many
open-source tools available for making quick and accurate work of this task.
Benchmarking is a notoriously difficult task on modern systems with complex,
dynamic CPU and memory architectures. Use off-the-shelf tools to get the most
accurate results before attempting to roll your own.

Remember to minimize system load while working through these exercises.

### The Baseline

Record the baseline with `perf stat`, saving the output for later comparison:

```bash
# Configure and build the programs
cmake -S . -B build -DCMAKE_BUILD_TYPE=Benchmark
cmake --build build

# Generate the baseline using a wrapper for `perf stat`
./tools/run-perf.sh -o results/baseline_perf.txt -- ./build/cmd/rot13-cli -f data/data_1GB.txt --bench
```


> [!NOTE]
> If you see "event not found" errors, run `perf list` to see what your CPU exposes and update the
> `EVENTS` variable in `run-perf.sh` accordingly. `perf` uses CPU-specific hardware counters. 
> Some events in `run-perf.sh` target Intel's PMU will not exist verbatim on other other processors.


### Reading the `perf stat` Output

The metadata header written by `run-perf.sh` captures the environment so every
result is reproducible and comparable.

```
OS CPU, pinned cores, compiler, governor
```

Below it, `perf stat` reports hardware performance counters. Here is what each
field means using `baseline_perf.txt` as the reference:

| Field                                 | Baseline value | What it measures                                                                                                                                                                   |
| ------------------------------------- | -------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `cpu_core/cycles/u`                   | 7,490,870,768  | CPU clock cycles consumed in user space. Divide by your clock frequency to get CPU-time; compare across runs to track overall work.                                                |
| `cpu_core/instructions/u`             | 28,260,177,542 | Retired instructions in user space. The ratio `instructions/cycles` is IPC (instructions per cycle); higher is better.                                                             |
| `cpu_core/mem_load_retired.l1_miss/u` | 41,104         | Load instructions that missed L1 cache. Near zero here because the working set is streamed sequentially and the prefetcher handles it.                                             |
| `cpu_core/mem_load_retired.l2_miss/u` | 3,808          | Load instructions that missed both L1 and L2. Also near zero for the same reason.                                                                                                  |
| `cpu_core/mem_load_retired.l3_miss/u` | 1,200          | Load instructions that missed all three cache levels and went to DRAM. Low here despite 1 GB input because the hardware prefetcher fills cache lines ahead of the sequential scan. |
| `cpu_core/dtlb-load-misses/u`         | 580            | [TLB][tlb] misses on load addresses. Low because the input is a single contiguous allocation; the OS maps it with a small number of large pages.                                   |
| `cpu_core/dtlb-loads/u`               | 1,682,340,562  | Total [TLB][tlb] lookups for loads. The miss rate `dtlb-load-misses / dtlb-loads` is effectively 0%, confirming translation is not a bottleneck.                                   |
| `page-faults:u`                       | 988            | User-space [page faults][pagefault] (soft faults on first touch). Proportional to the allocation size; not a per-iteration cost once the run is underway.                          |
| `cpu_core/branches/u`                 | 62,383,554     | Conditional and unconditional branches executed. Roughly one branch per ~16 bytes of input, the inner loop condition that selects how to wrap the around the alphabet.             |
| `cpu_core/branch-misses/u`            | 196            | Branches where the predictor was wrong. Near zero because the loop body is perfectly predictable.                                                                                  |
| `cpu_core/uops_issued.any/u`          | 28,187,546,428 | Micro-ops issued by the front end. Should be close to `uops_retired.slots`; a large gap would indicate the back end is stalling.                                                   |
| `cpu_core/uops_retired.slots/u`       | 28,216,153,170 | Micro-ops that completed execution. Matches `uops_issued` closely, meaning almost no speculative work was discarded.                                                               |
| `time elapsed`                        | 2.153 s        | Wall-clock time including startup. Noisy for single-shot runs; use `hyperfine` for reliable timing.                                                                                |
| `user`                                | 1.835 s        | Time spent in user-space code. The bulk of elapsed time is algorithm and I/O buffering.                                                                                            |
| `sys`                                 | 0.297 s        | Time spent in kernel code on behalf of the process. Dominated by `read()` syscalls bringing 1 GB off disk into page cache.                                                         |

The `<not counted>` rows are `cpu_atom` events on the E-core PMU. Those cores
are excluded by the `taskset 0-3` pinning, so their counters never increment.
The `<not supported>` rows (`stalled-cycles-frontend/backend`) require
additional [model-specific register][msr] access that this kernel configuration
does not expose.

[tlb]: https://en.wikipedia.org/wiki/Translation_lookaside_buffer
[pagefault]: https://en.wikipedia.org/wiki/Page_fault
[msr]: https://en.wikipedia.org/wiki/Model-specific_register
