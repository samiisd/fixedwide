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
import math
import os
from pathlib import Path
import platform
import statistics
import subprocess


def run(args, **kwargs):
    return subprocess.run([str(arg) for arg in args], check=True, text=True, **kwargs)


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def matrix_keys():
    return {(digits, f"{op}.Fixed64_{digits}.{mode}.{kind}.{sign}", mode)
            for digits in (8, 12) for op in ("mul", "div", "mul_div")
            for mode in ("toward_zero", "nearest_even")
            for kind in ("exact", "inexact") for sign in ("positive", "mixed")}


def validate_matrix(text, iterations, repetitions):
    rows = list(csv.DictReader(io.StringIO(text)))
    expected = {(*key, repeat) for key in matrix_keys() for repeat in range(repetitions)}
    seen = set()
    for row in rows:
        key = (int(row["digits"]), row["workload"], row["mode"], int(row["repeat"]))
        if key not in expected or key in seen:
            raise ValueError(f"unexpected or duplicate matrix sample: {key}")
        seen.add(key)
        timing = float(row["cpu_ns_per_op"])
        if int(row["iterations"]) != iterations or not math.isfinite(timing) or timing <= 0:
            raise ValueError(f"invalid matrix sample: {key}")
        if not 0 <= int(row["checksum"]) <= (1 << 64) - 1:
            raise ValueError(f"invalid matrix checksum: {key}")
    if seen != expected:
        raise ValueError(f"missing {len(expected - seen)} matrix samples")
    return rows


