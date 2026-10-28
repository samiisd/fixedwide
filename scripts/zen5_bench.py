#!/usr/bin/env python3
"""Manual-run measurement controls; Google Benchmark owns all timing.

ZEN5_* inputs mirror .github/workflows/zen5-benchmarks.yml. No process, power,
governor, SMT or machine-wide affinity settings are changed. The selected CPU
is an affinity constraint, not a reservation against unrelated local jobs.
"""

import json
import math
import os
from pathlib import Path
import re
import shlex
import shutil
import statistics
import subprocess
import sys
import tempfile
import time


def cpu_list(value):
    result = set()
    for part in value.strip().split(","):
        if not re.fullmatch(r"\d+(?:-\d+)?", part):
            raise ValueError(f"Invalid CPU list: {value!r}")
        ends = [int(x) for x in part.split("-")]
        first, last = ends[0], ends[-1]
        if last < first:
            raise ValueError(f"Invalid CPU range: {part}")
        result.update(range(first, last + 1))
    return result


def read(path, default=None):
    try:
        return Path(path).read_text().strip()
    except (OSError, UnicodeError):
        return default


def cache_bytes(value):
    match = re.fullmatch(r"(\d+)([KMG]?)", value)
    if not match:
        raise ValueError(f"Invalid cache size: {value!r}")
    return int(match[1]) * (1024 ** {"": 0, "K": 1, "M": 2, "G": 3}[match[2]])


def topology(root=Path("/sys/devices/system/cpu"), allowed=None):
    online = cpu_list(read(root / "online", ""))
    allowed = set(os.sched_getaffinity(0) if allowed is None else allowed) & online
    if not allowed:
        raise ValueError("No online CPU is allowed by the runner affinity")
    cpus = []
    for cpu in sorted(online):
        path = root / f"cpu{cpu}"
        caches = []
        for cache in sorted((path / "cache").glob("index*")):
            caches.append({
                "index": cache.name,
                "level": int(read(cache / "level")),
                "type": read(cache / "type"),
                "size_bytes": cache_bytes(read(cache / "size")),
                "id": read(cache / "id"),
                "shared_cpus": sorted(cpu_list(read(cache / "shared_cpu_list"))),
            })
        cpus.append({
            "cpu": cpu,
            "allowed": cpu in allowed,
            "package_id": read(path / "topology/physical_package_id"),
            "die_id": read(path / "topology/die_id"),
            "core_id": read(path / "topology/core_id"),
            "siblings": sorted(cpu_list(read(path / "topology/thread_siblings_list"))),
            "caches": caches,
        })
    return {"online": sorted(online), "allowed": sorted(allowed), "cpus": cpus}


def select_cpu(info, ccd):
    if ccd == "all":
        return None
    if ccd not in ("vcache", "frequency"):
        raise ValueError("ccd must be vcache, frequency or all")
    allowed = set(info["allowed"])
    groups = {}
    for cpu in info["cpus"]:
        for cache in cpu["caches"]:
            if cache["level"] == 3:
                groups[tuple(cache["shared_cpus"])] = cache["size_bytes"]
    sizes = set(groups.values())
    if len(sizes) < 2:
        raise ValueError("Cannot distinguish both CCDs from discovered L3 cache sizes")
    wanted = max(sizes) if ccd == "vcache" else min(sizes)
    eligible = sorted(key for key, size in groups.items() if size == wanted)
    if len(eligible) != 1:
        raise ValueError("Ambiguous CCD topology: expected one L3 group of each size")
    members = allowed.intersection(eligible[0])
    if not members:
        raise ValueError(f"No allowed CPU belongs to the requested {ccd} CCD")
    physical = {}
    for cpu in info["cpus"]:
        if cpu["cpu"] in members:
            # Sibling sets identify physical cores even when core IDs repeat
            # across packages. No assumed CPU-numbering or SMT-offset scheme.
            physical.setdefault(tuple(cpu["siblings"]), cpu["cpu"])
    choices = sorted(physical.values())
    # Deterministic selection, avoiding the first physical core when possible.
    # Observe sibling load, but never move or terminate unrelated work.
    return choices[1 if len(choices) > 1 else 0]


