#!/usr/bin/env bash
# Format the project's own C++, or check that it is already formatted.
#
#   scripts/format.sh          rewrite files in place
#   scripts/format.sh --check  exit non-zero if anything would change (CI)
#
# The exclusion list lives here rather than in .clang-format-ignore because
# clang-format does not honour that file when it is handed an explicit path,
# which is how both this script and every editor invoke it.
set -euo pipefail
SRC=$(cd "$(dirname "$0")/.." && pwd)
cd "$SRC"

# Pinned, because clang-format major versions format the same file differently
# and a mismatch turns this check into noise. CI installs exactly this version
# from PyPI (`pip install clang-format==22.1.8`), which is the same binary on
# every platform; set CLANG_FORMAT to override.
readonly REQUIRED_VERSION=22
CLANG_FORMAT=${CLANG_FORMAT:-clang-format}
command -v "$CLANG_FORMAT" >/dev/null || { echo "clang-format not found; pip install clang-format==22.1.8" >&2; exit 1; }

actual=$("$CLANG_FORMAT" --version | grep -oE '[0-9]+' | head -1)
if [ "$actual" != "$REQUIRED_VERSION" ]; then
    echo "clang-format $actual found, but this project is formatted with $REQUIRED_VERSION." >&2
    echo "Different majors disagree; install it with: pip install clang-format==22.1.8" >&2
    exit 1
fi

# Not formatted, and each for a reason:
#
#   benchmarks/rounding_bench.cpp  the paired 0.4 comparison compiles this exact
#                                  file against both trees and depends on it
#                                  being byte-identical to 0.4's copy. Formatting
#                                  it would invalidate every row in
#                                  reports/BENCHMARK_VS_0_4.md without any
#                                  visible failure.
#   benchmarks/reference/*         imported verbatim from an earlier release as
#                                  evidence, not as maintained source.
#   tests/audit_*.cpp              deliberately dense single-line drivers,
#                                  imported alongside the results they produced.
excluded() {
    case "$1" in
        benchmarks/rounding_bench.cpp|benchmarks/reference/*|tests/audit_*.cpp) return 0 ;;
        *) return 1 ;;
    esac
}

files=()
while IFS= read -r f; do
    excluded "$f" || files+=("$f")
done < <(git ls-files '*.cpp' '*.hpp')

if [ "${1:-}" = "--check" ]; then
    bad=()
    for f in "${files[@]}"; do
        "$CLANG_FORMAT" "$f" | diff -q "$f" - >/dev/null || bad+=("$f")
    done
    if [ ${#bad[@]} -gt 0 ]; then
        printf 'not formatted (%d files):\n' "${#bad[@]}" >&2
        printf '  %s\n' "${bad[@]}" >&2
        echo >&2
        # Keep the gate failing, but expose the exact correction in its log.
        # diff returns 1 for the expected mismatch; this is not a test pass.
        for f in "${bad[@]}"; do
            "$CLANG_FORMAT" "$f" | diff -u --label "$f" --label "formatted/$f" "$f" - || true
        done
        echo "run scripts/format.sh to fix" >&2
        exit 1
    fi
    echo "${#files[@]} files formatted correctly"
else
    "$CLANG_FORMAT" -i "${files[@]}"
    echo "formatted ${#files[@]} files"
fi
