# fixedwide 0.5.0-alpha.5 — status

**Version**: 0.5.0-alpha.5
**Standard**: C++23
**Disposition**: pre-1.0. The performance gate does not pass; see below.

This file records what has been executed. Nothing here is claimed unless a
command in `scripts/` produced it and the output is retained under `reports/`.

## What alpha.5 changed

alpha.5 is a performance pass. No public type, signature or semantic changed;
the internal `detail::` entry points changed shape, so headers and library must
be rebuilt together. All 25 tests pass in all ten executed configurations, and
the benchmark's own oracle validates 363,520 cases per run.

| Finding | What was done |
|---|---|
| The wide `Fixed128` gap the previous release could not explain | Root-caused: a 16-byte struct passed in memory is copied through a temporary with a vector move over two scalar stores, which does not forward in the store buffer. Fixed by not crossing the ABI boundary with a struct. |
| `inexact_chain.FP128.mul` +64% | The 128-bit multiply's fast path rounded with a branch on a coin flip. Made branchless, like the paths beside it. |
| `native_by128.FP128.div` +41% | `divide_native_n` was `noinline`; 0.4 marks the same function `always_inline`. Restored, with 0.4's toward-zero shortcut. |
| `format_2digits.FP128` +40% on Clang 17 and 18 | The 128-bit formatting kernel rounded through `wide::uint128` even when quotient, remainder and divisor all fit 64 bits. Now rounds narrow values in 64 bits: that row is 26% *faster* than 0.4 on Clang 17. |
| `mul`/`mul_div`'s 64-bit range test | Two paired equality tests and a branch each, replaced by one addition. |
| The mixed-scale rescale was a `__udivti3` call | Every divisor a mixed operation reaches is a power of ten that fits 64 bits; the value is 128 bits. One or two hardware divisions replace the libgcc call. `mul_to.Money.from.Price.Rate` 7.69 ns -> 2.48 ns, against 3.50 ns in alpha.4. |
| (checked, not assumed) `arithmetic.hpp` compile time | Measured 33 ms against 0.4's 48 ms per include, +45.5%, where alpha.4 measured +55.9%. The performance work costs nothing here -- compiled against the identical translation unit, alpha.4's headers and these take the same time -- and dropping `<concepts>` and `<limits>`, which this header did not need, took 4 ms off. |

## What alpha.4 changed

alpha.4 is a narrow performance, portability and reproducibility pass on top of
alpha.3. The type system is unchanged.

| Audit finding | What was done |
|---|---|
| 75/100 rows slower than 0.4 on Clang 17 | Root-caused and partly fixed. See `reports/BENCHMARK_VS_0_4.md`. |
| Performance reported from one compiler | The gate now runs on Clang 17, 18 and 22, with a measured noise floor. |
| `arithmetic.hpp` +68.9% compile time | Measured +55.9% on this host with instantiation included. (This row said +38.2% until alpha.5 checked it against `reports/COMPILE_TIME.md`, which alpha.4 itself had recorded as +55.9%.) |
| Competitor suite not reproducible | Rebuilt: pinned FetchContent, medians, validated outputs, semantic classes. |
| MSVC claim unsupported | The header-level blockers are removed. Still not executed; marked as such. |
| AArch64 claim unsupported | Executed on real hardware: 17/17 on a Pixel 6. |
| `_BitInt` in public headers | The conversion overloads are gone from `wide.hpp`. Two Clang-only inline fast paths still use `_BitInt(256)` as a local computation type; that is stated rather than denied. |
| Stale release metadata | Version, status, changelog and evidence regenerated from this commit. |
| (not in the audit) Mixed-scale had no performance evidence | Benchmarked, then made 70x-760x faster. |
| (not in the audit) `Fixed256` had no performance evidence | Benchmarked, then made 1.3x faster on multiply, divide and `mul_div`. |
| (not in the audit) arithmetic was not `constexpr` | `mul`, `div`, `mul_div`, `quantize` and `remainder` now are. |

## Executed configurations

`scripts/verify_all.sh` produces `reports/EXECUTION_MATRIX.csv`. Every row is
labelled `executed-pass`, `executed-fail`, `configured-not-executed`,
`not-configured` or `not-applicable`. A platform that is not `executed-pass`
must not be described as supported.

Executed and passing on this host:

