#!/usr/bin/env bash
# Measure header parse+instantiate cost and consumer object size against 0.4.
# Medians of N runs after a warmup, same compiler and flags for both trees.
set -euo pipefail
SRC=$(cd "$(dirname "$0")/.." && pwd)
BASELINE_SRC=${BASELINE_SRC:-/home/mega/local/downloads/fixedwide-0.4.0-nearest-even/fixedwide}
CXX_BIN=${CXX_BIN:-clang++}
RUNS=${RUNS:-11}
OUT=${OUT:-$SRC/reports/COMPILE_TIME.md}
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# 0.4 generates config.hpp into its build tree, so give it one.
BASE_GEN="$WORK/base_gen/fixedwide"
mkdir -p "$BASE_GEN"
sed -e "s/@FIXEDWIDE_DECIMALS@/12/g" -e "s/@PROJECT_VERSION@/0.4.0/g" \
    "$BASELINE_SRC/cmake/config.hpp.in" > "$BASE_GEN/config.hpp" 2>/dev/null || true

median() { sort -n | awk '{a[NR]=$1} END {print (NR%2) ? a[(NR+1)/2] : (a[NR/2]+a[NR/2+1])/2}'; }

# One measurement: parse the header and instantiate the workload it implies.
measure() { # label include_line workload include_dirs
    local include_line=$2 workload=$3 dirs=$4
    printf '%s\nint main(){ %s }\n' "$include_line" "$workload" > "$WORK/tu.cpp"
    $CXX_BIN -std=c++23 -O2 $dirs -c "$WORK/tu.cpp" -o "$WORK/tu.o" 2>/dev/null || { echo "SKIP"; return; }
    for _ in $(seq "$RUNS"); do
        local start end
        start=$(date +%s%N)
        $CXX_BIN -std=c++23 -O2 $dirs -c "$WORK/tu.cpp" -o "$WORK/tu.o" 2>/dev/null
        end=$(date +%s%N)
        echo $(( (end - start) / 1000000 ))
    done | median
    stat -c %s "$WORK/tu.o" > "$WORK/objsize"
}

CUR_DIRS="-I$SRC/include"
BASE_DIRS="-I$BASELINE_SRC/include -I$WORK/base_gen"

row() { # name cur_inc cur_work base_inc base_work
    local name=$1
    local c b csz bsz
    c=$(measure c "$2" "$3" "$CUR_DIRS"); csz=$(cat "$WORK/objsize" 2>/dev/null || echo 0)
    b=$(measure b "$4" "$5" "$BASE_DIRS"); bsz=$(cat "$WORK/objsize" 2>/dev/null || echo 0)
    if [ "$c" = "SKIP" ] || [ "$b" = "SKIP" ]; then
        printf '| `%s` | - | - | not comparable | - |\n' "$name"
        return
    fi
    local delta
    delta=$(awk -v c="$c" -v b="$b" 'BEGIN{ if (b==0) print "n/a"; else printf "%+.1f%%", (c-b)/b*100 }')
    printf '| `%s` | %s ms | %s ms | %s | %s / %s bytes |\n' "$name" "$b" "$c" "$delta" "$bsz" "$csz"
}

{
    echo "# Compile-time and object size versus 0.4"
    echo
    echo "Compiler: \`$($CXX_BIN --version | head -1)\`"
    echo "Flags: \`-std=c++23 -O2 -c\`. Median of $RUNS runs after one warmup."
    echo "Host: $(uname -srm)"
    echo "Date: $(date -Is)"
    echo
    echo "Each row compiles a translation unit that includes one header and"
    echo "instantiates the work that header exists for, so the number covers"
    echo "template instantiation and not only parsing."
    echo
    echo '| Include | 0.4 | this version | delta | object size 0.4 / now |'
    echo '|---|---:|---:|---:|---|'
    row "fixed.hpp" \
        '#include <fixedwide/fixed.hpp>' \
        'using T=fixedwide::Fixed64<12>; T a=T::from_raw(3), b=T::from_raw(4); return a<b;' \
        '#include <fixedwide/fixed.hpp>' \
        'auto a=fixedwide::FP64::from_raw(3), b=fixedwide::FP64::from_raw(4); return a<b;'
    row "arithmetic.hpp" \
        '#include <fixedwide/arithmetic.hpp>' \
        'using T=fixedwide::Fixed64<12>; auto r=fixedwide::mul(T::from_raw(3),T::from_raw(4)); return r?0:1;' \
        '#include <fixedwide/arithmetic.hpp>' \
        'auto r=fixedwide::mul(fixedwide::FP64::from_raw(3),fixedwide::FP64::from_raw(4)); return r?0:1;'
    row "chars.hpp" \
        '#include <fixedwide/chars.hpp>' \
        'char b[64]; auto r=fixedwide::to_chars(b,sizeof b,fixedwide::Fixed64<12>{}); return r?0:1;' \
        '#include <fixedwide/chars.hpp>' \
        'char b[64]; auto r=fixedwide::to_chars(b,sizeof b,fixedwide::FP64{}); return r?0:1;'
} > "$OUT"
cat "$OUT"
