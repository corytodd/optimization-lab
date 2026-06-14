# Setup

## Tools

- **clang** 22.1.6 (LLVM toolchain preferred; GCC works but SIMD intrinsics may differ)
- **cmake** 4.3.3
- **python** 3.x
- **python-matplotlib** Optional, for recreating charts
- **perf** 7.0.10-1 hardware performance counter collection and call-graph sampling
- **hyperfine** v1.20.0 wall-clock benchmarking with warmup and statistical summary
- **stackcollapse-perf** + **flamegraph** from [Brendan Gregg's FlameGraph repo](https://github.com/brendangregg/FlameGraph), required by `flame.sh`

## Building

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Benchmark
cmake --build build
ctest --test-dir build --output-on-failure
```

Build in Benchmark mode, which compiles at `-O3` with debug info retained for profiling.

## Test Data

```bash
./tools/gen-data.py data/data_1GB.txt
```