def settings(env):
    result = {
        "ccd": env.get("ZEN5_CCD") or "vcache",
        "compiler": env.get("ZEN5_COMPILER") or "clang",
        "flags": env.get("ZEN5_FLAGS") or "-O3 -march=x86-64",
        "filter": env.get("ZEN5_FILTER") or ".*",
        "min_time": env.get("ZEN5_MIN_TIME") or "0.2s",
        "baseline_sha": env.get("ZEN5_BASELINE_SHA", ""),
        "suite": env.get("ZEN5_SUITE") or "batch",
    }
    for field, choices in (("ccd", ("vcache", "frequency", "all")),
                           ("compiler", ("clang", "gcc")), ("suite", ("batch", "runtime", "zen5"))):
        if result[field] not in choices:
            raise ValueError(f"Invalid {field} choice")
    repetitions = env.get("ZEN5_REPETITIONS") or "5"
    if not re.fullmatch(r"\d+", repetitions) or not 1 <= int(repetitions) <= 30:
        raise ValueError("repetitions must be an integer from 1 through 30")
    result["repetitions"] = int(repetitions)
    if not re.fullmatch(r"(?:\d+(?:\.\d*)?|\.\d+)s", result["min_time"]):
        raise ValueError("min_time must be a positive number followed by s")
    if not 0.001 <= float(result["min_time"][:-1]) <= 60:
        raise ValueError("min_time must be between 0.001s and 60s")
    if result["baseline_sha"] and not re.fullmatch(r"[0-9a-fA-F]{40}", result["baseline_sha"]):
        raise ValueError("baseline_sha must be an exact 40-digit hexadecimal commit")
    if len(result["filter"]) > 2048 or any(x in result["filter"] for x in "\r\n\0"):
        raise ValueError("Invalid benchmark filter")
    # Each option is data, without shell interpolation or eval. Keep CMake's
    # compiler command safe too: accept simple switches, not shell fragments,
    # response files, source files or dependency installation commands.
    flags = shlex.split(result["flags"])
    if not flags or any(not re.fullmatch(r"-[A-Za-z0-9_=+.,-]+", flag) for flag in flags):
        raise ValueError("extra_flags must contain only simple compiler switches")
    result["flags"] = " ".join(flags + ["-ffp-contract=off", "-fno-fast-math"])
    result["stabilization_seconds"] = 5
    result["warmup_seconds"] = 0.05
    return result


def run(command, log=None, cwd=None, capture=False):
    print("+ " + shlex.join(str(x) for x in command), flush=True)
    if log is None:
        return subprocess.run(command, cwd=cwd, check=True, text=True,
                              stdout=subprocess.PIPE if capture else None).stdout
    with Path(log).open("w") as stream:
        stream.write("+ " + shlex.join(str(x) for x in command) + "\n")
        stream.flush()
        proc = subprocess.run(command, cwd=cwd, stdout=stream, stderr=subprocess.STDOUT)
    if proc.returncode:
        print(Path(log).read_text()[-12000:], file=sys.stderr)
        proc.check_returncode()


def write_json(path, value):
    Path(path).write_text(json.dumps(value, indent=2) + "\n")


def observations(cpu):
    result = {"time_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
              "loadavg": read("/proc/loadavg"), "proc_stat": read("/proc/stat"),
              "controller_affinity": sorted(os.sched_getaffinity(0)), "sensors": {}}
    paths = list(Path("/sys/class/hwmon").glob("hwmon*/temp*_input"))
    paths += list(Path("/sys/class/hwmon").glob("hwmon*/temp*_label"))
    paths += list(Path("/sys/class/hwmon").glob("hwmon*/name"))
    paths += list(Path("/sys/class/powercap").glob("*/energy_uj"))
    if cpu is not None:
        paths += list(Path(f"/sys/devices/system/cpu/cpu{cpu}/cpufreq").glob("*freq"))
        paths += [Path(f"/sys/devices/system/cpu/cpu{cpu}/cpufreq/scaling_governor")]
    for path in paths:
        value = read(path)
        if value is not None:
            result["sensors"][str(path)] = value
    return result


