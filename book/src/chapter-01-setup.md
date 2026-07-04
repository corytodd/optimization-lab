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
# Corresponds to core0-coreN where N is nproc
sys/devices/system/cpu/cpu*/cpufreq/base_frequency
1700000
1700000
1700000
1700000 # End of P-cores
1200000 # Start of E-cores
1200000
1200000
1200000
1200000
1200000
1200000
1200000
```

Remember to minimize system load while working through these exercises. If you
see a deviation in your datapoint, stop and confirm your system assumptions before
continuing.
