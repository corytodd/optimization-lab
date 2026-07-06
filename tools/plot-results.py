#!/usr/bin/env python3
"""
Renders a stacked horizontal bar chart of rot13 implementation timings.
Scans a results directory for *_hyperfine.json files produced by hyperfine --export-json,
extracts the user/system time from each, and plots them stacked, sorted slowest-first.

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

# Categorical slots 1 (blue) and 2 (aqua), in the palette's fixed adjacent
# order -- chosen together for maximum colorblind-safe separation.
USER_COLOR = "#2a78d6"
SYSTEM_COLOR = "#1baf7a"
SOL_COLOR = "#CC2222"
GAP_COLOR = "white"  # surface-color gap between stacked segments


def load_results(results_dir):
    results = []
    for path in sorted(Path(results_dir).glob("*_hyperfine.json")):
        text = path.read_text().strip()
        if not text:
            print(f"warning: skipping empty file {path.name}", file=sys.stderr)
            continue
        try:
            entry = json.loads(text)["results"][0]
            user_s = entry["user"]
            system_s = entry["system"]
        except (json.JSONDecodeError, KeyError, IndexError):
            print(f"warning: unexpected format in {path.name}", file=sys.stderr)
            continue
        label = path.stem.removesuffix("_hyperfine")
        results.append((label, user_s * 1000.0, system_s * 1000.0))
    return sorted(results, key=lambda r: r[1] + r[2], reverse=True)


def main():
    parser = argparse.ArgumentParser(description="Plot rot13 benchmark results")
    parser.add_argument("--sol",     type=float, required=True, help="speed-of-light in ms (from bw-probe.sh)")
    parser.add_argument("--results", default="results",         help="directory containing *_hyperfine.json files (default: results/)")
    parser.add_argument("--out",     default="results/chart.svg", help="output path (default: results/chart.svg)")
    args = parser.parse_args()

    results = load_results(args.results)
    if not results:
        sys.exit(f"no *_hyperfine.json files found in {args.results}")

    labels  = [r[0] for r in results]
    user_ms = [r[1] for r in results]
    sys_ms  = [r[2] for r in results]
    totals  = [u + s for u, s in zip(user_ms, sys_ms)]

    fig, ax = plt.subplots(figsize=(9, max(2, 0.6 * len(results) + 1.2)))

    # A thin surface-color edge on each segment reads as the 2px gap that
    # separates touching marks, without adding a data-weight border stroke.
    user_bars = ax.barh(labels, user_ms, color=USER_COLOR, height=0.5,
                         edgecolor=GAP_COLOR, linewidth=1.5, zorder=2, label="user")
    sys_bars = ax.barh(labels, sys_ms, left=user_ms, color=SYSTEM_COLOR, height=0.5,
                        edgecolor=GAP_COLOR, linewidth=1.5, zorder=2, label="system")

    ax.axvline(args.sol, color=SOL_COLOR, linestyle="--", linewidth=1.2, zorder=3)
    ax.text(args.sol + max(totals) * 0.005, 0.5,
            f"speed of light\n{args.sol:.1f} ms",
            color=SOL_COLOR, fontsize=8, va="center", ha="left",
            transform=ax.get_xaxis_transform(),
            bbox=dict(boxstyle="round,pad=0.3", facecolor="white", edgecolor="none", alpha=0.85),
            zorder=5)

    for bar, total in zip(user_bars, totals):
        ax.text(bar.get_x() + total + max(totals) * 0.01, bar.get_y() + bar.get_height() / 2,
                f"{total:.1f} ms", va="center", fontsize=9)

    ax.set_xlabel("time ms for 1 GB input (stacked: user + system)")
    ax.set_xlim(0, max(totals) * 1.15)
    ax.xaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f"{x:.0f}"))
    ax.invert_yaxis()
    ax.grid(axis="x", linestyle=":", alpha=0.4, zorder=1)
    ax.spines[["top", "right"]].set_visible(False)
    ax.legend(loc="lower right", frameon=False, fontsize=9)

    fig.tight_layout()
    fig.savefig(args.out, format="svg")
    print(f"chart written: {args.out}")


if __name__ == "__main__":
    sys.exit(main())
