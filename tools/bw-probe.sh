#!/bin/bash
# Measures achievable memory copy bandwidth with perf bench and prints the rot13 speed-of-light.

set -euo pipefail

SIZE=${SIZE:-1GB}                  # buffer size perf bench copies
LOOPS=${LOOPS:-5}                  # perf bench iterations
CORES=${TASKSET_CORES:-0-3}
INPUT_BYTES=${INPUT_BYTES:-$((1 << 30))}   # rot13 input size the speed-of-light is computed for

bench() {
    taskset -c "$CORES" perf bench --format=simple mem memcpy -s "$SIZE" -l "$LOOPS" -f "$1" |
        grep -v '^#'
}

nt=$(bench default)

awk -v nt="$nt" -v bytes="$INPUT_BYTES" 'BEGIN {
    printf "memcpy bandwidth:      %5.1f GB/s copied  (%.1f GB/s read+write)\n", nt / 1e9, 2 * nt / 1e9
    printf "rot13 speed-of-light:  %5.1f ms  (%d MiB input)\n", bytes / nt * 1e3, bytes / 1048576
}'