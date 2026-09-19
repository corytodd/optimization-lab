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
memcpy bandwidth:       11.5 GB/s copied  (22.9 GB/s read+write)
rot13 speed-of-light:   93.6 ms  (1024 MiB input)
```

93.6 ms is the floor. `rot13` must read and write 2GB in total by
1GB read + 1GB write. The throughput is 22.9 GB/S so the math becomes:

```
speed_of_light = 2GB / 22.9GB/s ~= 93.6 ms
```

Any implementation that processes 1 GB slower than this is leaving
performance on the table; any implementation that matches it has
extracted everything the hardware can give.
