## Flame Graph

A flame graph is a visualization of sampled call stacks where each box is a
function and its width is proportional to how often it appeared in the samples,
making hot code paths easy to spot at a glance.

To produce a flame graph and get per-instruction annotation:

```bash
./tools/flame.sh -n baseline --title "The most boring of Flame Graphs" -- ./build/cmd/rot13-cli -f data/data_1GB.txt --bench
# produces results/baseline_flame.svg and perf-out/baseline.data
# per-instruction breakdown (--stdio avoids libcapstone requirement):
# perf annotate --stdio -i perf-out/baseline.data
```


![Baseline flame graph](../results/baseline_flame.svg)

For a tight single-function loop like this, the flame graph will show
`rot13_process` dominating unconditionally. The interesting question is _which
instructions within it_ are slow. This is what `perf annotate --stdio` answers.
It interleaves source lines with sampled instruction counts, showing where
cycles are going inside the function.
