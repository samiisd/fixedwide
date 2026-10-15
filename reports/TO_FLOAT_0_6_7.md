# Correctly rounded fixed-to-floating conversion: 0.6.7

Measured and validated on 2026-10-15. This iteration accelerates the reverse
conversion; 0.6.5 and 0.6.6 accelerated floating-to-fixed conversion only.

## Scope and observable behavior

`to_float<Float>`, `to_double` and the member `to_double()` now use an inline
integer path for recognized binary32, binary64, x86 little-endian x87 extended,
and ordinary little-/big-endian IEEE binary128 outputs, from fixed types of
8, 16, 32, 64 or 128 bits at **every valid decimal scale**, through D=38.
The existing input-format traits also guard the output encoders.

The result is the correctly rounded value of the exact rational `raw / 10^D`,
with nearest, ties-to-even rounding. It is independent of the active FP rounding
mode, FTZ/DAZ and fast-math, and does not raise FP exceptions. There is no
allocation, exception, public API or object-layout change. The original compiled
kernel remains directly callable. `from_float` and `src/floating.cpp` are unchanged.

**This is not a bit-for-bit preservation of the old reverse converter.** The
old floating-limb accumulation and division could round incorrectly, despite
its unconditional nearest-result documentation. For example:

```cpp
using F = fixedwide::Fixed128<8>;
auto value = F::from_raw(9'007'199'254'740'993LL);
// Old kernel:     0x1.5798ee2308c3ap+26
// Correct / new:  0x1.5798ee2308c3bp+26
```

Tests also contain a float double-rounding regression at raw
`20'480'003'662'109'374`, D=13, and full-precision x87/binary128 cases. Callers
that relied on old last-bit errors, FP exception flags, or ambient directed
rounding will observe a change on the new path. Numerical correctness is judged
against an independent rational oracle, not against the old implementation.

`Fixed256` and unknown output layouts retain the legacy FP-arithmetic kernel,
including its potential additional rounding error and FP-environment dependency.
The API documentation now states this limitation explicitly rather than claiming
an unconditional guarantee. No approximate fast mode or new rounding parameter
is introduced.

## Algorithm and bounds

Let `n` be the unsigned raw magnitude, `b = bit_width(n)`, `C = 5^D`,
`B = bit_width(C)` and `p` the output significand precision. Zero is handled first.
Set `K = p + 1 + B`, and normalize `n` to a K-bit integer `N`. Left shifts are
exact. Right shifts record whether any discarded bit was nonzero.

Compute `q = floor(N / C)` and whether its remainder is nonzero. Because
`2^(K-1) <= N < 2^K` and `2^(B-1) <= C < 2^B`, q has p+1 or p+2 bits.
Consequently it supplies at least one guard bit. The discarded normalization
bits and division remainder together form an exact sticky bit. The unbiased
exponent is `b - B - D - 1 + extra`, where `extra` identifies the p+2-bit case.
Round once using guard/sticky bits and output parity, then encode the result.
Binary32 subnormal spacing is selected **before** this final rounding.

The normalization does not lose a possible quotient carry: any omitted fraction
is strictly less than one, and the integer division remainder is at most C-1.
Its only remaining effect is whether the final fraction is exactly a tie.

Division has three bounded implementations:

- K<=64 uses ordinary constant integer division; D=0 needs no division.
- On the existing native x86-64 assembly backend, eligible binary32/binary64
  cases use `divq`. The p+2-bit quotient bound is below 64 bits, proving the
  instruction's high-limb < divisor precondition. The constant divisor is nonzero.
- Other cases use the compile-time integer reciprocal `M=floor(2^K/C)`.
  `floor(N*M/2^K)` underestimates `floor(N/C)` by at most one, since the omitted
  reciprocal fraction times N is less than one. One exact remainder check
  corrects the estimate. This is exact constant division, not an FP reciprocal.

For D<=38, C has at most 89 bits; K is at most 203. M fits 128 bits. Binary128
can require a 318-bit intermediate product, so the implementation splits it at
128 bits and propagates the carry between two individually safe 256-bit products.
The remainder before correction is less than 2*C < 2^90. Therefore a low-128-bit
modular subtraction recovers it exactly even when the operands are wider.
Binary32/binary64 use smaller multiply chains where the bounds permit.

All raw sign handling is unsigned, including the signed minimum. Supported
sources have magnitude at most 2^127 and nonzero value at least 10^-38, so no
supported output overflows; only binary32 can become subnormal. No output
significand bits are discarded except by the single specified rounding step.

## Measured performance

Host: virtualized AMD EPYC 9V74, x86-64 Linux, GCC 14.2.0, C++23,
CMake Release (`-O3 -DNDEBUG`), no LTO. The benchmark target disables loop/SLP
vectorization and FP contraction. The process was pinned to CPU 0; no frequency
controls or exclusive-host guarantees were applied. Raw samples retain host noise.

Each row converts 600,000 raw integer inputs, with seven repetitions. `pnl600k`
samples the fixed-point grid over [-5000,5000]; `full128.600k` samples full-width
128-bit raw patterns. Seed: `0x746f666c6f6174`. Generation, allocation, independent
rational validation and counting old-kernel disagreements are outside timing.
Each result's value bits are consumed; x87 ABI padding is ignored.

