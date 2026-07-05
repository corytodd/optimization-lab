# Appendix: Does the CPU Governor Actually Matter?

While collecting the LUT numbers in Chapter 3, one `hyperfine` run was captured while the
laptop had silently dropped into the `powersave` cpufreq governor. That run reported a mean of
0.580 s; a later, correctly-captured `performance` run reported 0.886 s for the same binary.
Taken at face value, `powersave` looked *faster*. This is good time to sanity check out process.

## Method

A single before/after comparison can't separate a real governor effect from ordinary
session-to-session noise. Instead, [`tools/governor-experiment.sh`](../../tools/governor-experiment.sh)
alternates the governor in an A-B-B-A pattern running the same LUT workload under each.
If the governor is the true cause, timings should cluster by governor regardless of order.
If it's session noise, timings will drift with time and ignore the governor label.

```bash
./tools/governor-experiment.sh
# writes results/governor_experiment.csv
```

## Results

Four samples per governor (see [`results/governor_experiment.csv`](../../results/governor_experiment.csv)):

| Governor | mean (avg) | mean (stdev) | user (avg) | system (avg) |
|---|---|---|---|---|
| `performance` | 0.588 s | 0.0046 s | 0.286 s | 0.298 s |
| `powersave` | 0.588 s | 0.0035 s | 0.286 s | 0.298 s |

The two governors are indistinguishable. The gap between them is smaller than one standard
deviation. The original 0.580 s vs. 0.886 s split was session noise, not a governor effect
The `performance` and `powersave` samples interleave throughout the run and land on the same
numbers regardless of position, which rules out drift as an explanation too.

## Takeaway

A single `perf stat` or `hyperfine` capture reflects the whole machine's state at that moment,
not just the code being measured. `governor: <value>` is captured in every `run-perf.sh` output
for exactly this reason. It makes a plausible confounder visible instead of silent. When a
result looks surprising, the fix isn't to rationalize it; it's to design a comparison that can
actually falsify the suspect explanation, as above.
