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
| (cost) `arithmetic.hpp` compile time | Up from +38% to +62% over 0.4. The scale-specialised kernels now return the caller's own type so the callee writes the caller's return slot; that is more instantiation for less runtime. |

## What alpha.4 changed

alpha.4 is a narrow performance, portability and reproducibility pass on top of
alpha.3. The type system is unchanged.

| Audit finding | What was done |
|---|---|
| 75/100 rows slower than 0.4 on Clang 17 | Root-caused and partly fixed. See `reports/BENCHMARK_VS_0_4.md`. |
| Performance reported from one compiler | The gate now runs on Clang 17, 18 and 22, with a measured noise floor. |
| `arithmetic.hpp` +68.9% compile time | Now +38.2% on this host, measured with instantiation included. |
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
| rows faster than 0.4 | 42 | 48 | 62 |
| rows >5% slower | 32 | 21 | 7 |
| rows >10% slower | 22 | 12 | 3 |
| rows >25% slower | 15 | 9 | 0 |
| worst row | +161.0% | +63.8% | +13.1% |
| median row | +0.9% | +0.4% | -1.22% |

Across all three compilers:

| compiler | rows faster | >3% | >5% | >10% | >25% | worst row | median row |
|---|---:|---:|---:|---:|---:|---:|---:|
| Clang 17 | 62 | 14 | 7 | 3 | 0 | +13.1% | -1.22% |
| Clang 18 | 60 | 14 | 12 | 5 | 0 | +19.7% | -1.20% |
| Clang 22 | 49 | 26 | 18 | 11 | 0 | +24.1% | +0.06% |

**This still does not meet the 3% release gate**: 14 rows exceed it on Clang 17.
It is no longer the wide `Fixed128` paths -- those are at parity or faster. What
remains is 2.4 ns 64-bit rows where the gap is four instructions per operation,
none of them arithmetic. `reports/BENCHMARK_VS_0_4.md` counts them and says where
they come from.

The previous release called the wide gap a scheduling difference it could not
explain. That was measurable after all: sampling retired instructions rather than
cycles put 79% of them on two `movups` in a wrapper that does no arithmetic.

## Known open items

1. Fourteen rows still exceed the 3% gate on Clang 17, worst +13.1%. They are
   the 2.4 ns 64-bit `div` and `mul_div` rows; the cost is the caller's
   stack-protector prologue, paid because this library inlines its narrow fast
   path where 0.4 keeps it behind a call. Removing the inline path was measured
   and is worse.
2. Decimal parsing is about 2x slower than `std::from_chars` on a `double`. It is
   now 17-19% faster than 0.4 and faster than Boost.Decimal, which does the
   comparable job; `std::from_chars` produces a binary float and rejects nothing
   on a decimal grid.
3. Formatting is faster than 0.4 and than `std::to_chars` on a `double`, but
   slower than Boost.Decimal (14.9 ns against 12.4 ns).
4. `arithmetic.hpp` costs 62% more to include than 0.4's, up from 38% in
   alpha.4. The scale-specialised kernels are now instantiated per decimal count
   against the caller's own return type, which is what removes the return-path
   copy at runtime. Traded deliberately.
5. `Fixed256::quantize` is unchanged at about 27 ns and is the slowest
   `Fixed256` operation. Its divisor always fits one limb, and both its division
   and its multiply-back still run the general four-limb routines.
6. `mul_to.Money.from.Price.Rate` measures 7.69 ns, against 3.50 ns in alpha.4.
   The generated code for that operation is unchanged: compiled into an isolated
   translation unit, both trees produce 7.47 ns for the same loop. The move is
   code placement inside the benchmark's own translation unit, not the library.
   The underlying cost is real either way -- the mixed multiply's rescale is a
   `__udivti3` call, and the divisor always fits 64 bits.
7. Windows and macOS are configured in CI but have never been executed.
8. Big-endian byte order is implemented in `binary.hpp` but never executed.
