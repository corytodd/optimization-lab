#!/usr/bin/env python3
# Generates deterministic pseudo-random ASCII test data for benchmarking.
# A fixed seed ensures identical output across machines while avoiding the
# periodic repetition of yes(1), which would skew branch predictor results.

import argparse
import random
import sys

CHARS = (
    "abcdefghijklmnopqrstuvwxyz" * 5
    + "ABCDEFGHIJKLMNOPQRSTUVWXYZ" * 3
    + "0123456789 \n.,!?" * 2
).encode()


def main():
    parser = argparse.ArgumentParser(description="Generate deterministic benchmark data")
    parser.add_argument("output", help="output file path")
    parser.add_argument("--size", type=int, default=1_000_000_000, help="output size in bytes (default: 1000000000)")
    parser.add_argument("--seed", type=int, default=42, help="PRNG seed (default: 42)")
    args = parser.parse_args()

    rng = random.Random(args.seed)
    chunk = bytes(rng.choices(CHARS, k=65536))

    with open(args.output, "wb") as f:
        written = 0
        while written < args.size:
            n = min(len(chunk), args.size - written)
            f.write(chunk[:n])
            written += n

    print(f"Generated {args.output} ({args.size / 1e6:.0f} MB, seed={args.seed})")


if __name__ == "__main__":
    sys.exit(main())
