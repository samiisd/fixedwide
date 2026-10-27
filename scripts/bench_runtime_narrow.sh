#!/usr/bin/env bash
# Paired direct-kernel/public-API timings, never a wall-clock pass/fail gate.
set -euo pipefail
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$root"
out=${1:-reports/runtime-narrow-local}
mkdir -p "$out"
out=$(cd "$out" && pwd)
cxx=${CXX:-g++}
iterations=${ITERATIONS:-1048576}
repetitions=${REPETITIONS:-7}
{
    git rev-parse HEAD
    git status --short
    "$cxx" --version
    uname -a
    if command -v lscpu >/dev/null; then lscpu; fi
    printf 'iterations=%s repetitions=%s\n' "$iterations" "$repetitions"
    printf '%s\n' 'Release (-O3 -DNDEBUG), scalar loops, no fast-math; no frequency lock.'
    sha256sum benchmarks/runtime_narrow_bench.cpp include/fixedwide/detail/mixed_native.hpp \
        include/fixedwide/mixed.hpp src/mixed.cpp
} > "$out/environment.txt" 2>&1

runner=()
if command -v taskset >/dev/null && command -v python3 >/dev/null; then
    cpu=$(python3 -c 'import os; print(min(os.sched_getaffinity(0)))')
    runner=(taskset -c "$cpu")
    printf 'affinity_cpu=%s\n' "$cpu" >> "$out/environment.txt"
fi

for backend in native portable; do
    build="build-runtime-narrow-${backend}"
    extra=(-DFIXEDWIDE_FORCE_PORTABLE=OFF)
    if [[ "$backend" == portable ]]; then extra=(-DFIXEDWIDE_FORCE_PORTABLE=ON); fi
    cmake -S . -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER="$cxx" -DFIXEDWIDE_BUILD_BENCHMARKS=ON \
        -DFIXEDWIDE_BUILD_TESTS=OFF -DFIXEDWIDE_BUILD_EXAMPLES=OFF "${extra[@]}"
    cmake --build "$build" --target fixedwide_runtime_narrow_bench
    "${runner[@]}" "$build/benchmarks/fixedwide_runtime_narrow_bench" "$iterations" "$repetitions" \
        > "$out/samples-${backend}.csv" 2> "$out/validation-${backend}.csv"
    sha256sum "$build/benchmarks/fixedwide_runtime_narrow_bench" >> "$out/environment.txt"
done
