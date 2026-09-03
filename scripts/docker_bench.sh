#!/usr/bin/env bash
# Run the paired 0.4 comparison on the audit's compilers, in a pinned container.
#
# Clang only. The 0.4 baseline hard-requires C++ _BitInt(256) and refuses to
# configure without it, and GCC does not implement _BitInt in C++ -- so there is
# no GCC build of 0.4 to compare against. GCC is covered for correctness in CI
# and in the execution matrix, but no paired GCC performance row can exist
# without modifying the baseline, which would stop it being the baseline.
# Results land in reports/raw/<compiler>/ on the host.
set -euo pipefail
SRC=$(cd "$(dirname "$0")/.." && pwd)
BASELINE_SRC=${BASELINE_SRC:-/home/mega/local/downloads/fixedwide-0.4.0-nearest-even/fixedwide}
IMAGE=${IMAGE:-fixedwide-bench:ubuntu24.04}
COMPILERS=${COMPILERS:-"clang++-17 clang++-18"}

# Clang 17 and 18 report __cpp_concepts=201907; libstdc++ gates <expected> on
# 202002, so std::expected is unreachable for them with the system library.
# Both the 0.4 baseline and the current tree need it. Correcting the macro keeps
# every compiler on the SAME standard library, which matters more for a paired
# comparison than the macro's tidiness -- and both sides of each row get it.
clang_fix() { case "$1" in clang*) echo "-U__cpp_concepts -D__cpp_concepts=202002L";; *) echo "";; esac; }

DOCKER_BUILDKIT=0 docker build -q -t "$IMAGE" -f "$SRC/scripts/Dockerfile.bench" "$SRC/scripts" >/dev/null

for cxx in $COMPILERS; do
    label=$(echo "$cxx" | tr -d '+' )
    CLANG_EXPECTED_FIX=$(clang_fix "$cxx")
    docker run --rm \
        -v "$SRC:/src:ro" -v "$BASELINE_SRC:/baseline:ro" \
        -v "$SRC/reports/raw:/out" \
        -e CXX_BIN="$cxx" -e LABEL="$label" -e OUT="/out/$label" \
        -e CXXFLAGS="$CLANG_EXPECTED_FIX" \
        -e BASELINE_SRC=/baseline -e SEEDS="${SEEDS:-1 2 3}" \
        "$IMAGE" bash -c '
            set -e
            cp -r /src /work && cd /work
            rm -rf build*
            CURRENT_SRC=/work ./scripts/paired_bench.sh
        ' 2>&1 | grep -E "^#|wrote" || echo "$label: FAILED"
done
