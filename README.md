# Optimization Lab

A worked example of the low-level optimization workflow using `rot13` as the subject:
profiling with `perf`, establishing a speed-of-light floor, and iterating toward it.
The algorithm stays simple so the tooling and methodology stay in focus.

**Read the book:** [corytodd.github.io/optimization-lab](https://corytodd.github.io/optimization-lab)

## Quick Start

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Benchmark
cmake --build build
./tools/gen-data.py data/data_1GB.txt
./tools/run-perf.sh -o results/baseline_perf.txt -- ./build/cmd/rot13-cli -f data/data_1GB.txt --bench
```

## Developers

## Quality

The sample code can be built, tested, and run with the following:

```bash
# check formatting
cmake --build build --target format

# apply fixes in place
cmake --build build --target format-fix

# run clang-tidy
cmake --build build --target tidy

# Testing
cmake --build build && ctest --test-dir build --output-on-failure
```

The book can be rendered locally with [mdbook](https://rust-lang.github.io/mdBook/guide/installation.html).

```
mdbook serve book
```

## Licenses

Source code (`src/`, `include/`, `cmd/`, `test/`, `tools/`): [MIT](LICENSE).  
Documentation (`book/`): [CC BY-NC-ND 4.0](LICENSE-DOCS).
