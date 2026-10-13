# Exact double conversion: 0.6.5

Measured 2026-10-13. These are local measurements, not projected Ryzen/Xeon
numbers or a claim about every x86-64 system.

## Results

Host: AMD EPYC 9V74 (virtualized x86-64), GCC 14.2.0, C++23, CMake Release
(`-O3 -DNDEBUG`), no LTO. The existing benchmark target disables loop/SLP
vectorization and FP contraction. No CPU affinity or frequency controls were
applied; the raw samples are retained so host variability is visible.

Each repetition converts all 600,000 doubles sampled uniformly from [-5000,
5000], with `std::mt19937_64` seed `0x6f7261636c65`, using `nearest_even`.
Generation, allocation and generic-kernel validation occur before timing. Each
result is consumed by the existing checksum/compiler barrier. Five repetitions;
CSV reports elapsed nanoseconds per conversion (batch throughput, not a
single-call dependency-chain latency).

| Fractional digits | 0.6.4 median ns | 0.6.5 median ns | Speedup |
|---|---:|---:|---:|
| `d8` | 432.070 | 6.811 | 63.4x |
| `d10` | 482.073 | 6.117 | 78.8x |
| `d12` | 547.211 | 6.643 | 82.4x |

Raw data: [before](from-double-0.6.5-before.csv) and
[after](from-double-0.6.5-after.csv), in the existing timing CSV schema. The
`.generic` rows measure the retained kernel on the same input. Its unchanged
cost is a useful control; the historical instruction-count baseline is untouched.

The before executable was built from upstream commit
`fd80d53eccde182da9e9ce499cd6d8d5cabb2a64` (v0.6.4), with only the new benchmark
fixture applied. The after executable used the same fixture and the new inline
conversion. The library's compiled `src/floating.cpp` is unchanged.

## Scope and correctness

The dispatch is restricted to IEEE binary64 `double`, fixed-point widths up to
128 bits and D <= 32. It uses the native unsigned 128-bit integer when available,
otherwise the existing portable `wide::uint128`. Wider targets/scales and
`float`/`long double` keep the generic kernel; `to_float` is unchanged.

Factoring `10^D = 5^D * 2^D` keeps `(2^53 - 1) * 5^D` within 128 bits through
D=32, instead of D=22 for the unfactored product. The multiplier itself fits
64 bits through D=27. No floating multiply, division, SIMD or approximate rounding
is involved. A shift of exactly 128 is distinguished from larger shifts because
the D=32 numerator can exceed half a raw unit.

Local validation:

- Native Release: 57/57 CTest cases passed.
- Forced-portable Release: 57/57 CTest cases passed.
- Extended floating differential run: 7,493,130 comparisons, zero mismatches,
  covering 19 width/scale combinations and all six rounding modes.
- ASan + UBSan floating suite: 955,914 differential comparisons passed.
- Header-only fast-math / FMA-enabled, exceptions-disabled consumer: 588 checks
  passed, including FTZ/DAZ, signaling NaNs and signed minima. No library link.
- The same header-only test passed with `__SIZEOF_INT128__` undefined, exercising
  the portable multiply implementation. Allocation test: zero allocations.
- The benchmark validates 1.8 million input/type pairs against the generic kernel
  outside its timed loops.

These results do not substitute for the repository's Windows, macOS, other
compiler and cross-architecture CI jobs.

## Reproduction

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DFIXEDWIDE_BUILD_TESTS=ON -DFIXEDWIDE_BUILD_BENCHMARKS=ON
cmake --build build -j4
ctest --test-dir build --output-on-failure
FIXEDWIDE_FLOAT_SAMPLES=16384 build/tests/test_floating
build/benchmarks/fixedwide_bench --filter uniform600k --repetitions 5 > after.csv
```

For the paired pre-change executable, apply only the `benchmarks/benchmark.cpp`
diff to v0.6.4 and build/run with the same compiler and options. Alternatively,
the `.generic` rows allow a same-build comparison without checking out the old
release. For the portable build set `FIXEDWIDE_FORCE_PORTABLE=ON`; for the
instrumented floating test set `FIXEDWIDE_SANITIZE=ON`.