CPU_PROBE = r'''
#include <cstdio>
#include <expected>
#include <version>
#include <cpuid.h>
#include <sched.h>
int main() {
    // Fail before any benchmark build if this compiler/stdlib combination
    // cannot provide the library's required C++23 expected type.
    const std::expected<int, int> language_check{1};
    if (!language_check || *language_check != 1) return 1;
    cpu_set_t affinity;
    CPU_ZERO(&affinity);
    if (sched_getaffinity(0, sizeof(affinity), &affinity) != 0) return 1;
    std::printf("probe_cpu=%d affinity=", sched_getcpu());
    for (int cpu = 0; cpu < CPU_SETSIZE; ++cpu)
        if (CPU_ISSET(cpu, &affinity)) std::printf("%d,", cpu);
    std::printf("\n");
    unsigned a, b, c, d;
    __cpuid(1, a, b, c, d);
    std::printf("cpuid.1: eax=%08x ebx=%08x ecx=%08x edx=%08x\n", a, b, c, d);
    unsigned lo = 0, hi = 0;
    if ((c & (1u << 27)) != 0) {
        __asm__("xgetbv" : "=a"(lo), "=d"(hi) : "c"(0));
    }
    std::printf("xcr0=%08x%08x avx_state=%u avx512_state=%u\n",
                hi, lo, (lo & 6) == 6, (lo & 0xe6) == 0xe6);
    __cpuid_count(7, 0, a, b, c, d);
    std::printf("cpuid.7.0: eax=%08x ebx=%08x ecx=%08x edx=%08x\n", a, b, c, d);
    __builtin_cpu_init();
    std::printf("compiler_cpu_supports: avx2=%d avx512f=%d avx512vl=%d avx512dq=%d "
                "avx512ifma=%d avx512cd=%d bmi2=%d adx=%d\n",
                !!__builtin_cpu_supports("avx2"), !!__builtin_cpu_supports("avx512f"),
                !!__builtin_cpu_supports("avx512vl"), !!__builtin_cpu_supports("avx512dq"),
                !!__builtin_cpu_supports("avx512ifma"), !!__builtin_cpu_supports("avx512cd"),
                !!__builtin_cpu_supports("bmi2"), !!__builtin_cpu_supports("adx"));
#ifdef __GLIBCXX__
    std::printf("libstdc++ __GLIBCXX__=%d\n", __GLIBCXX__);
#endif
#ifdef _GLIBCXX_RELEASE
    std::printf("libstdc++ release=%d\n", _GLIBCXX_RELEASE);
#endif
#ifdef _LIBCPP_VERSION
    std::printf("libc++ version=%d\n", _LIBCPP_VERSION);
#endif
}
'''


def benchmark_rows(path):
    data = json.loads(Path(path).read_text())
    rows = [row for row in data.get("benchmarks", []) if row.get("run_type", "iteration") == "iteration"]
    if not rows:
        raise ValueError(f"Empty benchmark selection/results: {path}")
    for row in rows:
        if row.get("error_occurred") or int(row.get("iterations", 0)) <= 0:
            raise ValueError(f"Benchmark failed or did not execute: {row.get('name')}")
        if not all(math.isfinite(row.get(key, float("nan"))) and row[key] >= 0
                   for key in ("cpu_time", "real_time")):
            raise ValueError(f"Invalid benchmark timings: {row.get('name')}")
    return rows


def summary(results, config, metadata):
    grouped = {}
    for revision, pair, path in results:
        for row in benchmark_rows(path):
            key = (revision, row.get("run_name", row["name"]))
            unit = {"ns": 1, "us": 1000, "ms": 1000000, "s": 1000000000}[row["time_unit"]]
            completed = row.get("completed_rows", 0)
            divisor = completed if completed > 0 else 1
            grouped.setdefault(key, []).append({
                "pair": pair, "cpu": row["cpu_time"] * unit / divisor,
                "wall": row["real_time"] * unit / divisor,
                "unit": "ns/completed row" if completed > 0 else "ns/call",
            })
    lines = ["# Zen 5 benchmark evidence", "",
             f"Candidate: `{metadata['candidate_sha']}`; baseline: `{config['baseline_sha'] or 'same executable backends'}`.",
             f"CPU: `{metadata['selected_cpu']}`; CCD: `{config['ccd']}`; compiler: `{config['compiler']}`.",
             f"Flags: `{config['flags']}`. Run: `{metadata['run_id']}` attempt `{metadata['run_attempt']}`.", "",
             "All builds and separate validation passes completed before a five-second stabilization pause. "
             "Each benchmark process validates again before Google Benchmark's 0.05-second per-case warm-up. "
             "Validation is outside measured time. Raw repetitions, topology, XCR0, build "
             "commands and pre/post load observations are in the artifact. Affinity does not reserve the "
             "core or its SMT sibling from unrelated workstation jobs.", ""]
    if config["ccd"] == "all":
        lines += ["**Unpinned diagnostic run; this is not a multicore scaling measurement.**", ""]
    lines += ["CPU time and wall time are distinct. Ranges are repetition dispersion, not quote latency percentiles.", "",
              "| Revision | Case | Unit | CPU median | CPU min–max | Wall median | Samples |",
              "|---|---|---|---:|---:|---:|---:|"]
    for (revision, name), samples in sorted(grouped.items()):
        cpu = [s["cpu"] for s in samples]
        wall = [s["wall"] for s in samples]
        lines.append(f"| {revision} | {name} | {samples[0]['unit']} | {statistics.median(cpu):.4f} | "
                     f"{min(cpu):.4f}–{max(cpu):.4f} | {statistics.median(wall):.4f} | {len(cpu)} |")
    if config["baseline_sha"]:
        lines += ["", "Revision order alternates A/B and B/A. Paired CPU differences use matching rounds.", "",
                  "| Case | Median candidate − baseline | Paired range | Unit |",
                  "|---|---:|---:|---|"]
        for (revision, name), samples in sorted(grouped.items()):
            if revision != "candidate" or ("baseline", name) not in grouped:
                continue
            baseline = {s["pair"]: s["cpu"] for s in grouped[("baseline", name)]}
            differences = [s["cpu"] - baseline[s["pair"]] for s in samples]
            lines.append(f"| {name} | {statistics.median(differences):+.4f} | "
                         f"{min(differences):+.4f}–{max(differences):+.4f} | {samples[0]['unit']} |")
    return "\n".join(lines) + "\n"


