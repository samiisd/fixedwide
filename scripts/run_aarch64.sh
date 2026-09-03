#!/usr/bin/env bash
# Cross-build the test suite for AArch64 and RUN it on real hardware over adb.
#
# This is executed evidence, not a CI file: the binaries are statically linked
# and run on an actual arm64 device, so the AArch64 row of the execution matrix
# is executed-pass rather than configured-not-executed. Timings are not taken
# here - the device is a phone under thermal control - only correctness.
set -euo pipefail

SRC=$(cd "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-$SRC/build_aarch64}
DEVICE_DIR=${DEVICE_DIR:-/data/local/tmp/fixedwide}
LOG=${LOG:-$SRC/reports/aarch64_execution.log}

cmake -S "$SRC" -B "$BUILD" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$SRC/cmake/aarch64-linux-gnu.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DFIXEDWIDE_BUILD_TESTS=ON \
    -DFIXEDWIDE_BUILD_EXAMPLES=OFF >/dev/null
cmake --build "$BUILD" -j"$(nproc)" >/dev/null

mkdir -p "$(dirname "$LOG")"
{
    echo "# fixedwide AArch64 execution on real hardware"
    echo "date: $(date -Is)"
    echo "compiler: $(aarch64-linux-gnu-g++ --version | head -1)"
    echo "device: $(adb shell getprop ro.product.model | tr -d '\r') / $(adb shell uname -m | tr -d '\r')"
    echo "kernel: $(adb shell uname -sr | tr -d '\r')"
    echo
} > "$LOG"

adb shell "mkdir -p $DEVICE_DIR"
pass=0; fail=0
for binary in "$BUILD"/tests/test_* "$BUILD"/tests/audit_*; do
    [ -x "$binary" ] || continue
    name=$(basename "$binary")
    adb push "$binary" "$DEVICE_DIR/$name" >/dev/null
    if adb shell "chmod +x $DEVICE_DIR/$name && $DEVICE_DIR/$name" >> "$LOG" 2>&1; then
        echo "PASS $name" | tee -a "$LOG"
        pass=$((pass + 1))
    else
        echo "FAIL $name" | tee -a "$LOG"
        fail=$((fail + 1))
    fi
done
adb shell "rm -rf $DEVICE_DIR" >/dev/null 2>&1 || true

echo "aarch64: $pass passed, $fail failed" | tee -a "$LOG"
[ "$fail" -eq 0 ]
