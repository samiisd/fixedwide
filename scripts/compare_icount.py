#!/usr/bin/env python3
"""Gate a run of scripts/icount.sh against a committed baseline.

Instruction counts are deterministic, so the tolerance is tight on purpose: a
1% move is a real change in the generated code, not noise. Exit status is
non-zero if any workload regressed, which is what makes this usable as a
required CI check.

    compare_icount.py baseline.csv current.csv [--tolerance 1.0]
"""
import argparse
import csv
import os
import sys

DEFAULT_TOLERANCE = 1.0


def load(path):
    with open(path, newline="") as handle:
        return {row["workload"]: float(row["instructions_per_op"])
                for row in csv.DictReader(handle)}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("baseline")
    parser.add_argument("current")
    parser.add_argument("--tolerance", type=float, default=DEFAULT_TOLERANCE,
                        help="percent a workload may grow before it fails")
    args = parser.parse_args()

    baseline, current = load(args.baseline), load(args.current)

    rows, failures = [], []
    for workload in sorted(baseline.keys() | current.keys()):
        before, after = baseline.get(workload), current.get(workload)
        if before is None:
            # A new workload has nothing to regress against. Report it so the
            # baseline gets updated, but do not fail the build on it.
            rows.append((workload, "-", f"{after:.1f}", "new", "NEW"))
            continue
        if after is None:
            rows.append((workload, f"{before:.1f}", "-", "removed", "REMOVED"))
            continue
        delta = (after - before) / before * 100.0 if before else 0.0
        verdict = "FAIL" if delta > args.tolerance else "PASS"
        if verdict == "FAIL":
            failures.append((workload, before, after, delta))
        rows.append((workload, f"{before:.1f}", f"{after:.1f}", f"{delta:+.2f}%", verdict))

    width = max(len(row[0]) for row in rows)
    print(f"{'workload'.ljust(width)}  {'base':>10} {'current':>10} {'delta':>9}  verdict")
    for workload, before, after, delta, verdict in rows:
        print(f"{workload.ljust(width)}  {before:>10} {after:>10} {delta:>9}  {verdict}")

    # GitHub renders this in the job summary, so a failure is readable without
    # opening the log.
    summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary:
        with open(summary, "a") as handle:
            handle.write(f"### Instruction counts (tolerance {args.tolerance}%)\n\n")
            handle.write("| workload | baseline | current | delta | |\n")
            handle.write("|---|---:|---:|---:|---|\n")
            for workload, before, after, delta, verdict in rows:
                mark = {"PASS": "ok", "FAIL": "**REGRESSION**",
                        "NEW": "new", "REMOVED": "removed"}[verdict]
                handle.write(f"| `{workload}` | {before} | {after} | {delta} | {mark} |\n")

    if failures:
        print(f"\n{len(failures)} workload(s) regressed by more than {args.tolerance}%:",
              file=sys.stderr)
        for workload, before, after, delta in failures:
            print(f"  {workload}: {before:.1f} -> {after:.1f} ({delta:+.2f}%)", file=sys.stderr)
        print("\nIf the change is intentional, re-record with "
              "scripts/icount.sh --update and say why in the commit message.",
              file=sys.stderr)
        return 1

    print(f"\nall {len(rows)} workloads within {args.tolerance}%")
    return 0


if __name__ == "__main__":
    sys.exit(main())
