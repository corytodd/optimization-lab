#!/bin/bash
# Alternates the cpufreq governor in an A-B-B-A pattern and times the same
# workload under each, to separate a genuine governor effect from ordinary
# session-to-session noise.
# Requires sudo to change the governor; restores the original governor on exit.
#
# Usage: ./tools/governor-experiment.sh [-- TARGET ARGS...]
#   default target: ./build/cmd/rot13-cli -f data/data_1GB.txt --bench --impl lut

set -euo pipefail

OUT_CSV="results/governor_experiment.csv"
TARGET=(./build/cmd/rot13-cli -f data/data_1GB.txt --bench --impl lut)

if [[ "${1:-}" == "--" ]]; then
    shift
    TARGET=("$@")
fi

ORIG_GOV=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)
echo "original governor: ${ORIG_GOV}" >&2

cleanup() {
    echo "restoring governor to ${ORIG_GOV}" >&2
    sudo cpupower frequency-set -g "${ORIG_GOV}" >/dev/null
}
trap cleanup EXIT

# Prime a sudo credential up front so the loop isn't interrupted by prompts.
sudo -v

mkdir -p "$(dirname "${OUT_CSV}")"
echo "round,governor,mean_s,user_s,system_s" > "${OUT_CSV}"

# Two full A-B-B-A cycles = 4 samples per governor. The alternating order
# cancels any monotonic drift over the run instead of just a before/after split.
ORDER=(performance powersave powersave performance powersave performance performance powersave)

for i in "${!ORDER[@]}"; do
    gov="${ORDER[$i]}"
    echo "=== round $((i + 1))/${#ORDER[@]}: governor=${gov} ===" >&2
    sudo cpupower frequency-set -g "${gov}" >/dev/null
    sleep 1 # let the frequency settle

    json=$(mktemp)
    hyperfine --warmup 3 --runs 8 --export-json "${json}" \
        "${TARGET[*]}" >&2

    python3 -c "
import json
d = json.load(open('${json}'))['results'][0]
print(f'${i},${gov},{d[\"mean\"]},{d[\"user\"]},{d[\"system\"]}')
" >> "${OUT_CSV}"
    rm -f "${json}"
done

echo "results written to ${OUT_CSV}" >&2
cat "${OUT_CSV}"
