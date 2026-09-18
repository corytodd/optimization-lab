## Stdout and Measurement Noise

The binary writes its result to stdout, which adds cost that has nothing to do
with the algorithm. Redirecting to `/dev/null` eliminates the kernel-side
`write()` cost, but the user-space work, buffering and copying the output into
stdio's internal buffer, still runs and still appears in `perf stat`'s `:u`
counters. To truly isolate the computation, suppress output entirely with
`--bench`. The `:u` suffix on perf events is a useful reminder. It only counts
user-space work, so stdout overhead shows up there regardless of where the
output goes.

### Our First Datapoint

Compare the instruction counts with and without `--bench`. Without it, `fputs`
copies 1 GB through stdio's internal buffer, a user-space memcpy that adds
millions of instructions to the `:u` counter, none of which belong to the
algorithm. With `--bench` those instructions disappear and the counter reflects
only `rot13_process` and its supporting work.

> [!TIP]
> This is not an optimization, the binary is not performing its work any more
> efficiently. This is measurement hygiene.

Making sure the numbers describe the
thing being studied. Every benchmark involves this kind of scoping decision.
Getting it wrong doesn't corrupt the result catastrophically, but it adds noise
that can obscure real differences between implementations, especially when those
differences are small.
