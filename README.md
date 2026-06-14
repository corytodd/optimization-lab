# Optimization Lab

A worked example of the low-level optimization workflow using `rot13` as the subject:
profiling with `perf`, establishing a speed-of-light floor, and iterating toward it with
SIMD. The algorithm stays simple so the tooling and methodology stay in focus.

**Read the book:** [corytodd.github.io/optimization-demo](https://corytodd.github.io/optimization-demo)

## Quick Start

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Benchmark
cmake --build build
./tools/gen-data.py data/data_1GB.txt
./tools/run-perf.sh -o results/baseline_perf.txt -- ./build/cmd/rot13-cli -f data/data_1GB.txt --bench
```

## Licenses

Source code (`src/`, `include/`, `cmd/`, `test/`, `tools/`): [MIT](LICENSE).
Documentation (`book/`): [CC BY-NC-ND 4.0](LICENSE-DOCS).