**Baseline:** the `.legacy` rows call the unchanged compiled 0.6.6 kernel in the
same executable, with the same inputs and result consumption. They are not a
separately rebuilt old executable. Numbers are median elapsed nanoseconds per
conversion in a batch, not dependency-chain latency or a promise for other CPUs.
In particular, the baseline here is not the user's approximately 15 ns measurement.

| Source | Output | Input distribution | Legacy ns | New ns | Speedup |
|---|---|---|---:|---:|---:|
| Fixed128<8> | double | PnL | 36.719 | 6.888 | 5.33x |
| Fixed128<10> | double | PnL | 41.220 | 7.421 | 5.55x |
| Fixed128<12> | double | PnL | 37.935 | 7.395 | 5.13x |
| Fixed128<8> | double | full 128 bits | 39.661 | 8.199 | 4.84x |
| Fixed128<10> | double | full 128 bits | 43.087 | 8.534 | 5.05x |
| Fixed128<12> | double | full 128 bits | 40.936 | 8.681 | 4.72x |
| Fixed128<38> | double | full 128 bits | 38.588 | 23.105 | 1.67x |
| Fixed128<8> | float | PnL | 37.695 | 5.497 | 6.86x |
| Fixed128<10> | float | PnL | 37.533 | 5.529 | 6.79x |
| Fixed128<12> | float | PnL | 36.775 | 5.381 | 6.83x |
| Fixed128<8> | x87 long double | PnL | 595.635 | 36.833 | 16.17x |
| Fixed128<10> | x87 long double | PnL | 594.733 | 43.351 | 13.72x |
| Fixed128<12> | x87 long double | PnL | 592.618 | 37.971 | 15.61x |

The complete [raw CSV](to-float-0.6.7-native.csv) also includes Fixed64 and the
other full-width output cases. Fixed64-to-double PnL medians were about 11.2 ns
in this build; do not infer performance from storage width alone. D=38 is faster
than the legacy path but does not meet the single-digit-nanosecond objective.
Binary128 correctness was exercised with GCC's alternate ABI; its performance
on native binary128 hardware was not measured. Portable performance is not claimed.
Historical instruction-count baselines and thresholds are unchanged.

## Local validation

- Native and forced-portable Release: **78/78 CTest cases passed in each build**,
  with Boost oracle tests and examples enabled.
- The new suite covers all **76 valid source width/scale combinations**, all
  three native output types, both signs, every raw bit-width boundary, signed
  minima, exact ties/parities/carry-out and neighbours, random full-width and
  log-uniform magnitudes, float subnormals, and exhaustive 8-bit raw values.
- Extended native suite: **3,961,620 exact-rational comparisons, zero mismatches**.
  Forced-portable: **3,961,620**, zero mismatches. Extended alternate binary128:
  **3,961,242**, zero mismatches. Native suites additionally found 353,652 old-kernel
  disagreements each; that is a property of this constructed dataset, not an
  estimated production error rate.
- Native and forced-portable ASan+UBSan: **7/7 focused tests passed each**, including
  both alternate long-double layouts and the allocation test. A parallel compiler
  build exceeded this container's memory limit; the native build was retried
  serially and the resulting instrumented tests all passed.
- Header-only fast-math/no-exceptions consumer: **277 checks** on the native x87
  ABI, including all four FP rounding modes, FTZ/DAZ on/off, known result bits and
  no FP exceptions. It deliberately does not link the compiled library. Alternate
  binary64 and binary128 header-only tests also pass.
- The header-only consumer also passes with `__SIZEOF_INT128__` undefined and
  `FIXEDWIDE_FORCE_PORTABLE` enabled. Allocation test reports zero allocations.
- **18 million benchmark input/type pairs** were independently validated outside
  timing; no incorrect new result was accepted to obtain a timing.
- GCC strict warning checks and `git diff --check` pass.

The Boost oracle binary-searches the exact rational exponent, divides arbitrary
precision integers by 10^D at the final output spacing, and uses a separate test
encoder. It does not reuse production normalization, reciprocal or encoder logic.
Alternate-ABI tests avoid calls to host long-double libc or the compiled kernel.
These are local results; the PR's cross-platform CI is a separate validation.

## Reproduce

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DFIXEDWIDE_BUILD_TESTS=ON -DFIXEDWIDE_BUILD_ORACLE_TESTS=ON \
  -DFIXEDWIDE_BUILD_BENCHMARKS=ON
cmake --build build -j2
ctest --test-dir build --output-on-failure
FIXEDWIDE_TO_FLOAT_SAMPLES=4096 build/tests/test_to_floating
FIXEDWIDE_TO_FLOAT_SAMPLES=4096 build/tests/test_to_floating_ld128
# Omit taskset on systems that do not provide it.
taskset -c 0 build/benchmarks/fixedwide_bench --filter .to_ --repetitions 7 > to-float.csv
```

For the portable build add `-DFIXEDWIDE_FORCE_PORTABLE=ON`. For a sanitizer build
add `-DFIXEDWIDE_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug` and run
`ctest --test-dir build -R 'to_floating|no_alloc' --output-on-failure`.
The alternate long-double targets are added automatically on native GCC x86-64.
