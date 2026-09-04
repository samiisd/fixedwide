# fixedwide 0.5.0 — status

**Version**: 0.5.0
**Standard**: C++23
**Disposition**: pre-1.0. The API may still change; `CHANGELOG.md` will say when.

This file records what has been executed. Nothing here is claimed unless a
command in `scripts/` or a job in `.github/workflows/` produced it, and the
output is retained under `reports/` or in a CI run.

## The release gate

The gate is **no regression against this library's own committed baseline**, and
it passes. It is not the same thing as parity with 0.4, and this file does not
claim it is: across 100 paired workloads the generalized implementation has a
slightly faster median than 0.4, while 9 to 18 rows exceed a 5% regression
depending on the Clang version, worst +24.5%. Those rows are itemised below. `.github/workflows/ci.yml` re-measures retired instructions per
operation under Valgrind on every pull request and fails if any workload grows
by more than 1%. Instruction counts are deterministic — two runs of
`scripts/icount.sh` on the same binary are byte-identical — which is what makes
a 1% threshold usable on a shared runner.

The older gate was 0.4 parity, which never passed and was never going to be the
right release criterion: 0.4 is a fixed-scale library with a smaller API, and
some of the gap is what generality costs. That comparison is kept as historical
evidence in `reports/BENCHMARK_VS_0_4.md`, and the rows that are still slower
are listed under known open items below rather than blocking a release.

## What 0.5.0 changed

0.5.0 makes the repository publishable: CI that runs, a Conan package, examples
that are tested, and a regression gate. Getting CI to run for the first time
found eight defects, every one of them in a configuration nothing in
the tree had ever built.

| Finding | What was done |
|---|---|
| `from_integer` could not be called with a runtime integer under GCC | `detail::max_integer_allowed` was `consteval` and took the sign as an argument, so C++23 immediate escalation promoted `from_integer` itself into an immediate function. Both answers are still computed at compile time. |
| `std::formatter<basic_fixed>` was rejected by libc++ | It hard-coded `std::format_context&`; libc++ instantiates a formatter with its own context type. `format.hpp`, `iostream.hpp` and `hash.hpp` had no test at all, and now have one that asserts `std::formattable`. |
| MSVC could not compile `src/arithmetic.cpp` | Two unguarded `__builtin_*` calls, now routed through `detail/overflow.hpp`, which gained an unsigned `mul_overflow`. |
| GCC 13 could not compile `src/floating.cpp` | `std::frexpl` and `std::ldexpl` are not in namespace `std` before GCC 14. The C++ `std::frexp` / `std::ldexp` overloads take `long double` and work everywhere. |
| The significand width was an x86 assumption | Derived from `sizeof(Float)`, which claims 64 bits for any 16-byte long double. Now `std::numeric_limits<Float>::digits`, capped at 64. |
| Cross-scale `<=>` claimed `constexpr` and was not | It always fell through to a kernel compiled into the library, so any comparison involving a `Fixed128` or `Fixed256` failed inside a `static_assert`. Added a constant-evaluation path, differential-tested against the runtime kernel over every scale pair and both signs. |
| `<bit>` and `<algorithm>` were used without being included | Worked only because libstdc++ includes them transitively; libc++ does not. |
| Two CI rows could never have passed | Clang 17 and 18 report `__cpp_concepts` as 201907 and libstdc++ gates `<expected>` on 202002. Measured, then paired with libc++. `docs/ci.md` records the whole compatibility table. |
| Windows jobs died before compiling anything | `tests/CMakeLists.txt` passed `${Boost_INCLUDE_DIRS}` unconditionally; with no Boost that is `-NOTFOUND`, which CMake rejects at generate time. |
| clang-cl compiled and then failed to link | It defines `__SIZEOF_INT128__` but targets the MSVC CRT, which has no `__divti3` / `__udivti3` / `__umodti3` and does not link compiler-rt by default. The build now tests whether 128-bit division links and falls back to the portable backend if not, rather than naming compilers. |
| No performance regression could be detected | `benchmarks/icount.cpp`, `scripts/icount.sh`, `scripts/compare_icount.py` and a committed baseline, gating every pull request. |
| One example, not run by anything | Eight examples, each a ctest test that checks its own output. |
| No Conan package | `conanfile.py` and `test_package/`, both built by CI, including the portable backend. |
| The coverage job was false-green | `llvm-cov` failed on every run and the pipe through `tee` discarded its exit status. Fixed, with enforced thresholds. |
| The release archive contained stale evidence | `SHA256SUMS` and the extraction log were tracked, so `git archive HEAD` embedded the previous release's. Generated in a staging directory now, and shipped as separate assets. |
| The CNL benchmark was not measuring a decimal multiply | `scaled_integer::operator*` returns a wider scale rather than rescaling, so the timed expression was a bare 64-bit multiply. The reported 8x gap is 2.8x at a matched scale. |
| Overflow and inexact were interchangeable in the audits | The precedence is now stated in `error.hpp` and asserted exactly, over 5.1 million checks and both backends. |
| Negative compile tests accepted any error | Each now asserts the diagnostic it expects, so a typo or a renamed header cannot pass as success. |
| The general mixed kernel used 1024-bit limbs for every operation | It now sizes itself to its operands across four tiers. `mul_to` 8561 -> 416 instructions, `div_to` 8377 -> 729, with 1,017,500 differential checks unchanged. |
| Coverage measured one backend and understated | Native and portable profiles are merged; `src/division.hpp` went from 10.32% to 98.67%. |
| `all.hpp` cost 558 ms to include | It pulled `<format>` and `<iostream>` unconditionally, which together are 885 ms of standard-library headers. The three std adapters are opt-in now and `all.hpp` is 187 ms. |
| `std::format("{:.2}", value)` printed the first two characters | The formatter parses its own spec; precision means decimals. |
| 85 warnings in library code | Zero, across four configurations, with `-Wsign-conversion` and `-Wold-style-cast` added and `-Werror` enforced in CI. |

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
| (checked, not assumed) `arithmetic.hpp` compile time | Measured 34 ms against 0.4's 48 ms per include, +41.2%, where alpha.4 measured +55.9%. The performance work costs nothing here -- compiled against the identical translation unit, alpha.4's headers and these take the same time -- and dropping `<concepts>` and `<limits>`, which this header did not need, took 4 ms off. |

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

