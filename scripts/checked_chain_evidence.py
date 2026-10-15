#!/usr/bin/env python3
"""Build, validate and measure checked chains; retain raw evidence, not pass claims."""
from __future__ import annotations

import argparse
import csv
import datetime as dt
import json
import os
from pathlib import Path
import platform
import re
import shlex
import shutil
import statistics
import subprocess


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--iterations", type=int, default=1_048_576)
    parser.add_argument("--repetitions", type=int, default=15)
    parser.add_argument("--trials", type=int, default=3)
    parser.add_argument("--cpu", type=int)
    parser.add_argument("--jobs", type=int, default=2)
    args = parser.parse_args()
    if min(args.iterations, args.repetitions, args.trials, args.jobs) < 1:
        parser.error("counts must be positive")
    root = Path(__file__).resolve().parents[1]
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    compiler = shutil.which(args.compiler)
    if compiler is None:
        parser.error(f"compiler not found: {args.compiler}")
    commands: list[list[str]] = []

    def run(command: list[str], name: str) -> str:
        commands.append(command)
        proc = subprocess.run(command, cwd=root, text=True, capture_output=True, check=False)
        (out / name).write_text(proc.stdout)
        (out / (name + ".stderr")).write_text(proc.stderr)
        (out / "commands.json").write_text(json.dumps(commands, indent=2) + "\n")
        if proc.returncode:
            raise RuntimeError(f"exit {proc.returncode}: {shlex.join(command)}; see {out / name}")
        return proc.stdout

    version = run([compiler, "--version"], "compiler.txt")
    is_clang = "clang" in version.lower()
    vector_flags = (["-fno-vectorize", "-fno-slp-vectorize"] if is_clang else
                    ["-fno-tree-vectorize", "-fno-tree-slp-vectorize"])
    flags = ["-std=c++23", "-O3", "-DNDEBUG", *vector_flags, "-ffp-contract=off"]
    environment = {
        "timestamp_utc": dt.datetime.now(dt.timezone.utc).isoformat(),
        "platform": platform.platform(), "compiler": version,
        "assembly_flags": flags, "cpu_affinity_requested": args.cpu,
        "cpu_affinity_available": sorted(os.sched_getaffinity(0)) if hasattr(os, "sched_getaffinity") else None,
        "parameters": {k: str(v) if isinstance(v, Path) else v for k, v in vars(args).items()},
        "commit": run(["git", "rev-parse", "HEAD"], "commit.txt").strip(),
        "working_tree": run(["git", "status", "--short"], "working-tree.txt"),
    }
    if shutil.which("lscpu"):
        environment["lscpu"] = run(["lscpu"], "cpu.txt")
    (out / "environment.json").write_text(json.dumps(environment, indent=2) + "\n")
    build = out / "build"
    run(["cmake", "-S", str(root), "-B", str(build), "-G", "Ninja",
         f"-DCMAKE_CXX_COMPILER={compiler}", "-DCMAKE_BUILD_TYPE=Release",
         "-DFIXEDWIDE_BUILD_TESTS=OFF", "-DFIXEDWIDE_BUILD_EXAMPLES=OFF",
         "-DFIXEDWIDE_BUILD_BENCHMARKS=ON"], "configure.log")
    run(["cmake", "--build", str(build), "--target", "fixedwide_checked_chain_bench", "-j", str(args.jobs)], "build.log")
    binary = build / "benchmarks/fixedwide_checked_chain_bench"
    samples: dict[str, list[float]] = {}
    for trial in range(args.trials):
        command = [str(binary), "--iterations", str(args.iterations), "--repetitions", str(args.repetitions),
                   "--order", str(trial % 3 + 1)]
        if args.cpu is not None:
            if not shutil.which("taskset"):
                parser.error("--cpu requires taskset")
            command = ["taskset", "-c", str(args.cpu), *command]
        text = run(command, f"trial-{trial + 1}.csv")
        rows = list(csv.DictReader(text.splitlines()))
        if len(rows) != 9 or len({r["workload"] for r in rows}) != 9:
            raise RuntimeError("benchmark did not return nine distinct workload/policy rows")
        for row in rows:
            values = [float(x) for x in row["samples_ns"].split(";")]
            if len(values) != args.repetitions or any(x <= 0 for x in values):
                raise RuntimeError("invalid timing samples")
            samples.setdefault(row["workload"], []).extend(values)
    with (out / "summary.csv").open("w", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(["workload", "median_expected_ns", "median_accumulated_ns", "median_unchecked_ns",
                         "accumulated_vs_expected_pct", "accumulated_vs_unchecked_pct", "samples_per_policy"])
        for name in ("add8.Fixed128", "mul_chain.Fixed256", "float_mixed"):
            medians = [statistics.median(samples[name + "." + p]) for p in ("expected", "accumulated", "unchecked")]
            expected, accumulated, unchecked = medians
            writer.writerow([name, *medians, 100 * (accumulated / expected - 1),
                             100 * (accumulated / unchecked - 1), len(samples[name + ".expected"])])
    obj = out / "checked-chain.o"
    run([compiler, *flags, "-Iinclude", "-Ibenchmarks", "-c", "benchmarks/checked_chain_asm.cpp", "-o", str(obj)], "assembly-build.log")
    assembly = run(["objdump", "-drC", str(obj)], "checked-chain.asm")
    # Whole-function counts are navigation aids, NOT dynamic/happy-path proof.
    counts: list[list[object]] = []
    for part in re.split(r"(?=^[0-9a-f]+ <)", assembly, flags=re.MULTILINE):
        first = part.splitlines()[0] if part else ""
        if "<checked_chain_" not in first:
            continue
        insns = re.findall(r"^\s*[0-9a-f]+:\s+(?:[0-9a-f]{2}\s+)+\s*([a-z][a-z0-9]*)\s*(.*)", part, re.MULTILINE)
        counts.append([first, len(insns), sum(m.startswith("j") and m != "jmp" for m, _ in insns),
                       sum(m.startswith("call") for m, _ in insns),
                       sum(m in ("push", "pop") for m, _ in insns),
                       sum("(%rsp)" in operands for _, operands in insns)])
    with (out / "assembly-counts.csv").open("w", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(["function", "instructions_static", "conditional_branches_static", "calls_static",
                         "push_pop_static", "rsp_memory_operands_static"])
        writer.writerows(counts)
    print((out / "summary.csv").read_text(), end="")
    print("Raw samples and disassembly retained. No zero-overhead/branch-parity claim is inferred from these counts.")


if __name__ == "__main__":
    main()
