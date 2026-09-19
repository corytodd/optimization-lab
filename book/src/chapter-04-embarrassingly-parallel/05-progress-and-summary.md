## Progress Chart

```bash
hyperfine --warmup 3 --export-json results/simd_hyperfine.json \
  './build/cmd/rot13-cli -f data/data_1GB.txt --bench --impl simd'
python3 tools/plot-results.py --sol 93.6 --results results/ --out results/simd_chart.svg
```

![Progress chart](../results/simd_chart.svg)

Over 10 warmed-up runs:

- `baseline_hyperfine.json`: mean 2.448 s (user 2.079 s, system 0.347 s)
- `lut_hyperfine.json`: mean 0.886 s (user 0.295 s, system 0.587 s)
- `simd_hyperfine.json`: mean 0.444 s (user 0.119 s, system 0.321 s)

User time drops another 2.5x from the LUT build (17.5x from baseline),
consistent with the instruction and cycle counts above. System time drops too,
in absolute terms (0.587 s -> 0.321 s), but keeps claiming a larger share of the
total. 72% of wall-clock time now, up from the LUT's 66% and the baseline's 14%.
The algorithm has gotten fast enough that the surrounding cost of getting bytes
into and out of the process is now the largest single line item, not the
transform itself.

## Summary

Against the speed-of-light floor from Chapter 2, each step has closed most of
the remaining gap:

- baseline at 23x the floor
- LUT at 8.3x
- SIMD at 4.2x.

The compute side of `rot13` is now a small fraction of total time. `system`
time, driven by I/O and memory setup rather than the transform itself, is the
largest remaining cost and the next place to look.
