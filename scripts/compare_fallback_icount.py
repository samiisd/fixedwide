#!/usr/bin/env python3
"""Compare executed guest instructions; this does not measure hardware latency.

Freeze the candidate fixture, compile it against two isolated arithmetic
revisions, validate every case, then subtract Callgrind I(N) from I(2N).
Outputs are explicit artifacts, never committed baselines or new CI thresholds.
"""

import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tarfile


FLAGS = ["-O3", "-DNDEBUG", "-march=x86-64", "-ffp-contract=off", "-fno-fast-math"]
HARNESS = ["fallback_icount.cpp", "fallback_codegen.cpp", "batch_codegen.cpp"]
COMPLETED = re.compile(r"completed_calls=(\d+) completed_rows_per_call=(\d+) "
                       r"completed_positions_per_call=(\d+) digest=(\d+)")


def run(command, log, cwd=None, input_text=None):
    result = subprocess.run(command, cwd=cwd, input=input_text, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    Path(log).write_text("+ " + shlex.join(str(x) for x in command) + "\n" + result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(f"Command failed ({result.returncode}); {log}\n{result.stderr[-6000:]}")
    return result


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def save_json(path, data):
    Path(path).write_text(json.dumps(data, indent=2) + "\n")


def resolve(repo, value):
    if value != "HEAD" and not re.fullmatch(r"[0-9a-fA-F]{40}", value):
        raise ValueError("revisions must be exact 40-digit SHAs, or HEAD for the candidate")
    return subprocess.check_output(["git", "rev-parse", "--verify", value + "^{commit}"],
                                   cwd=repo, text=True).strip()


def extract(repo, sha, destination):
    destination.mkdir()
    archive = subprocess.check_output(["git", "archive", "--format=tar", sha], cwd=repo)
    with tarfile.open(fileobj=io.BytesIO(archive)) as content:
        content.extractall(destination, filter="data")


def completed(output, iterations):
    match = COMPLETED.fullmatch(output.strip())
    if not match:
        raise ValueError("Missing complete-call validation record")
    calls, rows, positions, checksum = (int(x) for x in match.groups())
    if calls != iterations or rows == 0:
        raise ValueError("Invalid completed-call/row count")
    return {"calls": calls, "rows_per_call": rows, "positions_per_call": positions, "digest": checksum}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", required=True, help="Exact baseline commit SHA already present locally")
    parser.add_argument("--candidate", default="HEAD", help="Exact candidate commit SHA, or HEAD")
    parser.add_argument("--compiler", default="g++-14")
    parser.add_argument("--output", required=True, type=Path, help="New evidence directory outside the checkout")
    parser.add_argument("--iterations", default=40, type=int, help="Positive even call count N; default 40")
    parser.add_argument("--filter", default=".*", help="Python regex selecting workload names")
    args = parser.parse_args()
    if args.iterations < 2 or args.iterations > 500_000 or args.iterations % 2:
        raise ValueError("iterations must be an even integer from 2 through 500000")
    # Argument parsing and the completion print are outside repeated work.
    # Keep their decimal conversion lengths equal in the two processes too.
    if len(str(args.iterations)) != len(str(2 * args.iterations)):
        raise ValueError("choose N with the same decimal digit count as 2N (for example 40)")
    if not re.fullmatch(r"[0-9a-fA-F]{40}", args.baseline):
        raise ValueError("baseline must be an exact 40-digit commit SHA")
    selected = re.compile(args.filter)
    repo = Path(__file__).resolve().parents[1]
    output = args.output.resolve()
    if output.is_relative_to(repo):
        raise ValueError("output must be outside the source checkout")
    compiler, valgrind = shutil.which(args.compiler), shutil.which("valgrind")
    if compiler is None or valgrind is None:
        raise ValueError("The selected compiler and valgrind must already be installed/on PATH")
    revisions = {"baseline": resolve(repo, args.baseline), "candidate": resolve(repo, args.candidate)}
    output.mkdir(parents=True, exist_ok=False)
    metadata = {"revisions": revisions, "flags": ["-std=c++23", *FLAGS], "iterations_N": args.iterations,
                "count_kind": "Callgrind executed guest instructions (Ir), not hardware cycles or retired counters",
                "filter": args.filter, "github_run_id": os.environ.get("GITHUB_RUN_ID"),
                "github_run_attempt": os.environ.get("GITHUB_RUN_ATTEMPT"), "sources": {}, "binaries": {}}
    metadata["compiler_version"] = run([compiler, "--version"], output / "compiler.log").stdout
    metadata["valgrind_version"] = run([valgrind, "--version"], output / "valgrind-version.log").stdout
    macros = run([compiler, "-std=c++23", "-dM", "-E", "-x", "c++", "-"],
                 output / "stdlib.log", input_text="#include <version>\n").stdout
    metadata["stdlib"] = [line for line in macros.splitlines()
                          if re.match(r"#define (?:__GLIBCXX__|_GLIBCXX_RELEASE|_LIBCPP_VERSION) ", line)]
    for name, sha in revisions.items():
        path = output / name
        path.mkdir()
        extract(repo, sha, path / "source")
        metadata["sources"][name] = {p.relative_to(path / "source").as_posix(): digest(p)
                                     for p in sorted((path / "source/include").rglob("*.hpp"))}
    harness = output / "harness"
    harness.mkdir()
    for name in HARNESS:
        shutil.copy2(output / "candidate/source/benchmarks" / name, harness / name)
    metadata["harness_sha256"] = {name: digest(harness / name) for name in HARNESS}
    save_json(output / "metadata.json", metadata)
    binaries = {}
    for name in revisions:
        path = output / name
        source, build = path / "source", path / "build"
        print(f"Building {name} {revisions[name]} with {args.compiler}", flush=True)
        run(["cmake", "-S", str(source), "-B", str(build), "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release",
             "-DCMAKE_CXX_COMPILER=" + compiler, "-DCMAKE_CXX_FLAGS=" + " ".join(FLAGS),
             "-DBUILD_SHARED_LIBS=OFF", "-DFIXEDWIDE_BUILD_TESTS=OFF", "-DFIXEDWIDE_BUILD_EXAMPLES=OFF",
             "-DFIXEDWIDE_BUILD_BENCHMARKS=OFF"], path / "configure.log")
        run(["cmake", "--build", str(build), "--parallel", "2"], path / "build.log")
        binary = path / "fallback_icount"
        run([compiler, "-std=c++23", *FLAGS, "-I" + str(source / "include"),
             *(str(harness / filename) for filename in HARNESS), str(build / "libfixedwide.a"),
             "-o", str(binary)], path / "compile-harness.log")
        binaries[name] = binary
        metadata["binaries"][name] = digest(binary)
        save_json(output / "metadata.json", metadata)
    lists = {name: run([str(binary), "--list"], output / name / "workloads.log").stdout.splitlines()
             for name, binary in binaries.items()}
    if lists["baseline"] != lists["candidate"]:
        raise ValueError("Baseline and candidate workload lists differ")
    workloads = [case for case in lists["candidate"] if selected.search(case)]
    if not workloads or len(workloads) != len(set(workloads)):
        raise ValueError("Workload selection is empty or contains duplicates")
    rows = []
    for workload in workloads:
        if not re.fullmatch(r"[a-zA-Z0-9_.-]+", workload):
            raise ValueError("Invalid workload name")
        sample = {"workload": workload}
        validation = None
        for name, binary in binaries.items():
            path = output / name
            native = run([str(binary), workload, "2"], path / (workload + "-validation.log"))
            current = completed(native.stdout, 2)
            if validation is not None and current != validation:
                raise ValueError("Baseline/candidate independent validation outputs differ")
            validation = current
            totals = []
            for n in (args.iterations, 2 * args.iterations):
                stem = path / (workload + "-" + str(n))
                result = run([valgrind, "--tool=callgrind", "--callgrind-out-file=" + str(stem) + ".callgrind",
                              str(binary), workload, str(n)], str(stem) + ".log")
                record = completed(result.stdout, n)
                if record["rows_per_call"] != current["rows_per_call"] or record["positions_per_call"] != current["positions_per_call"]:
                    raise ValueError("Completed work changed during counting")
                if record["digest"] != current["digest"]:
                    raise ValueError("Counted output differs from independently validated native output")
                match = re.search(r"I\s+refs:\s*([\d,]+)", result.stderr)
                if not match:
                    raise ValueError("Callgrind did not report executed guest instructions")
                totals.append(int(match[1].replace(",", "")))
            per_call = (totals[1] - totals[0]) / args.iterations
            if per_call <= 0:
                raise ValueError("Nonpositive marginal instruction count")
            sample[name] = {"I_N": totals[0], "I_2N": totals[1], "instructions_per_call": per_call,
                            "instructions_per_row": per_call / current["rows_per_call"],
                            "instructions_per_position": (per_call / current["positions_per_call"]
                                                          if current["positions_per_call"] else None)}
        sample["completed_rows_per_call"] = validation["rows_per_call"]
        sample["completed_positions_per_call"] = validation["positions_per_call"]
        sample["change_percent"] = (sample["candidate"]["instructions_per_row"] /
                                    sample["baseline"]["instructions_per_row"] - 1) * 100
        rows.append(sample)
        save_json(output / "counts.json", rows)
        print(f"{workload}: {sample['baseline']['instructions_per_row']:.3f} -> "
              f"{sample['candidate']['instructions_per_row']:.3f} guest instructions/row "
              f"({sample['change_percent']:+.2f}%)", flush=True)
    save_json(output / "metadata.json", metadata)
    summary = ["# Wide fallback instruction comparison", "",
               f"Baseline `{revisions['baseline']}`; candidate `{revisions['candidate']}`.", "",
               "Callgrind executed guest instructions (Ir); these are not Zen 5 timings, hardware-retired counters, "
               "latency estimates or performance gates. Both arithmetic revisions use the identical candidate fixture.", "",
               f"N={args.iterations} complete calls; per-call cost is `(I(2N)-I(N))/N`. "
               "Every call completes 400 input rows. Publication completes 100 positions per call; "
               "multiply its per-row number by four for the per-position cost. Other kernels report completed rows, "
               "without equating rows to distinct balance destinations.", "",
               "| Workload | Baseline instructions/row | Candidate instructions/row | Change |",
               "|---|---:|---:|---:|"]
    summary += [f"| `{r['workload']}` | {r['baseline']['instructions_per_row']:.3f} | "
                f"{r['candidate']['instructions_per_row']:.3f} | {r['change_percent']:+.2f}% |" for r in rows]
    summary += ["", "Prepared calls include preparation. Delta includes checked row and ordered duplicate-balance adds. "
                "Anchored calls include separate nearest-even price/FX products, residual addition, Wide replacement and "
                "checked narrowing with duplicate destinations and 5% sink holes. Publication preserves individual "
                "entry rounding and position order. Independent cpp_int validation runs outside counted repetitions.", ""]
    (output / "SUMMARY.md").write_text("\n".join(summary))
    save_json(output / "completion.json", {"complete": True, "workloads": len(rows), "revisions": revisions})


if __name__ == "__main__":
    main()
