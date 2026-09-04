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

CLANG_FORMAT=${CLANG_FORMAT:-clang-format}
command -v "$CLANG_FORMAT" >/dev/null || { echo "clang-format not found" >&2; exit 1; }

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
        echo "run scripts/format.sh to fix" >&2
        exit 1
    fi
    echo "${#files[@]} files formatted correctly"
else
    "$CLANG_FORMAT" -i "${files[@]}"
    echo "formatted ${#files[@]} files"
fi
