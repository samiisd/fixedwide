#!/usr/bin/env bash
# Run from any directory. No baseline update and no wall-clock pass/fail gate.
set -euo pipefail
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$root"
out=${1:-reports/log10-local}
mkdir -p "$out"
out=$(cd "$out" && pwd)
cxx=${CXX:-g++}
iterations=${ITERATIONS:-262144}
repetitions=${REPETITIONS:-11}
{
    git rev-parse HEAD
    git status --short
    "$cxx" --version
    uname -a
    if command -v lscpu >/dev/null; then lscpu; fi
    if command -v ldd >/dev/null; then ldd --version; fi
    printf 'iterations=%s repetitions=%s\n' "$iterations" "$repetitions"
    printf '%s\n' 'Release (-O3 -DNDEBUG), scalar loops, no fast-math; shared host, no frequency lock.'
} > "$out/environment.txt" 2>&1

# Preserve the measured source with the CI artifact, not in the git history.
# The script reports git status so a local dirty checkout is not mislabelled.
git archive --format=tar.gz -o "$out/source.tar.gz" HEAD
runner=()
if command -v taskset >/dev/null && command -v python3 >/dev/null; then
    cpu=$(python3 -c 'import os; print(min(os.sched_getaffinity(0)))')
    runner=(taskset -c "$cpu")
    printf 'affinity_cpu=%s\n' "$cpu" >> "$out/environment.txt"
fi

for backend in native portable; do
    build="build-log10-${backend}"
    extra=()
    defines=()
    if [[ "$backend" == portable ]]; then
        extra=(-DFIXEDWIDE_FORCE_PORTABLE=ON)
        defines=(-DFIXEDWIDE_FORCE_PORTABLE)
    fi
    cmake -S . -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER="$cxx" -DFIXEDWIDE_BUILD_BENCHMARKS=ON \
        -DFIXEDWIDE_BUILD_TESTS=OFF -DFIXEDWIDE_BUILD_EXAMPLES=OFF "${extra[@]}"
    cmake --build "$build" --target fixedwide_log10_bench
    # Compile and RUN the focused suite without linking the library. This also
    # proves that the new public header has no out-of-line kernel dependency.
    "$cxx" -std=c++23 -O2 -fno-exceptions -Wall -Wextra -Wconversion \
        -Wsign-conversion -Wshadow -Wold-style-cast -Werror -Iinclude \
        "${defines[@]}" tests/test_log10.cpp -o "$build/log10_header_only"
    "$build/log10_header_only" | tee "$out/tests-${backend}.txt"
    "${runner[@]}" "$build/benchmarks/fixedwide_log10_bench" "$iterations" "$repetitions" \
        > "$out/samples-${backend}.csv" 2> "$out/accuracy-${backend}.csv"
    "$cxx" -std=c++23 -O3 -DNDEBUG -fno-fast-math -Iinclude "${defines[@]}" \
        -S benchmarks/log10_asm.cpp -o "$out/log10-${backend}.s"
    sha256sum "$build/benchmarks/fixedwide_log10_bench" >> "$out/environment.txt"
done
cat "$out"/tests-*.txt
cat "$out"/accuracy-*.csv
