#!/usr/bin/env bash
# Build and test on a big-endian machine, under emulation.
#
# binary.hpp has always implemented both byte orders, and until this script
# existed only one of them had ever been executed: every host, CI runner and
# phone this library had run on was little-endian. `to_bytes<endian::big>` was
# shipped, documented and untested, which is the worst combination.
#
# s390x is the only big-endian target with a current distro image, so that is
# what this uses. It is emulated and therefore slow; it is correctness only, and
# no timing is taken from it.
#
#   scripts/test_big_endian.sh
#
# Requires docker with binfmt/qemu registered for s390x. On CI that is
# docker/setup-qemu-action; locally, `docker run --privileged --rm
# tonistiigi/binfmt --install s390x` once.
set -euo pipefail
SRC=$(cd "$(dirname "$0")/.." && pwd)
IMAGE=${IMAGE:-s390x/ubuntu:24.04}

docker run --rm --platform linux/s390x -v "$SRC:/src:ro" "$IMAGE" bash -c '
    set -e
    apt-get update -qq >/dev/null 2>&1
    apt-get install -y -qq --no-install-recommends g++-14 cmake ninja-build >/dev/null 2>&1

    # Prove the emulation really is big-endian before trusting anything it says.
    cat > /tmp/endian.cpp <<CPP
#include <bit>
#include <cstdio>
int main() {
    const bool big = std::endian::native == std::endian::big;
    std::printf("%s\n", big ? "big" : "little");
    return big ? 0 : 1;
}
CPP
    g++-14 -std=c++23 /tmp/endian.cpp -o /tmp/endian
    /tmp/endian || { echo "not a big-endian target; refusing to report a pass" >&2; exit 1; }

    cp -r /src /work && cd /work && rm -rf build*
    cmake -B /tmp/build -G Ninja -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_CXX_COMPILER=g++-14 -DFIXEDWIDE_BUILD_TESTS=ON \
          -DFIXEDWIDE_BUILD_EXAMPLES=ON >/dev/null
    cmake --build /tmp/build -j"$(nproc)" >/dev/null
    ctest --test-dir /tmp/build --output-on-failure
'
