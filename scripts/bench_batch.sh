#!/usr/bin/env bash
# One Google Benchmark SoA comparison, with correctness checked before timing.
# Reserve a CPU separately; this script pins to the explicit caller selection.
set -euo pipefail

batch_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$batch_root"
batch_out=reports/batch-local
batch_cpu=${BATCH_CPU:-}
batch_validate_only=false
batch_output_seen=false

usage() {
    cat <<'USAGE'
Usage: scripts/bench_batch.sh [--validate-only] [--cpu CPU] [OUTPUT_DIRECTORY]

Without an explicit CPU, only build and untimed fixture validation run.
Timing requires --cpu CPU or BATCH_CPU=CPU after reserving that CPU.

Environment:
  CXX                       Compiler (default: g++)
  BATCH_BACKEND             native or portable (default: native; one build)
  BATCH_CPU                 Explicit CPU number for taskset
  BATCH_FILTER              Google Benchmark filter (default: .*)
  BATCH_BUILD_DIR           Optional existing/new CMake build directory
  BUILD_JOBS                Build parallelism (default: 2)
  MIN_TIME                  Per-case Google Benchmark minimum (default: 0.05s)
  REPETITIONS               Google Benchmark repetitions (default: 3)
  GOOGLE_BENCHMARK_SOURCE   Optional offline checkout for the pinned dependency
USAGE
}

while (($#)); do
    case "$1" in
        --validate-only) batch_validate_only=true; shift ;;
        --cpu)
            if (($# < 2)); then usage >&2; exit 2; fi
            batch_cpu=$2
            shift 2
            ;;
        --help|-h) usage; exit 0 ;;
        --*) printf 'Unknown option: %s\n' "$1" >&2; exit 2 ;;
        *)
            if "$batch_output_seen"; then usage >&2; exit 2; fi
            batch_out=$1
            batch_output_seen=true
            shift
            ;;
    esac
done

if [[ -n "$batch_cpu" && ! "$batch_cpu" =~ ^[0-9]+$ ]]; then
    printf 'CPU must be one nonnegative integer, received: %s\n' "$batch_cpu" >&2
    exit 2
fi
batch_cxx=${CXX:-g++}
batch_backend=${BATCH_BACKEND:-native}
batch_min_time=${MIN_TIME:-0.05s}
batch_repetitions=${REPETITIONS:-3}
batch_filter=${BATCH_FILTER:-.*}
batch_build=${BATCH_BUILD_DIR:-build-batch-$(basename "$batch_cxx")-${batch_backend}}
batch_extra=(-DFIXEDWIDE_FORCE_PORTABLE=OFF)
case "$batch_backend" in
    native) ;;
    portable) batch_extra=(-DFIXEDWIDE_FORCE_PORTABLE=ON) ;;
    *) printf 'BATCH_BACKEND must be native or portable.\n' >&2; exit 2 ;;
esac
if [[ -n "${GOOGLE_BENCHMARK_SOURCE:-}" ]]; then
    batch_extra+=("-DFETCHCONTENT_SOURCE_DIR_GOOGLEBENCHMARK=$GOOGLE_BENCHMARK_SOURCE")
fi

mkdir -p "$batch_out"
batch_out=$(cd "$batch_out" && pwd)
{
    git rev-parse HEAD
    git status --short
    "$batch_cxx" --version
    uname -a
    if command -v lscpu >/dev/null; then lscpu; fi
    printf 'backend=%s requested_cpu=%s\n' "$batch_backend" "${batch_cpu:-none}"
    printf 'benchmark_min_time=%s benchmark_repetitions=%s filter=%s random_interleaving=true\n' \
        "$batch_min_time" "$batch_repetitions" "$batch_filter"
    printf '%s\n' 'Release (-O3 -DNDEBUG), compiler vectorization enabled, strict FP, no -march=native; no frequency lock.'
} > "$batch_out/environment.txt" 2>&1

cmake -S . -B "$batch_build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER="$batch_cxx" -DBUILD_SHARED_LIBS=OFF \
    -DFIXEDWIDE_BUILD_BENCHMARKS=ON -DFIXEDWIDE_BUILD_BATCH_BENCHMARK=ON \
    -DFIXEDWIDE_BUILD_RUNTIME_NARROW_BENCHMARK=OFF \
    -DFIXEDWIDE_BUILD_TESTS=OFF -DFIXEDWIDE_BUILD_EXAMPLES=OFF "${batch_extra[@]}"
cmake --build "$batch_build" --target fixedwide_batch_bench --parallel "${BUILD_JOBS:-2}"
ninja -C "$batch_build" -t commands fixedwide_batch_bench >> "$batch_out/environment.txt"
batch_binary="$batch_build/benchmarks/fixedwide_batch_bench"
"$batch_binary" --validate-only > "$batch_out/validation.txt" 2>&1
cat "$batch_out/validation.txt"

if "$batch_validate_only" || [[ -z "$batch_cpu" ]]; then
    printf 'Build and untimed validation complete: %s\n' "$batch_out"
    if [[ -z "$batch_cpu" ]] && ! "$batch_validate_only"; then
        printf 'For timing, select a reserved CPU with --cpu CPU or BATCH_CPU=CPU.\n'
    fi
    exit 0
fi

if ! command -v taskset >/dev/null; then
    printf 'Timing requires taskset to apply the explicit CPU selection.\n' >&2
    exit 2
fi
# Reject an unavailable CPU before starting Google Benchmark. taskset is
# affinity control, not an assertion that another process cannot use the CPU.
taskset -c "$batch_cpu" true
taskset -c "$batch_cpu" "$batch_binary" \
    "--benchmark_filter=$batch_filter" \
    "--benchmark_min_time=$batch_min_time" "--benchmark_repetitions=$batch_repetitions" \
    --benchmark_min_warmup_time=0.02 --benchmark_enable_random_interleaving=true \
    --benchmark_report_aggregates_only=false --benchmark_display_aggregates_only=true \
    --benchmark_out_format=json "--benchmark_out=$batch_out/benchmark.json" \
    > "$batch_out/benchmark.txt" 2> "$batch_out/benchmark-stderr.txt"
printf 'Benchmark results: %s\n' "$batch_out"
