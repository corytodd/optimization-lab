#!/bin/bash
# Usage: ./tools/run-perf.sh [run-perf options] -- TARGET [TARGET ARGS...]
# The -- separates run-perf options from the target binary and its arguments.
# perf stat also uses -- as its own separator, so it is passed straight through.
#
# Options:
#   -o, --output FILE   write perf stat output to FILE (default: perf-out/<timestamp>.txt)

set -euo pipefail

OUTPUT=""

while [[ $# -gt 0 && "$1" != "--" ]]; do
    case "$1" in
        -o|--output) OUTPUT="$2"; shift 2 ;;
        *) echo "unknown option: $1" >&2; exit 1 ;;
    esac
done
shift # consume --

mkdir -p "$(dirname "${OUTPUT:-perf-out/x}")"
OUTPUT="${OUTPUT:-perf-out/$(date +%Y%m%d-%H%M%S).txt}"

EVENTS=cycles,instructions
EVENTS+=,mem_load_retired.l1_miss,mem_load_retired.l2_miss,mem_load_retired.l3_miss
EVENTS+=,dtlb-load-misses,dtlb-loads
EVENTS+=,page-faults
EVENTS+=,branches,branch-misses
EVENTS+=,uops_issued.any,uops_retired.slots,stalled-cycles-frontend,stalled-cycles-backend

# Write metadata header before perf appends its output
PERF_TMP=$(mktemp)
taskset -c "${TASKSET_CORES:-0-3}" \
    perf stat -e "$EVENTS" -o "$PERF_TMP" \
    "$@"

{
    echo "## environment"
    echo "date:     $(date -u +"%Y-%m-%dT%H:%M:%SZ")"
    echo "os:       $(uname -sr)"
    echo "cpu:      $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | xargs)"
    echo "cores:    ${TASKSET_CORES:-0-3}"
    echo "compiler: $(clang --version 2>/dev/null | head -1 || cc --version | head -1)"
    echo "governor: $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo unknown)"
    echo "command:  $*"
    echo ""
    cat "$PERF_TMP"
} > "$OUTPUT"

rm "$PERF_TMP"
echo "perf output: ${OUTPUT}"
