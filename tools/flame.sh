#!/bin/bash
# Produces <name>_flame.svg and a perf report for intra-function annotation.
# Requires a Benchmark build (-fno-omit-frame-pointer) for frame-pointer unwinding.
#
# Usage: ./tools/flame.sh [flame options] -- TARGET [TARGET ARGS...]
#   -n NAME       label prefix; produces results/<name>_flame.svg and perf-out/<name>.data
#   -o DIR        directory for perf.data (default: perf-out)
#   -s FILE       explicit output path for flame.svg (default: results/<name>_flame.svg)
#   --title TEXT  title text embedded in the flame graph SVG

set -euo pipefail

NAME="baseline"
OUT_DIR="perf-out"
FLAME_SVG=""
TITLE=""

while [[ $# -gt 0 && "$1" != "--" ]]; do
    case "$1" in
        -n) NAME="$2"; shift 2 ;;
        -o) OUT_DIR="$2"; shift 2 ;;
        -s) FLAME_SVG="$2"; shift 2 ;;
        --title) TITLE="$2"; shift 2 ;;
        *)  echo "unknown option: $1" >&2; exit 1 ;;
    esac
done
shift # consume --

FLAME_SVG="${FLAME_SVG:-results/${NAME}_flame.svg}"
PERF_DATA="${OUT_DIR}/${NAME}.data"

mkdir -p "${OUT_DIR}" "$(dirname "${FLAME_SVG}")"

taskset -c "${TASKSET_CORES:-0-3}" \
    perf record -g --call-graph fp -o "${PERF_DATA}" \
    "$@"

FLAMEGRAPH_ARGS=()
if [[ -n "${TITLE}" ]]; then
    FLAMEGRAPH_ARGS+=(--title "${TITLE}")
fi

perf script -i "${PERF_DATA}" \
    | /usr/bin/stackcollapse-perf \
    | /usr/bin/flamegraph "${FLAMEGRAPH_ARGS[@]}" > "${FLAME_SVG}"

echo "flame graph: ${FLAME_SVG}"
echo ""
echo "For per-instruction hotspots:"
echo "  perf report -i ${PERF_DATA}"
echo "  perf annotate --stdio -i ${PERF_DATA}"