Two places record this, and they are kept consistent:

* `.github/workflows/ci.yml` — every configuration the README claims, built and
  tested on every push. `docs/ci.md` says what each job proves.
* `scripts/verify_all.sh` — the same coverage on a workstation, writing
  `reports/EXECUTION_MATRIX.csv`. Every row is labelled `executed-pass`,
  `executed-fail`, `configured-not-executed`, `not-configured` or
  `not-applicable`. A platform that is not `executed-pass` is not described as
  supported anywhere.

Executed and passing in CI:

* Linux x86-64, GCC 14, Debug and Release, with the Boost differential oracle
* Linux x86-64, Clang 18 with libc++, Debug and Release
* Linux x86-64, Clang 20 with libstdc++ (fuzzer and coverage)
* Linux AArch64, GCC 14, Release
* macOS on Apple silicon, AppleClang
* Windows x64, MSVC and clang-cl
* Forced portable backend, and portable with `__SIZEOF_INT128__` undefined
* ASan + UBSan over both the native and the portable backend
* **Big-endian (s390x, emulated)** -- 32/32, the first execution of the
  big-endian half of `binary.hpp`
* Shared library
* CMake install plus an external `find_package` consumer
* `conan create` plus `test_package`, native and portable
* libFuzzer under ASan + UBSan, 90 s per pull request and 30 minutes nightly
* The instruction-count regression gate

Executed on this host, and not reproducible in CI:

* Linux x86-64, Clang 22 and GCC 16, Release
* No-exceptions / no-RTTI library build
* **Linux AArch64 on real hardware** (Pixel 6, static cross build) — 19/19
* The paired wall-clock comparison against 0.4, which needs a 0.4 source tree
  that is not in this repository and cannot be

Not configured, and so not claimed: Windows ARM64. Big-endian is executed under
emulation rather than on hardware, which is stated as such.

## Compiler floor

GCC 14, Clang 18 with libc++ (or Clang 19+ with either), AppleClang 15, MSVC
19.3x. Two of those are not arbitrary:

* **GCC 13 cannot build this library.** libstdc++ did not put `frexpl` and
  `ldexpl` in namespace `std` until GCC 14. The library no longer names those
  functions, but GCC 13 has not been tested and is not claimed.
* **Clang 17 and 18 cannot build it against libstdc++**, and neither can
  anything else that uses `std::expected`: they report `__cpp_concepts` as
  `201907` and libstdc++ gates `<expected>` on `202002`. Use libc++, or Clang 19
  or newer. `conanfile.py` refuses the broken combination with that message
  rather than letting it fail as a wall of template errors.

## Performance

The gate is the instruction-count baseline above, and it passes.

For wall-clock, the paired comparison against the untouched 0.4 release is
retained as historical evidence: byte-identical benchmark source, core-pinned
and interleaved, medians of 27 samples. Full per-row output in
`reports/BENCHMARK_VS_0_4.md` and `reports/raw/`.

| Clang 17, versus 0.4 | alpha.3 | alpha.4 | alpha.5 / 0.5.0 |
|---|---:|---:|---:|
| rows faster than 0.4 | 42 | 48 | 62 |
| rows >5% slower | 32 | 21 | 9 |
| rows >10% slower | 22 | 12 | 3 |
| rows >25% slower | 15 | 9 | 0 |
| worst row | +161.0% | +63.8% | +13.5% |
| median row | +0.9% | +0.4% | -1.50% |