def main():
    fallback = Path(tempfile.gettempdir()) / f"fixedwide-zen5-results-{os.getpid()}"
    output = Path(os.environ.get("ZEN5_OUTPUT", str(fallback))).resolve()
    output.mkdir(parents=True, exist_ok=False)
    print(f"Evidence directory: {output}", flush=True)
    source = Path(__file__).resolve().parent.parent
    config = settings(os.environ)
    info = topology()
    cpu = select_cpu(info, config["ccd"])
    candidate_sha = run(["git", "rev-parse", "HEAD"], cwd=source, capture=True).strip()
    if os.environ.get("GITHUB_SHA", candidate_sha) != candidate_sha:
        raise ValueError("Checked-out SHA does not match the dispatched SHA")
    if run(["git", "status", "--porcelain", "--untracked-files=no"], cwd=source, capture=True).strip():
        raise ValueError("Tracked source modifications would invalidate the recorded SHA")
    metadata = {"candidate_sha": candidate_sha, "selected_cpu": cpu,
                "run_id": os.environ.get("GITHUB_RUN_ID", "local"),
                "run_attempt": os.environ.get("GITHUB_RUN_ATTEMPT", "1"),
                "kernel": list(os.uname()), "os_release": read("/etc/os-release"),
                "proc_cpuinfo": read("/proc/cpuinfo"), "settings": config, "topology": info}
    write_json(output / "environment.json", metadata)
    print(json.dumps(metadata, indent=2), flush=True)
    if os.environ.get("GITHUB_ACTIONS") == "true" and "9950X3D" not in (metadata["proc_cpuinfo"] or ""):
        raise ValueError("Runner CPU is not the intended Ryzen 9 9950X3D")
    if config["baseline_sha"]:
        # Accept only exact baseline commits from reviewed main history, never
        # a remote URL or pull-request ref supplied by an untrusted fork.
        run(["git", "merge-base", "--is-ancestor", config["baseline_sha"], "origin/main"], cwd=source)
    cxx = "clang++" if config["compiler"] == "clang" else "g++"
    run([cxx, "--version"], output / "compiler-version.txt")
    suite_name = {"batch": "fixedwide_batch_bench", "runtime": "fixedwide_runtime_narrow_bench",
                  "zen5": "fixedwide_zen5_bench"}[config["suite"]]
    prefix = ["taskset", "-c", str(cpu)] if cpu is not None else []
    if prefix:
        run(prefix + ["true"])
    results = []
    with tempfile.TemporaryDirectory(prefix="fixedwide-zen5-", dir=os.environ.get("RUNNER_TEMP")) as temporary:
        root = Path(temporary)
        (root / "cpu_probe.cpp").write_text(CPU_PROBE)
        (output / "cpu_probe.cpp").write_text(CPU_PROBE)
        probe_flags = [flag for flag in shlex.split(config["flags"]) if flag.startswith("-stdlib=")]
        run([cxx, "-std=c++23", "-O2", "-march=x86-64", *probe_flags,
             str(root / "cpu_probe.cpp"), "-o", str(root / "cpu_probe")], output / "cpu-probe-build.txt")
        run(prefix + [str(root / "cpu_probe")], output / "cpu-probe.txt")
        revisions = {"candidate": source}
        if config["baseline_sha"]:
            baseline = root / "baseline-source"
            run(["git", "clone", "--no-hardlinks", "--no-checkout", str(source), str(baseline)], output / "baseline-clone.txt")
            run(["git", "checkout", "--detach", config["baseline_sha"]], output / "baseline-checkout.txt", cwd=baseline)
            revisions = {"baseline": baseline, **revisions}
        binaries = {}
        selections = {}
        for revision, tree in revisions.items():
            evidence = output / revision
            evidence.mkdir()
            build = root / f"build-{revision}-{config['compiler']}"
            command = ["cmake", "-S", str(tree), "-B", str(build), "-G", "Ninja",
                       "-DCMAKE_BUILD_TYPE=Release", f"-DCMAKE_CXX_COMPILER={cxx}",
                       f"-DCMAKE_CXX_FLAGS={config['flags']}", "-DBUILD_SHARED_LIBS=OFF",
                       "-DFIXEDWIDE_BUILD_BENCHMARKS=ON", "-DFIXEDWIDE_BUILD_BATCH_BENCHMARK=ON",
                       "-DFIXEDWIDE_BUILD_RUNTIME_NARROW_BENCHMARK=ON",
                       "-DFIXEDWIDE_BUILD_TESTS=OFF", "-DFIXEDWIDE_BUILD_EXAMPLES=OFF"]
            if config["suite"] == "zen5":
                command.append("-DFIXEDWIDE_BUILD_ZEN5_BENCHMARK=ON")
            run(command, evidence / "configure.txt")
            targets = sorted({"fixedwide_batch_bench", "fixedwide_runtime_narrow_bench", suite_name})
            run(["cmake", "--build", str(build), "--parallel", "2", "--target", *targets], evidence / "build.txt")
            for name in ("CMakeCache.txt", "compile_commands.json"):
                shutil.copy2(build / name, evidence / name)
            for name in targets:
                binary = build / "benchmarks" / name
                run(prefix + [str(binary), "--validate-only"], evidence / f"{name}-validation.txt")
                shutil.copy2(binary, evidence / name)
            binary = build / "benchmarks" / suite_name
            binaries[revision] = binary
            listed = run(prefix + [str(binary), f"--benchmark_filter={config['filter']}",
                                  "--benchmark_list_tests=true"], capture=True)
            selections[revision] = [name for name in listed.splitlines() if name.strip()]
            if not selections[revision]:
                raise ValueError(f"No benchmarks selected for {revision}")
            write_json(evidence / "selected-benchmarks.json", selections[revision])
            run(["ldd", str(binary)], evidence / "linked-libraries.txt")
        if len(selections) == 2 and selections["baseline"] != selections["candidate"]:
            raise ValueError("Cross-revision selections differ; choose a filter with identical workloads")
        # No builds or separate correctness jobs after this point. Each timed
        # executable still validates fixtures before its per-case warm-up.
        # Observations are outside timing; no background poll competes for CPU.
        write_json(output / "before-stabilization.json", observations(cpu))
        time.sleep(config["stabilization_seconds"])
        paired = len(binaries) == 2
        for pair in range(config["repetitions"] if paired else 1):
            order = list(binaries)
            if pair % 2:
                order.reverse()
            for revision in order:
                evidence = output / revision
                tag = f"round-{pair + 1:02d}"
                path = evidence / f"{tag}.json"
                write_json(evidence / f"{tag}-before.json", observations(cpu))
                run(prefix + [str(binaries[revision]), f"--benchmark_filter={config['filter']}",
                              f"--benchmark_repetitions={1 if paired else config['repetitions']}",
                              f"--benchmark_min_time={config['min_time']}",
                              f"--benchmark_min_warmup_time={config['warmup_seconds']}",
                              "--benchmark_enable_random_interleaving=true",
                              "--benchmark_report_aggregates_only=false", "--benchmark_out_format=json",
                              f"--benchmark_out={path}"], evidence / f"{tag}.txt")
                write_json(evidence / f"{tag}-after.json", observations(cpu))
                benchmark_rows(path)
                results.append((revision, pair, path))
        report = summary(results, config, metadata)
        (output / "SUMMARY.md").write_text(report)
        print(report)
        if os.environ.get("GITHUB_STEP_SUMMARY"):
            with Path(os.environ["GITHUB_STEP_SUMMARY"]).open("a") as stream:
                stream.write(report)
    write_json(output / "completion.json", {"status": "complete", "candidate_sha": candidate_sha,
                                             "results": [str(path.relative_to(output)) for _, _, path in results]})


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"Zen 5 qualification failed: {error}", file=sys.stderr)
        output = os.environ.get("ZEN5_OUTPUT")
        if output and Path(output).is_dir():
            write_json(Path(output) / "failure.json", {"status": "failed", "reason": str(error)})
        sys.exit(1)
