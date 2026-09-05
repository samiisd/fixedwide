#!/usr/bin/env bash
# Run a configured Clang coverage build and retain matching profiles/exports.
# Usage: LLVM_COV=llvm-cov-20 LLVM_PROFDATA=llvm-profdata-20 \
#          scripts/coverage.sh build-native reports/coverage/native native
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
build=$(cd "${1:?build directory required}" && pwd)
mkdir -p "${2:?output directory required}"
out=$(cd "$2" && pwd)
backend=${3:?native or portable required}
[[ "$backend" == native || "$backend" == portable ]]
cov=${LLVM_COV:-llvm-cov}
profdata=${LLVM_PROFDATA:-llvm-profdata}
[[ -x "$build/tests/test_coverage" ]]
# Reusing a coverage directory must not merge counters from an older binary.
rm -rf "$out/profiles" "$out/suite-profiles"
mkdir -p "$out/profiles" "$out/suite-profiles"
{
    printf 'source_commit='; git -C "$root" rev-parse HEAD
    printf 'working_tree_changes=\n'; git -C "$root" status --short
    printf 'backend=%s\n' "$backend"
    "$cov" --version
    "$profdata" --version
    sha256sum "$build/tests/test_coverage"
} > "$out/environment.txt"
LLVM_PROFILE_FILE="$out/suite-profiles/%m-%p.profraw" \
    ctest --test-dir "$build" --output-on-failure -j2 2>&1 | tee "$out/ctest.log"
LLVM_PROFILE_FILE="$out/profiles/%m-%p.profraw" \
    "$build/tests/test_coverage" all 2>&1 | tee "$out/conformance.log"
shopt -s nullglob
profiles=("$out/profiles/"*.profraw)
[[ ${#profiles[@]} -gt 0 ]]
"$profdata" merge -sparse "${profiles[@]}" -o "$out/coverage.profdata"
# All project implementation sources/headers; no per-function or per-branch
# omissions. Test code, dependencies and standard-library code are not product.
mapfile -t sources < <(find "$root/src" "$root/include/fixedwide" -type f \( -name '*.hpp' -o -name '*.cpp' \) | sort)
args=("$build/tests/test_coverage" "-instr-profile=$out/coverage.profdata" -sources "${sources[@]}")
"$cov" report "${args[@]}" > "$out/llvm-summary.txt" 2> "$out/llvm-summary.stderr"
"$cov" export -format=lcov "${args[@]}" > "$out/coverage.lcov" 2> "$out/lcov.stderr"
"$cov" export -skip-functions "${args[@]}" > "$out/llvm-export.json" 2> "$out/llvm-export.stderr"
# A coverage-tool warning must be investigated, not masked by a pipe to tee.
if grep -Ei 'mismatch|error:|failed to load' "$out/"*.stderr; then
    echo "coverage map/profile mismatch or tool error" >&2
    exit 1
fi
python3 "$root/scripts/coverage_report.py" \
    --lcov "$out/coverage.lcov" --llvm-json "$out/llvm-export.json" \
    --source-root "$root" --backend "$backend" --output "$out" \
    --min-source-lines 99 --min-llvm-lines 95 --min-llvm-branches 90
