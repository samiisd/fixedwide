#!/usr/bin/env bash
# Build the pinned mpdecimal C and C++ libraries into an isolated prefix.
#
# Usage:
#   CC=clang-22 CXX=clang++-22 scripts/build_mpdecimal.sh /path/to/prefix
#
# MPDECIMAL_CHECK=1 runs the offline check_local suite before installation.
set -euo pipefail

readonly VERSION=4.0.1
readonly ARCHIVE="mpdecimal-${VERSION}.tar.gz"
readonly URL="https://www.bytereef.org/software/mpdecimal/releases/${ARCHIVE}"
readonly SHA256="96d33abb4bb0070c7be0fed4246cd38416188325f820468214471938545b1ac8"

if [[ $# -ne 1 ]]; then
    echo "usage: $0 INSTALL_PREFIX" >&2
    exit 2
fi

PREFIX=$(mkdir -p "$1" && cd "$1" && pwd)
WORK=${MPDECIMAL_WORK:-"${TMPDIR:-/tmp}/fixedwide-mpdecimal-${VERSION}"}
JOBS=${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}

rm -rf "$WORK"
mkdir -p "$WORK"
cd "$WORK"

curl --fail --location --silent --show-error --output "$ARCHIVE" "$URL"
printf '%s  %s\n' "$SHA256" "$ARCHIVE" | sha256sum --check --strict -
tar -xzf "$ARCHIVE"
cd "mpdecimal-${VERSION}"

./configure \
    CC="${CC:-cc}" \
    CXX="${CXX:-c++}" \
    --prefix="$PREFIX" \
    --enable-static \
    --disable-doc

make -j"$JOBS"
if [[ ${MPDECIMAL_CHECK:-0} == 1 ]]; then
    make check_local
fi
make install

# Make discovery failures obvious before CMake is involved. Only inspect
# directories that actually exist: under `pipefail`, passing a missing lib64
# directory to find would turn a successful install into a false failure.
test -f "$PREFIX/include/decimal.hh"
found_c=0
found_cpp=0
for libdir in "$PREFIX/lib" "$PREFIX/lib64"; do
    [[ -d "$libdir" ]] || continue
    while IFS= read -r library; do
        printf '%s\n' "$library"
        if [[ "$library" == *libmpdec++* ]]; then
            found_cpp=1
        elif [[ "$library" == *libmpdec.* ]]; then
            found_c=1
        fi
    done < <(find "$libdir" -maxdepth 1 -type f \
        \( -name 'libmpdec.*' -o -name 'libmpdec++.*' \) -print | sort)
done
[[ $found_c -eq 1 && $found_cpp -eq 1 ]]

