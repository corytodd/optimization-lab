# The Baseline

## Speed of Light

From our definition of optimization, we know that the theoretical limits must
first be established. For applications, there are two types of work that can be
optimized.

- Instruction execution: The count and type of instructions executed. This
  includes arithmetic and pipelining.
- Data transfer: Parameters to instruction execution moving to and from storage.
  This includes network access, local storage, system memory, and process memory.

For `rot13`, the work is to process data using a transform function. It reads
every input byte once and writes every output byte once. We know the input will
come from a user which means data is at best sourced from RAM. This makes the
application memory-bandwidth-bound. No amount of CPU optimization can make it
faster than the rate at which memory can supply and absorb data.

Measure the achievable read+write bandwidth on your machine:

```bash
./tools/bw-probe.sh
```

```
best memcpy 1 GB:          0.106 s
read+write bandwidth:      20.2 GB/s
rot13 speed-of-light:      106.3 ms  (1 GB input, 1 read + 1 write)
```

106.3 ms is the floor. Any implementation that processes 1 GB slower than that
is leaving performance on the table; any implementation that matches it has
extracted everything the hardware can give.
