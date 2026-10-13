# Exact floating conversion: 0.6.6

Measured 2026-10-13. These are local measurements, not predictions for every
machine. The v0.6.5 reference already has the optimized double path.

## Performance

Host: virtualized AMD EPYC 9V74, x86-64 Linux, GCC 14.2.0, C++23, CMake Release
(`-O3 -DNDEBUG`), no LTO. The existing benchmark target disables loop/SLP
vectorization and FP contraction. No CPU affinity or frequency controls were
applied; shared-host variation is visible in the raw samples.

Each repetition converts 600,000 inputs uniformly distributed over [-5000,5000],
with seven repetitions and `nearest_even`. Each source type has its own
`std::uniform_real_distribution<Float>` and `std::mt19937_64` seeded with
`0x6f7261636c65`; long-double inputs are NOT first rounded through double.
Generation, allocation and reference validation are outside the timed loop.
Every result is consumed through the existing checksum/compiler barrier.

| Source | Target fractional digits | 0.6.5 median ns | 0.6.6 median ns | Speedup |
|---|---:|---:|---:|---:|
| `float` | 8 | 439.178 | 4.634 | 94.78x |
| `float` | 10 | 485.134 | 7.691 | 63.08x |
| `float` | 12 | 502.051 | 5.851 | 85.81x |
| `double` | 8 | 11.318 | 8.473 | 1.34x |
| `double` | 10 | 11.338 | 9.091 | 1.25x |
| `double` | 12 | 11.703 | 9.708 | 1.21x |
| `long_double` | 8 | 527.127 | 8.662 | 60.85x |
| `long_double` | 10 | 518.423 | 7.748 | 66.91x |
| `long_double` | 12 | 555.587 | 7.865 | 70.64x |

The destination is `Fixed64<D>`. These are **batch-throughput costs per
conversion**, not single-call dependency-chain latency. The long-double timings
are for native x87 extended precision (64 significand bits in a 16-byte object).
Binary128 correctness is exercised separately, but its performance on native
binary128 hardware was not measured. No universal speedup is claimed.

Raw [before](from-float-0.6.6-before.csv) and
[after](from-float-0.6.6-after.csv) data retain the existing CSV schema and
same-build `.generic` controls. The same new fixture was used in both builds;
the before library is unchanged v0.6.5, commit
`a9d4740ac2111ec16ab768a5c8f66debc59b2a2e`. Absolute timings should not be compared
with an earlier report collected under a different fixture/compiler context or
host load. All 5.4 million source/target input pairs are validated before timing.
Historical instruction-count baselines and thresholds are untouched.

## Implementation and precision

A format-specific decoder produces the full integer significand, binary
exponent, sign and nonfinite flag. The shared engine computes
`(significand * 5^D) * 2^(exponent + D)` with exact integer multiplication,
checked shifts and the existing six rounding modes. No floating arithmetic,
heap allocation or exception is needed in an eligible conversion.

The intermediate width is selected at compile time to hold both the destination
and a conservative bound on the complete significand product:

| Input format | Significand bits | Intermediate choice |
|---|---:|---|
| binary32 | 24 | 64 bits for <=64-bit targets and D<=17; otherwise 128 |
| binary64 | 53 | 64 bits for <=64-bit targets and D<=4; 128 through D=32; otherwise 256 |
| x87 extended | 64 | 64 bits for <=64-bit targets and D=0; 128 through D=27; otherwise 256 |
| binary128 | 113 | 128 through D=6; otherwise 256 |

All valid scales of supported fixed-point destinations up to 128 bits are
covered, including D=38. Binary128 at D=8/10/12 retains all 113 significand bits
using a 256-bit intermediate. Native unsigned 128-bit arithmetic is used when
available; forced-portable builds use the existing portable wide integers.

Format detection checks numeric traits and known layouts rather than identifying
long double from its size alone. x87 decoding is restricted to the known
little-endian x86 ABI; it reads only the ten value bytes, ignores ABI padding,
handles pseudo-denormals and rejects invalid unnormal encodings. IEEE binary128
supports the ordinary little- and big-endian word layouts. Unknown layouts and
`Fixed256` destinations retain `detail::from_float_kernel`; `src/floating.cpp`
and every `to_float` path remain unchanged.

Error precedence, signed minima, directed rounding, ties, nonfinite handling and
signed-zero results are preserved. Right shifts equal to the intermediate width
are checked separately from larger shifts, so nonzero and half-unit remainders
are not silently discarded. No narrowing through double is used.

## Local validation

- Native and forced-portable Release: **72/72 CTest cases passed in each build**,
  with Boost oracle tests and examples enabled.
- Extended format suite: **6,247,494 exact-rational comparisons, zero mismatches**,
  over float, double and native x87 long double, 21 width/scale combinations,
  all six rounding modes, plus wider-destination fallback cases. The native ABI
  also compares each case directly with the unchanged generic kernel.
- Extended existing double suite: **8,281,866 kernel comparisons**, zero mismatches.
- Native and forced-portable ASan+UBSan: seven focused floating CTest cases each.
- Header-only fast-math tests: all source types, full-significand precision,
  signed minima, subnormals, signaling/quiet NaNs, all four FP rounding modes,
  FTZ/DAZ, and no raised FP exceptions. Native x87 run: **4,722 checks**.
- Header-only test also passes with `__SIZEOF_INT128__` undefined and the
  portable backend selected. Allocation test: **zero allocations**.
- GCC x86 tests execute alternate binary64 and binary128 long-double formats,
  including randomized full-113-bit inputs against an independent Boost
  integer-rational oracle. They intentionally do not link the compiled library
  or call host long-double libc functions, which have an incompatible ABI under
  those compiler switches. Both alternate formats are tested in portable and
  sanitizer configurations as well.
- GCC strict warning syntax checks and `git diff --check` pass.

The independent oracle takes integer fields from a separate test encoder, not
from the production decoder. It uses full arbitrary-precision decimal scaling
and quotient/remainder division, not the optimized product/shift algorithm.
Tests include constructed ties and their neighbours, signed-limit neighbours,
normal/subnormal boundaries, all intermediate-width shift boundaries, random
full significands, x87 padding, pseudo-denormals and explicit invalid operands.
These local results do not substitute for the repository's full cross-platform
CI, including native ARM64 and big-endian binary128 execution.

## Reproduce

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DFIXEDWIDE_BUILD_TESTS=ON -DFIXEDWIDE_BUILD_ORACLE_TESTS=ON \
  -DFIXEDWIDE_BUILD_BENCHMARKS=ON
cmake --build build -j2
ctest --test-dir build --output-on-failure
FIXEDWIDE_FLOAT_SAMPLES=8192 build/tests/test_floating_formats
FIXEDWIDE_FLOAT_SAMPLES=16384 build/tests/test_floating
build/benchmarks/fixedwide_bench --filter uniform600k --repetitions 7 > after.csv
```

For the before build, apply only this PR's `benchmarks/benchmark.cpp` diff to
v0.6.5 and run the same benchmark command. For portable validation add
`-DFIXEDWIDE_FORCE_PORTABLE=ON`. For sanitizer checks configure another build
with `-DFIXEDWIDE_SANITIZE=ON`, build its floating targets and run
`ctest --test-dir build-sanitize -R fixedwide.floating --output-on-failure`.
The alternate long-double tests are added automatically on native GCC x86-64
builds; the independent oracle requires Boost headers only in tests.
