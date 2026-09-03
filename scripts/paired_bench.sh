#!/usr/bin/env bash
# Paired 0.4-vs-current benchmark. Both trees build the byte-identical
# benchmarks/rounding_bench.cpp with identical flags, then run interleaved with
# the same seeds. Emits raw CSV per implementation; compare_bench.py aggregates.
set -euo pipefail

BASELINE_SRC=${BASELINE_SRC:-/home/mega/local/downloads/fixedwide-0.4.0-nearest-even/fixedwide}
CURRENT_SRC=${CURRENT_SRC:-$(cd "$(dirname "$0")/.." && pwd)}
SCRIPT_SRC=$(cd "$(dirname "$0")/.." && pwd)
CXX_BIN=${CXX_BIN:-clang++}
LABEL=${LABEL:-$("$CXX_BIN" --version | head -1 | tr ' /' '__')}
OUT=${OUT:-$CURRENT_SRC/reports/raw/$LABEL}
SEEDS=${SEEDS:-"1 2 3"}
ITERATIONS=${ITERATIONS:-1048576}
REPETITIONS=${REPETITIONS:-9}
WORK=${WORK:-$(mktemp -d)}

mkdir -p "$OUT"

# The 0.4 tree gates its rounding benchmark behind FIXEDWIDE_BUILD_ORACLE_TESTS
# and hard-codes Clang-only vectorisation flags. Copy it and fix only that build
# glue: rounding_bench.cpp itself stays byte-identical between the two trees.
prepare_baseline() {
    local dst="$WORK/src_baseline"
    cp -r "$BASELINE_SRC" "$dst"
    rm -rf "$dst"/build*
    python3 - "$dst/benchmarks/CMakeLists.txt" <<'EOF'
import sys, pathlib
p = pathlib.Path(sys.argv[1]); t = p.read_text()
t = t.replace("if(FIXEDWIDE_BUILD_ORACLE_TESTS)", "if(TRUE)")
t = t.replace("-fno-vectorize -fno-slp-vectorize",
              "$<IF:$<CXX_COMPILER_ID:GNU>,-fno-tree-vectorize;-fno-tree-slp-vectorize,-fno-vectorize;-fno-slp-vectorize>")
p.write_text(t)
EOF
    echo "$dst"
}

build() { # name src
    local name=$1
    local src=$2
    local dir="$WORK/build_$name"
    cmake -S "$src" -B "$dir" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER="$CXX_BIN" \
        -DFIXEDWIDE_BUILD_BENCHMARKS=ON \
        -DFIXEDWIDE_BUILD_TESTS=OFF \
        -DFIXEDWIDE_BUILD_EXAMPLES=OFF >/dev/null
    cmake --build "$dir" --target fixedwide_rounding_bench -j"$(nproc)" >/dev/null
    echo "$dir/benchmarks/fixedwide_rounding_bench"
}

BASE_SRC=$(prepare_baseline)
BASE_BIN=$(build baseline "$BASE_SRC")
# SELF_CHECK=1 compares the baseline against itself: the resulting spread is the
# measurement noise floor, and no row below it should be called a regression.
if [ "${SELF_CHECK:-0}" = "1" ]; then CURRENT_SRC_BIN_OVERRIDE=$BASE_BIN; fi
CURR_BIN=${CURRENT_SRC_BIN_OVERRIDE:-$(build current "$CURRENT_SRC")}

{
  echo "label=$LABEL"
  echo "compiler=$("$CXX_BIN" --version | head -1)"
  echo "baseline_src=$BASELINE_SRC"
  echo "bench_source_sha256=$(sha256sum "$CURRENT_SRC/benchmarks/rounding_bench.cpp" | cut -d' ' -f1)"
  echo "baseline_bin_sha256=$(sha256sum "$BASE_BIN" | cut -d' ' -f1)"
  echo "current_bin_sha256=$(sha256sum "$CURR_BIN" | cut -d' ' -f1)"
  echo "iterations=$ITERATIONS repetitions=$REPETITIONS seeds=$SEEDS"
  echo "host=$(uname -srm) cpu=$(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2- | xargs)"
  echo "date=$(date -Is)"
} > "$OUT/environment.txt"

# Pin both implementations to the same core. Without this, CPU frequency and
# CCD-placement drift between two sequential runs was worth more than the
# differences being measured.
PIN=${PIN:-taskset -c ${PIN_CPU:-4}}

run_seed() { # bin outfile seed append
    local bin=$1
    local out=$2
    local seed=$3
    local append=$4
    $PIN "$bin" --iterations "$ITERATIONS" --repetitions "$REPETITIONS" --seed "$seed" \
        2>>"$OUT/$(basename "$out" .csv).log" \
        | { if [ "$append" -eq 0 ]; then cat; else tail -n +2; fi; } >> "$out"
}

# Interleave per seed, alternating which implementation goes first, so thermal
# and frequency drift is shared rather than attributed to one side.
: > "$OUT/baseline_0_4.csv"
: > "$OUT/current.csv"
append=0
for seed in $SEEDS; do
    if [ $((append % 2)) -eq 0 ]; then
        run_seed "$BASE_BIN" "$OUT/baseline_0_4.csv" "$seed" "$append"
        run_seed "$CURR_BIN" "$OUT/current.csv" "$seed" "$append"
    else
        run_seed "$CURR_BIN" "$OUT/current.csv" "$seed" "$append"
        run_seed "$BASE_BIN" "$OUT/baseline_0_4.csv" "$seed" "$append"
    fi
    append=$((append + 1))
done

python3 "$SCRIPT_SRC/scripts/compare_bench.py" \
    "$OUT/baseline_0_4.csv" "$OUT/current.csv" "$LABEL" > "$OUT/comparison.csv"
echo "wrote $OUT/comparison.csv"
