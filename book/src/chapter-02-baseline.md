# The Baseline

Record the baseline with `perf stat`, saving the output for later comparison:

```bash
./tools/run-perf.sh -o results/baseline_perf.txt -- ./build/cmd/rot13-cli -f data/data_1GB.txt --bench
```

Minimize system load while measuring; background processes and frequency scaling both
introduce noise that can mask or exaggerate real changes. Ensure your power profile is
running in the same mode across tests, for example `performance` mode.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Benchmark
cmake --build build
```

## CPU Affinity

Both `run-perf.sh` and `flame.sh` pin the process to cores `0-3` via `taskset`. On this
machine (i7-1255U) those are the two P-cores, which keeps the process off the E-cores and
prevents the scheduler from migrating it mid-run. On a different machine the core numbering
will differ; check `lscpu` and override with `TASKSET_CORES=<range>` if needed:

```bash
TASKSET_CORES=0-3 ./tools/run-perf.sh ...
```

> perf uses CPU-specific hardware counters. The events in `run-perf.sh` target Intel's PMU:
> `mem_load_retired.*`, `uops_issued.any`, and `uops_retired.slots` are Intel names and will
> not exist verbatim on AMD or ARM. If you see "event not found" errors, run `perf list` to
> see what your CPU exposes and update the `EVENTS` variable in `run-perf.sh` accordingly.
> `cycles`, `instructions`, `branches`, `branch-misses`, and `page-faults` are generic and
> work everywhere.

## Stdout and Measurement Noise

The binary writes its result to stdout, which adds cost that has nothing to do with the
algorithm. Redirecting to `/dev/null` eliminates the kernel-side `write()` cost, but the
user-space work, buffering and copying the output into stdio's internal buffer, still runs
and still appears in `perf stat`'s `:u` counters. To truly isolate the computation, suppress
output entirely with `--bench`. The `:u` suffix on perf events is a useful reminder: it only
counts user-space work, so stdout overhead shows up there regardless of where the output goes.

### Our First Datapoint

Compare the instruction counts with and without `--bench`. Without it, `fputs` copies 1 GB
through stdio's internal buffer, a user-space memcpy that adds millions of instructions to
the `:u` counter, none of which belong to the algorithm. With `--bench` those instructions
disappear and the counter reflects only `rot13_process` and its supporting work.

This is not an optimization, the binary is not faster at its actual job. It is measurement
hygiene: making sure the numbers describe the thing being studied. Every benchmark involves
this kind of scoping decision. Getting it wrong doesn't corrupt the result catastrophically,
but it adds noise that can obscure real differences between implementations, especially when
those differences are small.

## Speed of Light

Before optimizing, establish the theoretical maximum: the fastest this program can possibly
run, regardless of how clever the algorithm is. Since rot13 reads every input byte once and
writes every output byte once, it is purely memory-bandwidth-bound. No amount of CPU
optimization can make it faster than the rate at which memory can supply and absorb data.

Measure the achievable read+write bandwidth on this machine:

```bash
./tools/bw-probe.sh
```

```
best memcpy 1 GB:          0.106 s
read+write bandwidth:      20.2 GB/s
rot13 speed-of-light:      106.3 ms  (1 GB input, 1 read + 1 write)
```

110 ms is the floor. Any implementation that processes 1 GB slower than that is leaving
performance on the table; any implementation that matches it has extracted everything the
hardware can give. The scalar baseline runs in ~1.85 s, about 16x above the limit. That gap
is the optimization budget.

## Flame Graph

To produce a flame graph and get per-instruction annotation:

```bash
./tools/flame.sh -n baseline -- ./build/cmd/rot13-cli -f data/data_1GB.txt --bench
# produces results/baseline_flame.svg and perf-out/baseline.data
# per-instruction breakdown (--stdio avoids libcapstone requirement):
# perf annotate --stdio -i perf-out/baseline.data
```

For a tight single-function loop like this, the flame graph will show `rot13_process` dominating
unconditionally. The interesting question is _which instructions within it_ are slow. That is what
`perf annotate --stdio` answers: it interleaves source lines with sampled instruction counts, showing
exactly where cycles are going inside the function.

## Progress Chart

`perf stat` runs the command once with no warmup, so its wall-clock time is noisy. Use
`hyperfine` instead: it does warmup runs and reports mean, stddev, and min across many
iterations, making small improvements reliably detectable. Export to JSON so `plot-results.py`
can pick it up automatically. Name each file `<label>_hyperfine.json`:

```bash
hyperfine --warmup 3 --export-json results/new_optimization_hyperfine.json \
  './build/cmd/rot13-cli -f data/data_1GB.txt --bench'
# --sol is speed-of-light from bw-probe.sh output
python3 tools/plot-results.py --sol 106.3 --results results/ --out results/chart.svg
```

![Baseline progress chart](images/baseline_chart.svg)

## Summary

We have established our speed-of-light (106 ms on this system) and our scalar baseline (2153 ms),
a gap of roughly 20x. System-specifics like P-core affinity, CPU governor, and compiler version
are captured in the metadata header written by `run-perf.sh`, so every result is tied to a known
environment and regressions are traceable to a specific change.

From here, each optimization attempt follows the same loop: implement, confirm with `hyperfine`,
profile with `perf stat` and `perf annotate`, add to the progress chart. The 106 ms floor is the
target; anything that reaches it has extracted everything the hardware can give.
