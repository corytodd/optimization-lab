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

In Chapter 1, the following CPU frequency table was shown.

```bash
# Corresponds to core0-coreN where N is nproc
cat /sys/devices/system/cpu/cpu*/cpufreq/base_frequency
1700000
1700000
1700000
1700000 # End of P-cores
1200000 # Start of E-cores
1200000
1200000
1200000
1200000
1200000
1200000
1200000
```

This tells us my CPU has 4 P cores with a minimum frequency of 1.7 GHz and 8 E
cores with a minimum frquency of 1.2 GHz.

```bash
cat /sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_max_freq | uniq 
4700000
3500000
```

Is a more honest telling of what the system is capable of. 4.7 GHz on P cores
and 3.5 GHz on E cores.
