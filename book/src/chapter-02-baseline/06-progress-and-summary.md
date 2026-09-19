## Progress Chart

`perf stat` runs the command once with no warmup, so its wall-clock time is
noisy. Use `hyperfine` instead because it does warmup runs and reports mean,
stddev, and min across many iterations. This makes it possible to reason about
small improvements. Export to JSON so `plot-results.py` can pick it up
automatically. Name each file `<label>_hyperfine.json`:

```bash
hyperfine --warmup 3 --export-json results/baseline.json \
  './build/cmd/rot13-cli -f data/data_1GB.txt --bench'
# --sol is speed-of-light from bw-probe.sh output
python3 tools/plot-results.py --sol 93.6 --results results/ --out results/baseline_chart.svg
```

![Baseline progress chart](../results/baseline_chart.svg)

This is a good time for a refresher on different types of "time" For our
purposes, we care about total time, that's the wall-clock execution time of the
algorithm from program start to end. This is composed of two time slices: user
time, that's the code we directly control, and system time, that's kernel code
that we indirectly control. Our optimization process will address both slices to
treat the overall execution time.

## Summary

We have established our speed-of-light and our scalar baseline. On this system
we see a gap of roughly 10x. System-specifics like P-core affinity, CPU
governor, and compiler version are captured in the metadata header written by
`run-perf.sh`, so every result is tied to a known environment and regressions
are traceable to a specific change.

From here, each optimization attempt follows the same loop.