| compiler | rows faster | >5% | >10% | >25% | worst row | median row |
|---|---:|---:|---:|---:|---:|---:|
| Clang 17 | 62 | 9 | 3 | 0 | +13.5% | -1.50% |
| Clang 18 | 60 | 12 | 5 | 0 | +19.6% | -1.08% |
| Clang 22 | 50 | 18 | 7 | 0 | +24.5% | -0.07% |

## Documentation

Every public declaration carries a `///` doc comment: what it does, what each
parameter means, and which errors it can return. `clangd` shows them on hover,
verified over the language-server protocol rather than assumed, and a top-level
build exports `compile_commands.json` so an editor finds the flags without being
configured by hand. The comments cost no build time — compiled against an
identical translation unit, the tree with and without them takes the same 47 ms.

`docs/api_reference.md` covers every header and function, the four naming rules
and the 0.4 compatibility surface. `docs/benchmarks.md` says how each number was
produced. `docs/ci.md` records the compiler and standard-library combinations
that work, measured. Eight examples in `examples/` are ctest tests.

## Known open items

1. Cross-scale `mul_to` and `div_to` still cost more off the native path than
   on it — 53 instructions against 416 for a multiply — because the general
   path evaluates an exact rational and performs a division the native path
   avoids. That is inherent to the operation. What was *not* inherent has been
   removed: the kernel used 1024-bit limbs for everything, so the same multiply
   cost 8561. All three tiers are in the regression baseline.
2. Fourteen rows still exceed the old 3% 0.4-parity threshold on Clang 17,
   worst +13.5%. They are the 2.4 ns 64-bit `div` and `mul_div` rows; the cost
   is the caller's stack-protector prologue, paid because this library inlines
   its narrow fast path where 0.4 keeps it behind a call. Removing the inline
   path was measured and is worse.
3. Decimal parsing is about 2x slower than `std::from_chars` on a `double`. It
   is 19-24% faster than 0.4 and faster than Boost.Decimal, which does the
   comparable job; `std::from_chars` produces a binary float and rejects nothing
   on a decimal grid.
4. Formatting is faster than 0.4 and than `std::to_chars` on a `double`, but
   slower than Boost.Decimal (14.0 ns against 12.4 ns).
5. `arithmetic.hpp` costs 41.2% more to include than 0.4's, down from 55.9% in
   alpha.4. Most of what remains is `detail/constexpr_arith.hpp`: 9 ms of the
   15 ms gap, measured by including it alone. It cannot be dropped without
   dropping `constexpr` arithmetic.
6. `Fixed256::quantize` is about 27 ns and is the slowest `Fixed256` operation.
   Its divisor always fits one limb, and both its division and its multiply-back
   still run the general four-limb routines.
7. `Fixed256` multiply and divide are about 28 ns. Both build a 512-bit
   intermediate and divide it by a scale that occupies one limb.
8. Big-endian is now executed, under s390x emulation: 32/32 including the
   examples, on every push. It had been implemented, documented and shipped
   without ever running, because every host, CI runner and phone this library
   had touched was little-endian. The job proves the target really is
   big-endian before it reports anything. Emulated, so correctness only -- no
   timing is taken from it.
9. On a platform whose `long double` is IEEE binary128, `from_float` keeps 64
   significand bits rather than 113. The cap is explicit in `src/floating.cpp`
   and is what the `std::uint64_t` accumulator can hold.
10. Coverage now merges the native and portable backends, which was the right
    fix rather than a threshold change: `src/division.hpp` read 10.32% covered
    from a native-only run and reads 98.67% merged, because the portable jobs
    were exercising it all along. Merged totals are 75.1% of lines and 77.9% of
    branches locally. The gate is a ratchet set below the first merged CI
    measurement; raise it, never lower it.
11. `arithmetic.hpp` costs about 41% more to include than 0.4's -- 42 ms
    against 34 ms on clang 22. That gap was the headline compile-time item in
    previous releases and it was the wrong thing to look at by a factor of
    thirty: `all.hpp` cost 558 ms, of which 8 ms was this. The real cost was
    `<format>` (435 ms) and `<iostream>` (450 ms), pulled in unconditionally by
    the umbrella header. `all.hpp` is now 187 ms. What remains of the
    `arithmetic.hpp` difference is `detail/constexpr_arith.hpp`, which cannot be
    dropped without dropping `constexpr` arithmetic.
12. The competitor comparison against CNL is at scale 6 only, because CNL
    overflows at scale 12 for ordinary values. A comparison against a library
    that does check overflow at 12 decimals would be more informative than
    either.
