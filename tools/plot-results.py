#!/usr/bin/env python3
"""
Renders a horizontal bar chart of rot13 implementation timings.
Scans a results directory for *_hyperfine.json files produced by hyperfine --export-json,
extracts the mean elapsed time from each, and plots them sorted slowest-first.

Usage:
  python3 tools/plot-results.py --sol 106.3 --results results/ [--out results/chart.svg]
"""

import argparse
import json
import sys
from pathlib import Path

try:
    import matplotlib.pyplot as plt
    import matplotlib.ticker as ticker
except ImportError:
    sys.exit("matplotlib is required: sudo pacman -S python-matplotlib")


def load_results(results_dir):
    results = []
    for path in sorted(Path(results_dir).glob("*_hyperfine.json")):
        text = path.read_text().strip()
        if not text:
            print(f"warning: skipping empty file {path.name}", file=sys.stderr)
            continue
        try:
            mean_s = json.loads(text)["results"][0]["mean"]
        except (json.JSONDecodeError, KeyError, IndexError):
            print(f"warning: unexpected format in {path.name}", file=sys.stderr)
            continue
        label = path.stem.removesuffix("_hyperfine")
        results.append((label, mean_s * 1000.0))
    return sorted(results, key=lambda r: r[1], reverse=True)


def main():
    parser = argparse.ArgumentParser(description="Plot rot13 benchmark results")
    parser.add_argument("--sol",     type=float, required=True, help="speed-of-light in ms (from bw-probe.sh)")
    parser.add_argument("--results", default="results",         help="directory containing *_hyperfine.json files (default: results/)")
    parser.add_argument("--out",     default="results/chart.svg", help="output path (default: results/chart.svg)")
    args = parser.parse_args()

    results = load_results(args.results)
    if not results:
        sys.exit(f"no *_hyperfine.json files found in {args.results}")

    labels = [r[0] for r in results]
    times  = [r[1] for r in results]

    fig, ax = plt.subplots(figsize=(9, max(2, 0.6 * len(results) + 1.2)))

    bars = ax.barh(labels, times, color="#4C8EDA", height=0.5, zorder=2)

    ax.axvline(args.sol, color="#CC2222", linestyle="--", linewidth=1.2, zorder=3)
    ax.text(args.sol + max(times) * 0.005, 0.5,
            f"speed of light\n{args.sol:.1f} ms",
            color="#CC2222", fontsize=8, va="center", ha="left",
            transform=ax.get_xaxis_transform(),
            bbox=dict(boxstyle="round,pad=0.3", facecolor="white", edgecolor="none", alpha=0.85),
            zorder=5)

    for bar, ms in zip(bars, times):
        ax.text(bar.get_width() + max(times) * 0.01, bar.get_y() + bar.get_height() / 2,
                f"{ms:.1f} ms", va="center", fontsize=9)

    ax.set_xlabel("time ms for 1 GB input")
    ax.set_xlim(0, max(times) * 1.15)
    ax.xaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f"{x:.0f}"))
    ax.invert_yaxis()
    ax.grid(axis="x", linestyle=":", alpha=0.4, zorder=1)
    ax.spines[["top", "right"]].set_visible(False)

    fig.tight_layout()
    fig.savefig(args.out, format="svg")
    print(f"chart written: {args.out}")


if __name__ == "__main__":
    sys.exit(main())
