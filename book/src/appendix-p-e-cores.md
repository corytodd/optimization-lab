# Appendix: What are P and E Cores?

During the setup chapter, caution was advised to the reader to be aware
of their CPU topology. Specifically, P and E cores happen to have a
meaningful impact on performance, particularly when work migrates
between them. Running the application on one type, then having the
scheduler move the work to another can skew the results.

So what are they?

On newer Intel cores, starting on Gen12, hybrid topologies were introduced.
Some cores are optimized for high performance and other are optimized for
efficiency. Operating systems can choose the best core for the job for the
purpose of balancing battery life and performance.

In Chapter 1, the following CPU topology table was shown.

```bash
lscpu -e=CPU,CORE,MAXMHZ
CPU CORE    MAXMHZ
  0    0 4700.0000
  1    0 4700.0000
  2    1 4700.0000
  3    1 4700.0000 # End of P-cores
  4    2 3500.0000 # Start of E-cores
  5    3 3500.0000
  6    4 3500.0000
  7    5 3500.0000
  8    6 3500.0000
  9    7 3500.0000
 10    8 3500.0000
 11    9 3500.0000
```

This tells us my CPU has 4 P cores capable of 4.7 GHz and 8 E cores capable of
3.5 GHz.

Prefer `lscpu -e` over reading `/sys/devices/system/cpu/cpu*/cpufreq/*` directly with a
shell glob. The glob expands in lexical order, so once a machine has 10 or more CPUs the
listing comes out `cpu0, cpu1, cpu10, cpu11, cpu2, ...` rather than numeric order. The
"corresponds to core0-coreN" assumption silently breaks and you'll misattribute frequencies
to the wrong CPU number. `lscpu -e` reports the same sysfs-backed topology but sorted
numerically by CPU, so `CPU` lines up correctly with `CORE` and `MAXMHZ`.
