#!/usr/bin/env bash
# Run every configuration the release claims, record what actually happened.
#
# The point is the execution matrix: a row may only say executed-pass if this
# script ran it and it passed. Anything this script cannot run is reported as
# configured-not-executed or not-configured, never as supported.
set -uo pipefail
SRC=$(cd "$(dirname "$0")/.." && pwd)
WORK=${WORK:-$SRC/build_verify}
MATRIX=$SRC/reports/EXECUTION_MATRIX.csv
LOG=$SRC/reports/verification.log
mkdir -p "$(dirname "$MATRIX")"
: > "$LOG"
echo "configuration,status,detail" > "$MATRIX"

# Quote every field: configuration names contain commas.
record() { printf '"%s",%s,"%s"\n' "$1" "$2" "$3" >> "$MATRIX"; printf '%-52s %s (%s)\n' "$1" "$2" "$3"; }

run_config() { # name cxx extra_cmake_args...
    local name=$1; shift
    local cxx=$1; shift
    local dir="$WORK/$(echo "$name" | tr ' /+' '___')"
    rm -rf "$dir"
    {
        echo "=== $name ==="
        cmake -S "$SRC" -B "$dir" -G Ninja -DCMAKE_BUILD_TYPE=Release \
              -DCMAKE_CXX_COMPILER="$cxx" -DFIXEDWIDE_BUILD_TESTS=ON "$@" 2>&1 &&
        cmake --build "$dir" -j"$(nproc)" 2>&1
    } >> "$LOG" 2>&1 || { record "$name" "executed-fail" "build failed, see reports/verification.log"; return; }
    # ctest prints "100% tests passed out of N" when nothing fails, and
    # "X% tests passed, F tests failed out of N" otherwise. Match both.
    local out total
    out=$(ctest --test-dir "$dir" 2>&1 | tee -a "$LOG" | grep -E "tests passed" | tail -1)
    total=$(echo "$out" | grep -oE "out of [0-9]+" | grep -oE "[0-9]+")
    if echo "$out" | grep -q "100% tests passed"; then
        record "$name" "executed-pass" "${total}/${total} tests"
    else
        record "$name" "executed-fail" "${out:-no ctest summary}"
    fi
}

CLANG=${CLANG:-clang++}
GCC=${GCC:-g++}

run_config "Linux x86-64 $($CLANG --version | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1) Release" "$CLANG"
run_config "Linux x86-64 GCC $($GCC -dumpversion) Release" "$GCC"
run_config "Linux x86-64 Clang forced portable" "$CLANG" -DFIXEDWIDE_FORCE_PORTABLE=ON
run_config "Linux x86-64 Clang portable, __SIZEOF_INT128__ undefined" "$CLANG" \
           -DFIXEDWIDE_FORCE_PORTABLE=ON -DCMAKE_CXX_FLAGS=-U__SIZEOF_INT128__
run_config "Linux x86-64 Clang ASan+UBSan native" "$CLANG" -DFIXEDWIDE_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug
run_config "Linux x86-64 Clang ASan+UBSan forced portable" "$CLANG" \
           -DFIXEDWIDE_SANITIZE=ON -DFIXEDWIDE_FORCE_PORTABLE=ON -DCMAKE_BUILD_TYPE=Debug
run_config "Linux x86-64 Clang shared library" "$CLANG" -DBUILD_SHARED_LIBS=ON

# Library-only build without exceptions or RTTI: no test binaries, so this is a
# build check and is recorded as one.
dir="$WORK/no_eh"; rm -rf "$dir"
if cmake -S "$SRC" -B "$dir" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="$CLANG" \
      -DFIXEDWIDE_BUILD_TESTS=OFF -DFIXEDWIDE_BUILD_EXAMPLES=OFF \
      -DCMAKE_CXX_FLAGS="-fno-exceptions -fno-rtti" >>"$LOG" 2>&1 &&
   cmake --build "$dir" -j"$(nproc)" >>"$LOG" 2>&1; then
    record "Linux x86-64 Clang no-exceptions / no-RTTI library" "executed-pass" "library builds; no test binaries in this configuration"
else
    record "Linux x86-64 Clang no-exceptions / no-RTTI library" "executed-fail" "see reports/verification.log"
fi

# Install, then build a standalone consumer that sees only the installed package.
stage="$WORK/stage"; rm -rf "$stage" "$WORK/install" "$WORK/consumer"
if cmake -S "$SRC" -B "$WORK/install" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="$CLANG" \
       -DCMAKE_INSTALL_PREFIX="$stage" -DFIXEDWIDE_BUILD_TESTS=OFF -DFIXEDWIDE_BUILD_EXAMPLES=OFF >>"$LOG" 2>&1 &&
   cmake --build "$WORK/install" --target install >>"$LOG" 2>&1 &&
   cmake -S "$SRC/examples/consumer" -B "$WORK/consumer" -DCMAKE_PREFIX_PATH="$stage" \
       -DCMAKE_CXX_COMPILER="$CLANG" >>"$LOG" 2>&1 &&
   cmake --build "$WORK/consumer" >>"$LOG" 2>&1; then
    value=$("$WORK/consumer/consumer")
    record "CMake install + external find_package consumer" "executed-pass" "consumer printed $value"
else
    record "CMake install + external find_package consumer" "executed-fail" "see reports/verification.log"
fi

echo
echo "matrix written to $MATRIX"
