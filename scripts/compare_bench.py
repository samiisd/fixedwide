#!/usr/bin/env python3
"""Aggregate paired rounding-benchmark CSVs into per-row medians and deltas.

Median of CPU nanoseconds per operation over every sample for a
(workload, rounding mode) pair. No averaging across categories: one row in,
one row out, so a slow division can never be hidden by a fast addition.
"""
import csv, statistics, sys
from collections import defaultdict


def load(path):
    samples = defaultdict(list)
    with open(path, newline="") as fh:
        for row in csv.DictReader(fh):
            samples[(row["workload"], row["mode"])].append(float(row["cpu_ns_per_op"]))
    return samples


def category(workload):
    if workload.startswith(("wide_product", "wide_by", "native_by")):
        return "wide"
    if "parse" in workload or "format" in workload:
        return "text"
    if workload.startswith("chain"):
        return "chain"
    return "common"


# Regression gate per category, from the release acceptance checklist.
LIMIT = {"common": 3.0, "chain": 5.0, "wide": 5.0, "text": 5.0}


def main(baseline_path, current_path, label):
    base, curr = load(baseline_path), load(current_path)
    out = csv.writer(sys.stdout)
    out.writerow(["label", "workload", "mode", "category", "n_base", "n_curr",
                  "baseline_ns", "current_ns", "delta_pct", "limit_pct", "verdict"])
    worst = 0.0
    failures = 0
    for key in sorted(base.keys() & curr.keys()):
        workload, mode = key
        b, c = statistics.median(base[key]), statistics.median(curr[key])
        delta = (c - b) / b * 100.0
        cat = category(workload)
        limit = LIMIT[cat]
        verdict = "PASS" if delta <= limit else "FAIL"
        failures += verdict == "FAIL"
        worst = max(worst, delta)
        out.writerow([label, workload, mode, cat, len(base[key]), len(curr[key]),
                      f"{b:.4f}", f"{c:.4f}", f"{delta:+.2f}", f"{limit:.1f}", verdict])
    missing = (base.keys() ^ curr.keys())
    for key in sorted(missing):
        out.writerow([label, key[0], key[1], category(key[0]), "", "", "", "", "", "", "MISSING"])
    print(f"# {label}: {failures} failing rows of {len(base.keys() & curr.keys())}, "
          f"worst {worst:+.2f}%", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:4]))
