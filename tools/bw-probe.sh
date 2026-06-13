#!/bin/bash
# Measures achievable memory read+write bandwidth and prints the rot13 speed-of-light.

set -euo pipefail

BIN=$(mktemp)
trap 'rm -f "$BIN"' EXIT

clang -O3 -o "$BIN" "$(dirname "$0")/bw-probe.c"
taskset -c "${TASKSET_CORES:-0-3}" "$BIN"
