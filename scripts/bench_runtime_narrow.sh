#!/usr/bin/env bash
# Google Benchmark direct-kernel/public-API comparisons; no timing pass/fail gate.
set -euo pipefail
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$root"
out=${1:-reports/runtime-narrow-local}
mkdir -p "$out"
out=$(cd "$out" && pwd)
cxx=${CXX:-g++}
min_time=${MIN_TIME:-0.1s}
repetitions=${REPETITIONS:-9}
hash_command=(sha256sum)
if ! command -v sha256sum >/dev/null; then hash_command=(shasum -a 256); fi
{
    git rev-parse HEAD
    git status --short
    "$cxx" --version
    uname -a
    if command -v lscpu >/dev/null; then lscpu; fi
    printf 'benchmark_min_time=%s benchmark_repetitions=%s random_interleaving=true\n' "$min_time" "$repetitions"
    printf '%s\n' 'Release (-O3 -DNDEBUG), scalar loops, no fast-math; no frequency lock.'
    "${hash_command[@]}" benchmarks/runtime_narrow_bench.cpp benchmarks/CMakeLists.txt \
        include/fixedwide/detail/mixed_native.hpp include/fixedwide/mixed.hpp src/mixed.cpp
} > "$out/environment.txt" 2>&1

runner=()
if command -v taskset >/dev/null && command -v python3 >/dev/null; then
    cpu=$(python3 -c 'import os; print(min(os.sched_getaffinity(0)))')
    runner=(taskset -c "$cpu")
    printf 'affinity_cpu=%s\n' "$cpu" >> "$out/environment.txt"
fi

for backend in native portable; do
    build="build-runtime-narrow-$(basename "$cxx")-${backend}"
    extra=(-DFIXEDWIDE_FORCE_PORTABLE=OFF)
    if [[ "$backend" == portable ]]; then extra=(-DFIXEDWIDE_FORCE_PORTABLE=ON); fi
    if [[ -n "${GOOGLE_BENCHMARK_SOURCE:-}" ]]; then
        extra+=("-DFETCHCONTENT_SOURCE_DIR_GOOGLEBENCHMARK=$GOOGLE_BENCHMARK_SOURCE")
    fi
    cmake -S . -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER="$cxx" -DBUILD_SHARED_LIBS=OFF -DFIXEDWIDE_BUILD_BENCHMARKS=ON \
        -DFIXEDWIDE_BUILD_RUNTIME_NARROW_BENCHMARK=ON \
        -DFIXEDWIDE_BUILD_TESTS=OFF -DFIXEDWIDE_BUILD_EXAMPLES=OFF "${extra[@]}"
    cmake --build "$build" --target fixedwide_runtime_narrow_bench
    ninja -C "$build" -t commands fixedwide_runtime_narrow_bench >> "$out/environment.txt"
    "${runner[@]}" "$build/benchmarks/fixedwide_runtime_narrow_bench" \
        "--benchmark_min_time=$min_time" "--benchmark_repetitions=$repetitions" \
        --benchmark_min_warmup_time=0.02 --benchmark_enable_random_interleaving=true \
        --benchmark_report_aggregates_only=false --benchmark_display_aggregates_only=true \
        --benchmark_out_format=json "--benchmark_out=$out/benchmark-${backend}.json" \
        > "$out/benchmark-${backend}.txt" 2> "$out/validation-${backend}.txt"
    "${hash_command[@]}" "$build/benchmarks/fixedwide_runtime_narrow_bench" >> "$out/environment.txt"
    "${hash_command[@]}" "$build/libfixedwide.a" >> "$out/environment.txt"
done