* Linux x86-64, Clang 22 and GCC 16, Release — 23/23
* Linux x86-64, Clang 17 in a pinned `ubuntu:24.04` container — 23/23
* Forced portable backend — 23/23
* Portable backend with `__SIZEOF_INT128__` undefined — 23/23
* ASan + UBSan over both the native and the portable backend — 23/23, no diagnostics
* Shared library — 23/23
* No-exceptions / no-RTTI library build
* Install plus an external `find_package` consumer
* **Linux AArch64 on real hardware** (Pixel 6, static cross build) — 19/19

Configured but not executed here: Windows MSVC, Windows clang-cl, macOS arm64,
macOS x86-64, Linux AArch64 CI. Not configured: Windows ARM64, big-endian.

## Performance

Paired against the untouched 0.4 release, byte-identical benchmark source,
core-pinned and interleaved, medians of 27 samples. Full per-row output in
`reports/BENCHMARK_VS_0_4.md` and `reports/raw/`.

| Clang 17, versus 0.4 | alpha.3 | alpha.4 | alpha.5 |
|---|---:|---:|---:|
| rows faster than 0.4 | 42 | 48 | 63 |
| rows >5% slower | 32 | 21 | 7 |
| rows >10% slower | 22 | 12 | 3 |
| rows >25% slower | 15 | 9 | 0 |
| worst row | +161.0% | +63.8% | +12.6% |
| median row | +0.9% | +0.4% | -1.88% |

Across all three compilers:

| compiler | rows faster | >3% | >5% | >10% | >25% | worst row | median row |
|---|---:|---:|---:|---:|---:|---:|---:|
| Clang 17 | 63 | 14 | 7 | 3 | 0 | +12.6% | -1.88% |
| Clang 18 | 60 | 14 | 12 | 4 | 0 | +20.3% | -1.15% |
| Clang 22 | 50 | 26 | 17 | 11 | 0 | +24.2% | +0.02% |

**This still does not meet the 3% release gate**: 14 rows exceed it on Clang 17,
worst +12.6%. It is no longer the wide `Fixed128` paths -- those are at parity or
faster. What remains is 2.4 ns 64-bit rows where the gap is four instructions per
operation, none of them arithmetic. `reports/BENCHMARK_VS_0_4.md` counts them and
says where they come from.

The previous release called the wide gap a scheduling difference it could not
explain. That was measurable after all: sampling retired instructions rather than
cycles put 79% of them on two `movups` in a wrapper that does no arithmetic.

## The 0.4 compatibility surface

`FP64`, `FP128`, `fp64_min/max`, `fp128_min/max`, `fractional_digits`, `scale`,
`mul_wide`, `narrow`, `parse64`, `parse128`, `from_double64` and `from_double128`
fix the scale at 12 digits, which is what 0.4's API was. They exist so the paired
benchmark can compile 0.4's byte-identical source against this library. Each is
marked in its header with the generic replacement, and README.md lists them in
one table. Nothing else in the library uses them, and no other public name is
tied to a particular width or scale.

## Known open items

1. Fourteen rows still exceed the 3% gate on Clang 17, worst +12.6%. They are
   the 2.4 ns 64-bit `div` and `mul_div` rows; the cost is the caller's
   stack-protector prologue, paid because this library inlines its narrow fast
   path where 0.4 keeps it behind a call. Removing the inline path was measured
   and is worse.
2. Decimal parsing is about 2x slower than `std::from_chars` on a `double`. It is
   now 19-24% faster than 0.4 and faster than Boost.Decimal, which does the
   comparable job; `std::from_chars` produces a binary float and rejects nothing
   on a decimal grid.
3. Formatting is faster than 0.4 and than `std::to_chars` on a `double`, but
   slower than Boost.Decimal (14.0 ns against 12.4 ns).
4. `arithmetic.hpp` costs 45.5% more to include than 0.4's, down from 55.9% in
   alpha.4. Most of what remains is `detail/constexpr_arith.hpp`, the
   compile-time evaluation path: 9 ms of the 15 ms gap, measured by including it
   alone. It cannot be dropped without dropping `constexpr` arithmetic.
5. `Fixed256::quantize` is unchanged at about 27 ns and is the slowest
   `Fixed256` operation. Its divisor always fits one limb, and both its division
   and its multiply-back still run the general four-limb routines.
6. `Fixed256` multiply and divide are about 28 ns. Both build a 512-bit
   intermediate and divide it by a scale that occupies one limb.
7. Windows and macOS are configured in CI but have never been executed.
8. Big-endian byte order is implemented in `binary.hpp` but never executed.
