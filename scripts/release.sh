#!/usr/bin/env bash
# Build the release archive from the committed tree, then prove it by extracting
# it somewhere else and building and testing it there.
#
# The archive comes from `git archive`, so it contains exactly what is committed
# and nothing from the working tree. That is what the previous release could not
# say: its manifest and its reports described a directory, not an archive.
set -euo pipefail
SRC=$(cd "$(dirname "$0")/.." && pwd)
cd "$SRC"

VERSION=$(sed -n 's/.*FIXEDWIDE_VERSION_STRING "\(.*\)"/\1/p' include/fixedwide/version.hpp)
ZIP="fixedwide-${VERSION}-source.zip"
LOG=reports/fresh_extraction_verification.log

if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "refusing to build a release archive from a dirty tree" >&2
    exit 1
fi

./scripts/make_manifest.sh
if ! git diff --quiet MANIFEST.sha256; then
    echo "MANIFEST.sha256 is out of date; commit the regenerated file first" >&2
    exit 1
fi

rm -f "$ZIP" SHA256SUMS
git archive --format=zip --prefix="fixedwide-${VERSION}/" -o "$ZIP" HEAD
sha256sum "$ZIP" > SHA256SUMS

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
{
    echo "# fresh-extraction verification"
    echo "date:    $(date -Is)"
    echo "version: $VERSION"
    echo "archive: $ZIP"
    echo "sha256:  $(cut -d' ' -f1 SHA256SUMS)"
    echo "commit:  $(git rev-parse HEAD)"
    echo
    echo "## unzip -t"
    unzip -tq "$ZIP" && echo "no errors in compressed data"
    echo
    echo "## manifest verification inside the extracted tree"
} > "$LOG"

unzip -q "$ZIP" -d "$WORK"
tree="$WORK/fixedwide-${VERSION}"
( cd "$tree" && sha256sum -c MANIFEST.sha256 2>&1 | grep -v ': OK$' || true ) >> "$LOG"
echo "$(cd "$tree" && grep -vc '^#' MANIFEST.sha256) files verified, $(cd "$tree" && sha256sum -c MANIFEST.sha256 2>/dev/null | grep -cv ': OK$') failures" >> "$LOG"

{
    echo
    echo "## build and test from the extracted archive"
} >> "$LOG"
(
    cd "$tree"
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DFIXEDWIDE_BUILD_TESTS=ON
    cmake --build build -j"$(nproc)"
    ctest --test-dir build --output-on-failure
) >> "$LOG" 2>&1

if grep -q "100% tests passed" "$LOG"; then
    echo "RESULT: $(grep -oE '100% tests passed out of [0-9]+' "$LOG" | tail -1)" >> "$LOG"
    tail -1 "$LOG"
else
    echo "RESULT: FAILED" >> "$LOG"
    echo "fresh-extraction verification FAILED, see $LOG" >&2
    exit 1
fi
echo "archive: $ZIP"
cat SHA256SUMS
