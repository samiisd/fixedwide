#!/usr/bin/env python3
"""Compare two checkouts with the same oracle-backed rounding benchmark.

No timing threshold is treated as a correctness or CI gate. The existing
instruction-count gate is deliberately unchanged. Retains all samples, metadata,
and the derived scale-8 driver; scale-12's historical driver is never modified.
"""
import argparse
import csv
import hashlib
import io
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess


def run(args, **kwargs):
    return subprocess.run([str(arg) for arg in args], check=True, text=True, **kwargs)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("base", type=Path)
    parser.add_argument("head", type=Path)
    parser.add_argument("--output", type=Path, default=Path("fastpath-results"))
    parser.add_argument("--compiler", default="g++-14")
    parser.add_argument("--icount", action="store_true", help="also compare all existing Callgrind workloads")
    parser.add_argument("--rounds", type=int, default=4)
    parser.add_argument("--iterations", type=int, default=262144)
    args = parser.parse_args()
    if args.rounds < 2 or args.rounds % 2:
        parser.error("--rounds must be positive, even, and at least 2")
    if not 4096 <= args.iterations <= 16777216:
        parser.error("--iterations must be in [4096, 16777216]")
    roots = {"base": args.base.resolve(), "head": args.head.resolve()}
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    # Restrict only this process/children, never the machine's governor or CPUs.
    cpu = None
    if hasattr(os, "sched_getaffinity"):
        cpu = min(os.sched_getaffinity(0))
        os.sched_setaffinity(0, {cpu})

    original = (roots["head"] / "benchmarks/rounding_bench.cpp").read_text()
    # The compatibility mul_wide adapter is specifically scale 12. Do not
    # mislabel a scale-8 multiplication as that adapter: omit its two rows.
    start = original.index("void wide_output_product(bool full_range)")
    end = original.index("int main(", start)
    derived = original[:start] + original[end:]
    calls = "    wide_output_product(false); wide_output_product(true);\n"
    if derived.count(calls) != 1:
        raise RuntimeError("rounding benchmark layout changed; review scale-8 derivation")
    derived = derived.replace(calls, "")
    substitutions = {
        "fw::FP64": "fw::Fixed64<8>",
        "fw::FP128": "fw::Fixed128<8>",
        "fw::fractional_digits": "8u",
        "fw::scale": "fw::Fixed64<8>::scale()",
        "fw::parse64": "fw::parse<fw::Fixed64<8>>",
        "fw::parse128": "fw::parse<fw::Fixed128<8>>",
    }
    for before, after in substitutions.items():
        if before not in derived:
            raise RuntimeError(f"benchmark layout changed: missing {before}")
        derived = derived.replace(before, after)
    drivers = {8: output / "rounding_scale8.cpp", 12: output / "rounding_scale12.cpp"}
    drivers[8].write_text(derived)
    drivers[12].write_text(original)
    flags = ["-O3", "-std=c++23", "-fno-tree-vectorize", "-fno-tree-slp-vectorize", "-ffp-contract=off"]
    versions = {}
    for name, root in roots.items():
        versions[name] = run(["git", "-C", root, "rev-parse", "HEAD"], capture_output=True).stdout.strip()
        build = output / ("build-" + name)
        run(["cmake", "-S", root, "-B", build, "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release",
             f"-DCMAKE_CXX_COMPILER={args.compiler}", "-DFIXEDWIDE_BUILD_TESTS=OFF",
             "-DFIXEDWIDE_BUILD_EXAMPLES=OFF", "-DFIXEDWIDE_BUILD_ORACLE_TESTS=OFF",
             "-DFIXEDWIDE_BUILD_BENCHMARKS=OFF", "-DBUILD_SHARED_LIBS=OFF"])
        run(["cmake", "--build", build, "--parallel", "2"])
        for digits, source in drivers.items():
            run([args.compiler, *flags, f"-I{root / 'include'}", source,
                 build / "libfixedwide.a", "-o", output / f"{name}-{digits}"])
    metadata = {"commits": versions, "compiler": run([args.compiler, "--version"], capture_output=True).stdout,
                "flags": flags, "platform": platform.platform(), "affinity_cpu": cpu,
                "iterations": args.iterations, "balanced_rounds": args.rounds,
                "driver_sha256": {str(d): hashlib.sha256(p.read_bytes()).hexdigest() for d, p in drivers.items()},
                "note": "Hosted timing is evidence, not a zero-regression guarantee. No timing gate is weakened."}
    cpu_info = Path("/proc/cpuinfo")
    if cpu_info.exists():
        metadata["cpuinfo"] = cpu_info.read_text()
    (output / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n")
    rows = []
    for batch in range(args.rounds):
        order = ("base", "head") if batch % 2 == 0 else ("head", "base")
        for digits in (8, 12):
            for name in order:
                # The benchmark itself alternates rounding modes and checks
                # fixtures against Boost before timing either mode.
                result = subprocess.run([str(output / f"{name}-{digits}"), "--iterations", str(args.iterations),
                                         "--repetitions", "3", "--seed", str(batch + 1)],
                                        capture_output=True, text=True)
                (output / f"{name}-{digits}-{batch}.csv").write_text(result.stdout)
                (output / f"{name}-{digits}-{batch}.log").write_text(result.stderr)
                if result.returncode:
                    print(result.stderr)
                    result.check_returncode()
                if "PASSED oracle_checks=" not in result.stderr:
                    raise RuntimeError("benchmark did not report a successful oracle preflight")
                for row in csv.DictReader(io.StringIO(result.stdout)):
                    row.update(variant=name, batch=batch)
                    rows.append(row)
    with (output / "samples.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    groups = {}
    for row in rows:
        key = (int(row["digits"]), row["workload"], row["mode"])
        groups.setdefault(key, {"base": [], "head": []})[row["variant"]].append(float(row["cpu_ns_per_op"]))
    summary = ["# Same-runner before/after rounding benchmark", "",
               f"Base: `{versions['base']}`; head: `{versions['head']}`.", "",
               "Median thread-CPU ns/op; positive change means slower. AB/BA checkout order, "
               "three internal pairs per round, same driver and flags for both revisions. "
               "All retained runs passed the independent Boost fixture oracle. "
               "The scale-8 driver omits the scale-12-only `mul_wide` adapter.", "",
               "These timing results are not a deterministic gate or a cross-machine guarantee. "
               "Inspect repeated changes alongside the unchanged instruction-count gate.", "",
               "| Digits | Workload | Mode | Before | After | Change |",
               "|---:|---|---|---:|---:|---:|"]
    for (digits, workload, mode), values in sorted(groups.items()):
        before = statistics.median(values["base"])
        after = statistics.median(values["head"])
        summary.append(f"| {digits} | {workload} | {mode} | {before:.3f} | {after:.3f} | {(after / before - 1) * 100:+.2f}% |")
    if args.icount:
        counts = {}
        for name, root in roots.items():
            build = output / ("build-" + name)
            run(["cmake", "-S", root, "-B", build, "-DFIXEDWIDE_BUILD_BENCHMARKS=ON"])
            run(["cmake", "--build", build, "--target", "fixedwide_icount", "--parallel", "2"])
            result = run(["bash", root / "scripts/icount.sh", "--binary", build / "benchmarks/fixedwide_icount"],
                         capture_output=True, env={**os.environ, "CXX": args.compiler})
            (output / f"icount-{name}.csv").write_text(result.stdout)
            (output / f"icount-{name}.log").write_text(result.stderr)
            counts[name] = {row["workload"]: float(row["instructions_per_op"])
                            for row in csv.DictReader(io.StringIO(result.stdout))}
        historical = roots["base"] / "benchmarks/baseline/x86_64-gcc-14.csv"
        counts["committed"] = {row["workload"]: float(row["instructions_per_op"])
                               for row in csv.DictReader(io.StringIO(historical.read_text()))}
        if counts["base"].keys() != counts["head"].keys():
            raise RuntimeError("instruction-count workload sets differ; do not silently compare a subset")
        summary += ["", "## Instruction counts", "",
                    "Same runner and compiler, all existing workloads unchanged. The committed column "
                    "is the historical baseline; base/head isolate this PR from pre-existing baseline drift. "
                    "This diagnostic comparison does not replace or relax the separate 1% CI gate.", "",
                    "| Workload | Committed | Base now | Head now | Head vs base |",
                    "|---|---:|---:|---:|---:|"]
        for workload, before in counts["base"].items():
            after = counts["head"][workload]
            old = counts["committed"].get(workload)
            old_text = f"{old:.3f}" if old is not None else "n/a"
            summary.append(f"| {workload} | {old_text} | {before:.3f} | {after:.3f} | "
                           f"{(after / before - 1) * 100:+.2f}% |")
    text = "\n".join(summary) + "\n"
    (output / "summary.md").write_text(text)
    print(text)


if __name__ == "__main__":
    main()
