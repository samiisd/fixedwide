#!/usr/bin/env bash
# Measure retired instructions per operation, deterministically.
#
# Wall-clock on a shared CI runner cannot support a 3% gate; the neighbour noise
# is larger than the signal. Instruction counts can: Valgrind executes the same
# binary on the same input in exactly the same number of instructions, every
# run, on every machine with the same compiler.
#
# Method: run each workload twice, at N and 2N iterations, and report
# (I(2N) - I(N)) / N. Everything that does not scale with the iteration count --
# process startup, dynamic loading, page faults, the fixture setup, the final
# print -- appears identically in both runs and cancels exactly. What is left is
# the marginal cost of one operation.
#
#   scripts/icount.sh                       measure, write CSV to stdout
#   scripts/icount.sh --update              measure and overwrite the baseline
#   scripts/icount.sh --binary path/to/bin  use an already-built binary
set -euo pipefail

SRC=$(cd "$(dirname "$0")/.." && pwd)
ITERATIONS=${ITERATIONS:-20000}
BINARY=""
UPDATE=0

while [ $# -gt 0 ]; do
    case "$1" in
        --update) UPDATE=1; shift ;;
        --binary) BINARY=$2; shift 2 ;;
        --iterations) ITERATIONS=$2; shift 2 ;;
        *) echo "unknown argument: $1" >&2; exit 2 ;;
    esac
done

command -v valgrind >/dev/null || { echo "valgrind is required" >&2; exit 1; }

# Build it if we were not handed one.
if [ -z "$BINARY" ]; then
    BUILD=${BUILD:-$SRC/build_icount}
    cmake -S "$SRC" -B "$BUILD" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DFIXEDWIDE_BUILD_BENCHMARKS=ON \
        -DFIXEDWIDE_BUILD_TESTS=OFF \
        -DFIXEDWIDE_BUILD_EXAMPLES=OFF >/dev/null
    cmake --build "$BUILD" --target fixedwide_icount -j"$(nproc)" >/dev/null
    BINARY="$BUILD/benchmarks/fixedwide_icount"
fi

# The label names the toolchain, because instruction counts are compiler
# specific: a baseline recorded under GCC says nothing about Clang.
compiler_label() {
    local version
    version=$("${CXX:-c++}" --version | head -1)
    case "$version" in
        *clang*) echo "clang-$(echo "$version" | grep -oE '[0-9]+' | head -1)" ;;
        *)       echo "gcc-$(echo "$version" | grep -oE '[0-9]+\.[0-9]+' | head -1 | cut -d. -f1)" ;;
    esac
}
LABEL=${LABEL:-$(uname -m)-$(compiler_label)}
BASELINE=${BASELINE:-$SRC/benchmarks/baseline/$LABEL.csv}

# One run under callgrind. Its "refs:" line is the retired instruction count.
instructions() { # workload iterations
    valgrind --tool=callgrind --callgrind-out-file=/dev/null \
             "$BINARY" "$1" "$2" 2>&1 >/dev/null \
        | grep -oE 'refs: *[0-9,]+' | tr -d ' ,' | cut -d: -f2
}

{
    echo "workload,instructions_per_op"
    "$BINARY" --list | while read -r workload; do
        low=$(instructions "$workload" "$ITERATIONS")
        high=$(instructions "$workload" $((ITERATIONS * 2)))
        [ -n "$low" ] && [ -n "$high" ] || { echo "no callgrind output for $workload" >&2; exit 1; }
        awk -v w="$workload" -v l="$low" -v h="$high" -v n="$ITERATIONS" \
            'BEGIN { printf "%s,%.3f\n", w, (h - l) / n }'
    done
} > "${TMPDIR:-/tmp}/icount.$$.csv"

if [ "$UPDATE" -eq 1 ]; then
    mkdir -p "$(dirname "$BASELINE")"
    cp "${TMPDIR:-/tmp}/icount.$$.csv" "$BASELINE"
    echo "wrote $BASELINE" >&2
fi

cat "${TMPDIR:-/tmp}/icount.$$.csv"
rm -f "${TMPDIR:-/tmp}/icount.$$.csv"