def compare_matrix(rows):
    groups = {}
    for row in rows:
        key = (int(row["digits"]), row["workload"], row["mode"])
        group = groups.setdefault(key, {"base": [], "head": [], "checksums": set()})
        group[row["variant"]].append(float(row["cpu_ns_per_op"]))
        group["checksums"].add(int(row["checksum"]))
    if set(groups) != matrix_keys():
        raise ValueError("incomplete paired matrix")
    for key, group in groups.items():
        if not group["base"] or len(group["base"]) != len(group["head"]):
            raise ValueError(f"unbalanced paired matrix: {key}")
        if len(group["checksums"]) != 1:
            raise ValueError(f"result checksum mismatch: {key}")
    return groups


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
    # No silent baseline/gate/workload changes in this safety comparison.
    protected = ("benchmarks/icount.cpp", "benchmarks/baseline/x86_64-gcc-14.csv",
                 "scripts/icount.sh", "scripts/compare_icount.py", ".github/workflows/ci.yml",
                 "benchmarks/rounding_bench.cpp")
    protected_hashes = {}
    for relative in protected:
        before, after = (sha256(root / relative) for root in roots.values())
        if before != after:
            raise RuntimeError(f"protected comparison input changed: {relative}")
        protected_hashes[relative] = before
    matrix_source = output / "fastpath_matrix.cpp"
    matrix_source.write_bytes((roots["head"] / "benchmarks/fastpath_matrix.cpp").read_bytes())
    versions = {}
    binaries = {}
    libraries = {}
    for name, root in roots.items():
        versions[name] = run(["git", "-C", root, "rev-parse", "HEAD"], capture_output=True).stdout.strip()
        build = output / ("build-" + name)
        run(["cmake", "-S", root, "-B", build, "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release",
             f"-DCMAKE_CXX_COMPILER={args.compiler}", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DFIXEDWIDE_BUILD_TESTS=OFF",
             "-DFIXEDWIDE_BUILD_EXAMPLES=OFF", "-DFIXEDWIDE_BUILD_ORACLE_TESTS=OFF",
             "-DFIXEDWIDE_BUILD_BENCHMARKS=OFF", "-DBUILD_SHARED_LIBS=OFF"])
        run(["cmake", "--build", build, "--parallel", "2"])
        database = build / "compile_commands.json"
        retained_database = output / f"{name}-library-compile_commands.json"
        retained_database.write_bytes(database.read_bytes())
        libraries[name] = {"sha256": sha256(build / "libfixedwide.a"),
                           "compile_commands": retained_database.name,
                           "compile_commands_sha256": sha256(retained_database)}
        for label, source in {**drivers, "matrix": matrix_source}.items():
            binary = output / f"{name}-{label}"
            command = [str(v) for v in (args.compiler, *flags, f"-I{root / 'include'}", source,
                                       build / "libfixedwide.a", "-o", binary)]
            run(command)
            binaries[binary.name] = {"sha256": sha256(binary), "size": binary.stat().st_size,
                                     "compile_link_command": command}
            if label == "matrix":
                with (output / f"{name}-matrix.asm").open("w") as stream:
                    run(["objdump", "-d", "-C", binary], stdout=stream)
    metadata = {"commits": versions, "compiler": run([args.compiler, "--version"], capture_output=True).stdout,
                "flags": flags, "platform": platform.platform(), "affinity_cpu": cpu,
                "executables": binaries, "libraries": libraries, "protected_sha256": protected_hashes,
                "matrix_sha256": sha256(matrix_source), "matrix_workloads": len(matrix_keys()),
                "iterations": args.iterations, "balanced_rounds": args.rounds,
                "driver_sha256": {str(d): hashlib.sha256(p.read_bytes()).hexdigest() for d, p in drivers.items()},
                "note": "Hosted timing is evidence, not a zero-regression guarantee. No timing gate is weakened."}
    cpu_info = Path("/proc/cpuinfo")
    if cpu_info.exists():
        metadata["cpuinfo"] = cpu_info.read_text()
    (output / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n")
    rows = []
    matrix_rows = []
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
        for name in order:
            result = subprocess.run([str(output / f"{name}-matrix"), "--timing", str(args.iterations), "3"],
                                    capture_output=True, text=True)
            (output / f"matrix-{name}-{batch}.csv").write_text(result.stdout)
            (output / f"matrix-{name}-{batch}.log").write_text(result.stderr)
            result.check_returncode()
            if "PASSED oracle_checks=12288" not in result.stderr:
                raise RuntimeError("matrix preflight did not validate all 48 fixtures")
            for row in validate_matrix(result.stdout, args.iterations, 3):
                row.update(variant=name, batch=batch)
                matrix_rows.append(row)
    matrix_groups = compare_matrix(matrix_rows)
    with (output / "matrix-samples.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(matrix_rows[0]))
        writer.writeheader()
        writer.writerows(matrix_rows)
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
    summary += ["", "## Constant-policy scale-8/12 matrix", "",
                "48 additional independent-throughput workloads: each operation and rounding policy "
                "has positive/mixed-sign and exact/inexact fixtures. The policy is a template constant "
                "in these loops, complementing the historical runtime-policy driver above. "
                "Every fixture has an independent signed-128 rational oracle and matching "
                "base/head checksums. These are not dependent-operation latency measurements.", "",
                "| Digits | Workload | Mode | Before | After | Change |",
                "|---:|---|---|---:|---:|---:|"]
    for (digits, workload, mode), values in sorted(matrix_groups.items()):
        before, after = (statistics.median(values[v]) for v in ("base", "head"))
        summary.append(f"| {digits} | {workload} | {mode} | {before:.3f} | {after:.3f} | "
                       f"{(after / before - 1) * 100:+.2f}% |")
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
        matrix_counts = {}
        for name, root in roots.items():
            result = run(["bash", root / "scripts/icount.sh", "--binary", output / f"{name}-matrix"],
                         capture_output=True, env={**os.environ, "CXX": args.compiler})
            (output / f"matrix-icount-{name}.csv").write_text(result.stdout)
            (output / f"matrix-icount-{name}.log").write_text(result.stderr)
            matrix_counts[name] = {row["workload"]: float(row["instructions_per_op"])
                                   for row in csv.DictReader(io.StringIO(result.stdout))}
        expected_names = {key[1] for key in matrix_keys()}
        if any(set(values) != expected_names for values in matrix_counts.values()):
            raise RuntimeError("missing supplementary instruction-count workloads")
        summary += ["", "## Supplementary instruction counts", "",
                    "Same-runner base/head only; these 48 new workloads do not overwrite or "
                    "relax the existing committed 34-row baseline. Positive change is reported "
                    "even when the original gate passes.", "",
                    "| Workload | Base | Head | Change |", "|---|---:|---:|---:|"]
        for workload in sorted(expected_names):
            before, after = (matrix_counts[v][workload] for v in ("base", "head"))
            if not all(math.isfinite(v) and v > 0 for v in (before, after)):
                raise RuntimeError(f"invalid supplementary instruction count: {workload}")
            summary.append(f"| {workload} | {before:.3f} | {after:.3f} | {(after / before - 1) * 100:+.2f}% |")
    text = "\n".join(summary) + "\n"
    (output / "summary.md").write_text(text)
    print(text)


if __name__ == "__main__":
    main()
