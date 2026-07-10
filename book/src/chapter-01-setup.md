# Setup

## Tools

- **clang** 22.1.6 (LLVM toolchain preferred; GCC works but SIMD intrinsics may differ)
- **cmake** 4.3.4
- **python** 3.x
- **python-matplotlib** Optional, for recreating charts
- **perf** 7.0.10-1 hardware performance counter collection and call-graph sampling
- **hyperfine** v1.20.0 wall-clock benchmarking with warmup and statistical summary
- **stackcollapse-perf** + **flamegraph** from [Brendan Gregg's FlameGraph repo](https://github.com/brendangregg/FlameGraph), required by `flame.sh`

## Building

Test your system setup by building the application and running the tests.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Benchmark
cmake --build build
ctest --test-dir build --output-on-failure
```

Build in Benchmark mode, which compiles at `-O3` with debug info retained for profiling.

## Test Data

The rest of this book uses a static 1GB sample dataset. Generate this now.

```bash
./tools/gen-data.py data/data_1GB.txt
```

## System Setup

Check your power profile and take note of your CPU topology before proceeding. Both of
these significantly affect application performance. Running in `performance` mode
is generally what you want but whatever you choose, be consistent. For CPU topology,
write down the cores with the high frequency so you can pin work to this cores.

```bash
 cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# If the output of the above is not performance...
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# ... and check again
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

```bash
lscpu -e=CPU,CORE,MAXMHZ
CPU CORE    MAXMHZ
  0    0 4700.0000
  1    0 4700.0000
  2    1 4700.0000
  3    1 4700.0000 # End of P-cores
  4    2 3500.0000 # Start of E-cores
  5    3 3500.0000
  6    4 3500.0000
  7    5 3500.0000
  8    6 3500.0000
  9    7 3500.0000
 10    8 3500.0000
 11    9 3500.0000
```

Don't rely on globbing `/sys/devices/system/cpu/cpu*/...` and reading the results in order:
the shell expands that glob lexically, so on any machine with 10+ CPUs the order comes out
`cpu0, cpu1, cpu10, cpu11, cpu2, ...` instead of numeric order, silently mismatching CPU
numbers to frequencies. `lscpu -e` reads the same topology but sorts by CPU number correctly.

Both `run-perf.sh` and `flame.sh` pin the process to cores `0-3` via `taskset`. On this
machine (i7-1255U) those are the two P-cores, which keeps the process off the E-cores and
prevents the scheduler from migrating it mid-run. On a different machine the core numbering
will differ; check `lscpu -e=CPU,CORE,MAXMHZ` and override with `TASKSET_CORES=<range>` if needed:

```bash
TASKSET_CORES=0-3 ./tools/run-perf.sh ...
```

> perf uses CPU-specific hardware counters. The events in `run-perf.sh` target Intel's PMU:
> `mem_load_retired.*`, `uops_issued.any`, and `uops_retired.slots` are Intel names and will
> not exist verbatim on AMD or ARM. If you see "event not found" errors, run `perf list` to
> see what your CPU exposes and update the `EVENTS` variable in `run-perf.sh` accordingly.
> `cycles`, `instructions`, `branches`, `branch-misses`, and `page-faults` are generic and
> work everywhere.

Remember to minimize system load while working through these exercises. If you
see a deviation in your datapoint, stop and confirm your system assumptions before
continuing.
