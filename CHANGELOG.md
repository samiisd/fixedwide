# Changelog

All notable changes to the `fixedwide` library are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.6.0] - 2026-09-04

Everything here was found by measuring rather than by guessing, and two of the
items are the measurement pointing somewhere other than where the previous
release was looking.

Two changes are not patch-level, which is why this is 0.6.0 and not 0.5.1:
`all.hpp` no longer includes three headers, and `std::format` with a precision
now produces a different string. Both are described under **Changed**.

### Fixed

- **`std::format("{:.2}", value)` printed the first two characters, not two
  decimals.** The formatter inherited `std::formatter<std::string_view>`, so
  precision meant "truncate the string": `123.4567` formatted as `12`. For a
  decimal type that is a silently wrong number, which is the one failure mode
  this library exists to prevent, and `{:.2}` is what anyone formatting money
  types first. The formatter now parses its own spec --
  `[[fill]align][width][.precision][f]` -- where precision means decimals and
  rounds nearest-even, matching `to_chars`. A precision wider than the type
  carries is a `std::format_error` rather than a padded lie, and numbers
  right-align by default as every arithmetic type does.

### Performance

- **`#include <fixedwide/all.hpp>` costs 187 ms instead of 558 ms.** It pulled
  in `format.hpp`, `iostream.hpp` and `hash.hpp` unconditionally, and those drag
  `<format>` (435 ms), `<iostream>` (450 ms) and `<functional>` (103 ms) behind
  them -- each more expensive than the whole of the rest of this library. They
  are adapters to standard facilities, not part of the numeric API, so they are
  now opt-in and `all.hpp` says so where a reader will find it.

  This also corrects where previous releases were looking. The headline
  compile-time item was `arithmetic.hpp` being 41% more expensive than 0.4's,
  which is 8 ms of a 558 ms umbrella header: the wrong target by a factor of
  thirty.

  **Migration**: a file that included `all.hpp` and then used `std::format`,
  `operator<<` or `std::hash` on a `basic_fixed` now needs the matching header
  next to it. It is a compile error with an obvious fix, never a silent change
  in behaviour. Nothing in this repository needed one.

- **The general cross-scale kernel sizes itself to its operands.** Every mixed
  operation that missed the narrow `mixed_native` path did all of its arithmetic
  in sixteen 64-bit limbs, regardless of the values: a `Fixed64` x `Fixed64`
  multiply into a `Fixed128` needs about eighty bits and was running a 16x16
  schoolbook multiply, 256 partial products of which four were not multiplying
  by zero. `src/mixed.cpp` now computes an upper bound on the bits the numerator
  and denominator need and dispatches to the smallest of four tiers — 128, 256,
  512 or 1024 bits.

  | | before | after |
  |---|---:|---:|
  | `mul_to`, 64-bit operands | 8561 | **416** |
  | `div_to`, 64-bit operands | 8377 | **729** |
  | `mul_to`, 256-bit operands | 8561 | **1764** |

  In wall-clock, on the same machine, timing the identical operation against the
  old kernel and the new one: `mul_to` 332.67 ns -> 19.69 ns and `div_to`
  590.95 ns -> 35.36 ns, with the native path unchanged at 1.45 ns as a control.

  Instructions per operation, measured deterministically. Same results: 1,017,500
  differential checks against Boost.Multiprecision, unchanged, over the native
  and portable backends, clean under ASan and UBSan. Every other workload in the
  baseline is byte-identical, which is the evidence the change is confined to
  this kernel. The remaining gap to the native path is the exact-rational
  division the general path has to do and the native one does not.

### Added
- **A big-endian job**, under s390x emulation. `binary.hpp` has always
  implemented both byte orders and only little-endian had ever been executed --
  every host, CI runner and phone this library had run on was little-endian, so
  `to_bytes<endian::big>` was shipped, documented and untested. It passes 32/32
  including the examples. `scripts/test_big_endian.sh` proves the target really
  is big-endian before reporting anything, so the job cannot pass by silently
  running on the wrong architecture.

### Changed

- **`#include <fixedwide/all.hpp>` no longer pulls in `format.hpp`,
  `iostream.hpp` or `hash.hpp`.** Source-breaking: a file that used
  `std::format`, `operator<<` or `std::hash` on a `basic_fixed` through the
  umbrella header now needs the matching header beside it. It is a compile error
  with an obvious fix, never a silent change in behaviour.
- **`std::format("{:.2}", value)` produces a different string.** It used to
  print the first two characters; it now prints two decimals. Any caller relying
  on the old output was relying on a bug.

## [0.5.0] - 2026-09-04

The release that makes the repository publishable: continuous integration that
actually runs, a Conan package, examples that are tested, and a regression gate.

`.github/workflows/ci.yml` had never executed — there was no remote. Running it
for the first time found eight defects, every one of them in a configuration
nothing in this tree had ever built. Two of its own rows could not have compiled.

### Fixed

- **`from_integer` could not be called with a runtime integer under GCC.**
  `detail::max_integer_allowed` was `consteval` and took the sign as a
  parameter. Because every caller passes a runtime `bool`, C++23's
  immediate-escalation rules (P2564) promoted `from_integer` itself into an
  immediate function, so passing it an ordinary `int` did not compile. The two
  possible limits are still computed during compilation; only the select
  between them is not.
- **`std::formatter<basic_fixed>` was rejected by libc++.** Its `format` member
  hard-coded `std::format_context&`, but an implementation may instantiate a
  formatter with its own context type, and libc++ does — it uses a
  `back_insert_iterator` where `std::format_context` has a `char*`. Every
  `basic_fixed` was therefore non-formattable there, while compiling fine under
  libstdc++. `format`, `iostream` and `hash` had no test at all; the new
  `tests/test_format.cpp` asserts `std::formattable` rather than only calling
  `std::format`.
- **MSVC could not compile `src/arithmetic.cpp`.** Two `__builtin_add_overflow`
  / `__builtin_mul_overflow` calls were outside any guard. Both now go through
  `detail/overflow.hpp`, which gained an unsigned `mul_overflow` whose portable
  branch the forced-portable CI job exercises.
- **GCC 13 could not compile `src/floating.cpp`.** libstdc++ did not put
  `frexpl` and `ldexpl` in namespace `std` until GCC 14. The C++ `<cmath>`
  overloads of `std::frexp` and `std::ldexp` take `long double` and work
  everywhere, so the `l` suffixes were never needed.
- **The significand width was an x86 assumption.** It came from `sizeof(Float)`,
  which claims 64 bits for any 16-byte `long double` — wrong for IEEE binary128
  — and silently changes meaning where `long double` is `double`. It is now
  `std::numeric_limits<Float>::digits`, capped at the 64 bits the accumulator
  holds.
- **Cross-scale `operator<=>` and `operator==` claimed `constexpr` and were
  not.** Outside the narrow `mixed_native` fast path they fell through to a
  kernel compiled into the library, so any comparison involving a `Fixed128` or
  `Fixed256` failed inside a `static_assert`. Added a constant-evaluation path
  and differential-tested it against the runtime kernel across every scale pair
  and both signs.
- `std::countl_zero` and `std::min` were used without including `<bit>` and
  `<algorithm>`; that only worked because libstdc++ includes them transitively.
- `tests/CMakeLists.txt` passed `${Boost_INCLUDE_DIRS}` to the audit targets
  unconditionally. Without Boost that expands to `Boost_INCLUDE_DIR-NOTFOUND`,
  which CMake rejects at generate time — both Windows jobs died there before
  compiling a file.
- `tests/audit_floating.cpp` perturbed a `long double` by 2^-60, which needs 61
  significand bits. On Apple silicon `long double` *is* `double`, so the test
  asserted a precision the platform does not have. It now scales the
  perturbation to the platform and still checks the 18th decimal moves.
- `enable_testing()` was called inside the tests block, so examples could not
  register ctest tests without tests also being enabled.
- **The coverage job reported success while producing no coverage.** `llvm-cov`
  failed on every run — source directories were passed where object files
  belong — and the pipe through `tee` discarded its exit status. It now runs
  under `set -euo pipefail`, with one positional binary and the rest as
  `-object`, sources selected by `-ignore-filename-regex`, and enforced
  thresholds taken from a CI measurement rather than a workstation one.
- **The release archive contained the previous release's evidence.**
  `SHA256SUMS` and the extraction log were tracked files, so `git archive HEAD`
  embedded a checksum and a verification log describing a *different* archive.
  A manifest mismatch inside the archive was also swallowed by `|| true` and
  merely counted. `scripts/release.sh` now stages, generates the manifest from
  what is staged, and fails hard on any mismatch — verified by tampering with an
  extracted file. Those three generated files are no longer tracked, which also
  removes the staleness that two unrelated Dependabot merges had already caused.
- **The CNL benchmark was not timing a decimal multiply.**
  `cnl::scaled_integer::operator*` returns a type whose exponent is the *sum* of
  the operands', so the timed expression was a bare 64-bit multiply with the
  result left at the wrong scale. That is where "CNL is about 8x faster" came
  from; forcing the result back to the declared type, the gap at a matched scale
  is 2.8x. The comparison was also at mismatched scales (`Fixed64<12>` against a
  scale-6 CNL type) and validated against a 0.01 floating tolerance that cannot
  distinguish a correct decimal result from a wrong one.
- 85 compiler warnings in library code, mostly one Knuth division loop indexing
  an `int` into a `std::size_t`. Zero now across native, portable, no-`__int128`
  and GCC, with `-Wsign-conversion` and `-Wold-style-cast` added permanently.
  The loop rewrite also removed a sign-extension: `mul.Fixed256` is 5
  instructions cheaper.
- Both differential audits accepted `ArithmeticError::overflow` and `inexact`
  interchangeably, so the precedence between them was never tested. It is now
  stated in `<fixedwide/error.hpp>` — a result that does not fit is `overflow`,
  even under `Rounding::exact` — and asserted exactly over 5.1 million checks
  across both backends.
- Negative compile tests used `WILL_FAIL`, which accepts *any* compile error, so
  a typo or a renamed header would have passed while the rule under test had
  stopped being enforced. Each now asserts the diagnostic it expects.

### Added
- **Conan package.** `conanfile.py` with a `force_portable` option mirroring the
  CMake one, a version read from `version.hpp` so it cannot drift, and a
  `validate()` that refuses Clang < 19 with libstdc++ up front with the reason.
  `test_package/` links the package and checks the number it prints. CI builds
  both the native and the portable package.
- **An instruction-count regression gate.** `benchmarks/icount.cpp` and
  `scripts/icount.sh` count retired instructions per operation under Valgrind,
  using a two-point measurement that cancels everything not proportional to the
  iteration count. Two independent runs are byte-identical, which is what makes
  a 1% threshold usable on a shared CI runner where wall-clock noise is far
  larger. `scripts/compare_icount.py` gates every pull request against the
  committed baseline in `benchmarks/baseline/`.
- **Eight examples**, replacing the single untested one. Each is one file, one
  idea, and a ctest test that checks its own output before printing `OK`.
- `tests/test_format.cpp`, covering `format.hpp`, `iostream.hpp` and `hash.hpp`,
  none of which had any test.
- CI jobs for Conan, the shared library, coverage, a fuzz smoke run, and the
  regression gate; a nightly workflow with a 30-minute fuzz run and the
  competitor benchmark; a release workflow that builds the archive, extracts it,
  tests the extraction and publishes it; and Dependabot for action versions.
- `docs/ci.md`, recording which compiler and standard-library combinations
  actually work, measured rather than assumed.
- The coverage job merges profiles from the native and portable backends
  instead of measuring one. This was a measurement bug, not a testing gap:
  `src/division.hpp` read 10.32% line coverage from a native-only run and reads
  98.67% merged, because the portable jobs had been exercising it all along.
- `.clang-format`, `scripts/format.sh` and a `lint` CI job that also compiles
  with `-Werror` over three toolchains and both backends. clang-format is pinned
  to an exact version from PyPI because majors format the same file differently,
  and `StatementMacros` was needed to make the check converge at all — without
  it clang-format re-indented the explicit-instantiation macros differently on
  every run. `benchmarks/rounding_bench.cpp` is excluded and must stay so: the
  paired 0.4 comparison requires it to be byte-identical to 0.4's copy.
- A CI job that consumes the library through `FetchContent`, because the
  README's install instructions were never tested.
- A `decimal fixed, matched scale` benchmark class that pairs each scale with
  its own counterpart, seeds both libraries from identical raw integers,
  includes negative operands, and checks an exact integer oracle. It surfaced
  that **CNL cannot do 12 decimals at all**: it forms the product in `int64_t`,
  so `123.456789012345 * 2` gives `-0.000002` where fixedwide returns
  `246.913578024690`. The benchmark asserts this on every run.

### Changed
- **The release gate is now no regression against this library's own committed
  baseline**, which passes, rather than parity with the 0.4 release, which never
  did and was never the right criterion — 0.4 is a fixed-scale library with a
  smaller API. The 0.4 comparison is kept as historical evidence in
  `reports/BENCHMARK_VS_0_4.md` and its remaining slower rows are listed under
  known open items in `STATUS.md`.
- **The CI compiler matrix.** Clang 17 and 18 report `__cpp_concepts` as
  `201907` and libstdc++ gates `<expected>` on `202002`, so those rows could
  never have compiled. Clang is now paired with libc++; the fuzz and coverage
  jobs use Clang 20 with libstdc++, because the libFuzzer runtime Ubuntu ships
  is built against libstdc++ and will not link into a libc++ binary.
- **README** now leads with the problem the library solves and a side-by-side
  against `double` and hand-rolled `int64` cents, then install, then the
  comparison table. The 0.4 compatibility surface and the naming rules moved to
  `docs/api_reference.md`, which was expanded to cover every header and
  function.
- `docs/benchmarks.md` documents the two-tier measurement and the cross-scale
  performance cliff: `mul_to` and `div_to` are about 160x cheaper when the
  aligned intermediate fits 126 bits, which is now stated with the rule for
  where the edge is.
- MSVC builds add `/Zc:__cplusplus`, `/Zc:preprocessor` and `/utf-8`.

## [0.5.0-alpha.5] - 2026-09-04

A performance pass. No public type, signature or semantic changed. The internal
`detail::` entry points changed shape, so headers and library must be rebuilt
together.

Against the untouched 0.4 release on Clang 17, the audit's compiler: 63 of 100
rows faster (was 48), no row more than 25% slower (was 9), worst row +12.6% (was
+63.8%), median row -1.88% (was +0.4%). Clang 18 and Clang 22 also have zero rows
above 25%. Full per-row output in `reports/BENCHMARK_VS_0_4.md`.

### Fixed
- A 16-byte operand passed in memory cost about twenty cycles per call. Three
  128-bit operands plus a returned `std::expected` do not fit the argument
  registers, so `mul_div`'s divisor went on the stack -- and Clang materialises a
  struct argument in a temporary and copies it there with a 16-byte vector move
  over two 8-byte stores, which does not forward in the store buffer. 0.4's
  `FP128` holds a scalar `__int128` and is pushed from the registers it is
  already in. `detail::mul_div128_impl` now takes the divisor as its two limbs.
  `wide_product.FP128.mul_div` went from +56% to +3%.
- The same mismatch on the return path: a kernel writes two 8-byte halves into
  the caller's return buffer and the caller reloads them 16 bytes wide.
  `mul128_scaled` and `div128_scaled` now return the caller's own return type, so
  the callee writes the caller's return slot and there is no copy.
  `wide_product.FP128.mul` went from +30% to +5%.
- The 128-bit multiply's inline fast path rounded to nearest-even with a branch
  on a coin flip, where the 64-bit and `mul_div` paths beside it were already
  branchless. On a serially dependent chain that mispredict cost more than the
  division: `inexact_chain.FP128.mul` went from +64% to parity.
- `divide_native_n` was `noinline`, so a division whose operands were already in
  registers paid a call to reach two instructions; 0.4 marks the same function
  `always_inline`. Restored, together with 0.4's toward-zero shortcut.
  `native_by128.FP128.div` went from +41% to parity.
- The 128-bit formatting kernel rounded through `wide::uint128` even when the
  quotient, remainder and divisor all fit 64 bits, which is every value a
  `Fixed128` shares with a `Fixed64`. Every comparison, shift and subtraction ran
  as a member function on two limbs. Clang 22 undid that; Clang 17 and 18 did
  not, and it was the whole of `format_2digits.FP128` at +40%. That row is now
  26% faster than 0.4 on Clang 17.
- The 64-bit range test in `mul` and `mul_div` was two paired equality tests per
  operand, each with a branch. Replaced by one addition.
- Every mixed-scale rescale divided a 128-bit value by a power of ten with a
  `__udivti3` call out to libgcc, although every divisor a mixed operation
  reaches fits 64 bits. `detail::mixed_native::divide_magnitude` does it in one
  or two hardware divisions, keeping the generic form for a wider divisor and for
  constant evaluation. `mul_to.Money.from.Price.Rate` 7.69 ns -> 2.48 ns, against
  3.50 ns in alpha.4; measured in isolation the same loop went 7.47 -> 2.09 ns.
  `tests/test_mixed_native.cpp` sweeps every magnitude width from 0 to 128
  against every divisor width from 1 to 128 against the compiler's own division,
  because `divq` faults rather than wraps when its quotient does not fit a limb.
- `arithmetic.hpp` included `<concepts>` and `<limits>` for `std::same_as`,
  `std::integral` and two raw bounds. All three are one line each over
  `<type_traits>` and a shift. `mixed.hpp` included `<concepts>` and used
  nothing from it. Worth 4 ms of parse time per translation unit.

### Changed
- The 256-bit entry points take their operands by reference. A 32-byte struct is
  passed in memory by value, so each call copied three of them onto the stack.
  Measured effect under 2%; kept because it is strictly less work.
- The names that fix the scale at 12 digits -- `FP64`, `FP128`, `fp64_min/max`,
  `fp128_min/max`, `fractional_digits`, `scale`, `mul_wide`, `narrow`, `parse64`,
  `parse128`, `from_double64`, `from_double128` -- are now labelled in their
  headers as the 0.4 compatibility surface, each with its generic replacement,
  and listed in one table in README.md. They exist because the paired benchmark
  compiles 0.4's byte-identical source against this library. No other public name
  is tied to a particular width or scale.
- `arithmetic.hpp` costs 45.5% more to include than 0.4's, where alpha.4 measured
  55.9%. STATUS.md had claimed 38.2% for alpha.4; `reports/COMPILE_TIME.md` from
  that release says 55.9%, and the stale figure is corrected. Compiled against an
  identical translation unit, alpha.4's headers and these take the same time: the
  performance work above costs no build time, and dropping two standard headers
  bought some back.

### Documentation
- Every public declaration now carries a `///` doc comment: what it does, what
  each parameter means, and which errors it can return. Enums document each
  enumerator, so `Rounding::nearest_even` and `ParseError::too_precise` explain
  themselves at the call site instead of in a separate document. Verified over
  the language-server protocol rather than assumed: `clangd --check` parses the
  headers with 0 errors and a `textDocument/hover` request returns the text.
- The numbered section markers (`// 1. ADD`, `// 5. MUL`) were the first line of
  every hover popup, above the description. Removed; the brief leads now.
- `\copydoc` and `\copydetails` are written out. Doxygen expands them, clangd
  does not, so a reader in VS Code or Neovim saw the literal command and nothing
  else. Thirteen of them.
- The top-level build sets `CMAKE_EXPORT_COMPILE_COMMANDS`, so `build/` carries
  the `compile_commands.json` that clangd needs to know the include paths and
  the standard. Only at top level: depending on this library does not change a
  consumer's build.
- The comments cost no build time. Compiled against an identical translation
  unit, the tree with and without them takes the same 47 ms.

### Naming
An audit of every public name, and the fixes.

- **`bit_width` was an ambiguous overload pair.** `fixedwide::wide::bit_width`
  returned `int` and `fixedwide::bit_width` returned `unsigned` from an identical
  body. For any caller that wrote `using namespace fixedwide;` and called it
  unqualified, ordinary lookup found one and argument-dependent lookup added the
  other: a hard compile error, and neither copy was used anywhere in the library.
  The duplicate is gone and the header says why it must not come back.
- **`ParseError::inexact` could never be returned.** Nothing produced it: text
  off the type's decimal grid is `too_precise`, and there is no other way for a
  parse to be inexact. Removed, and `error.hpp` now states the difference between
  `invalid_precision` (the caller asked for more decimals than the type has),
  `too_precise` (the data carries more than it can hold) and `inexact`.
- **One word for the count of fractional digits: `decimals`.** It was five --
  `Decimals`, `fractional_digits`, `digits`, `decimals`, `dec` -- with
  `current_dec` and `current_decimals` in adjacent declarations of the same
  parameter. Two spellings survive because callers write them, and both are
  marked in their headers as the same quantity: `basic_fixed::fractional_digits`
  and `FormatOptions::digits`, the latter written as a designated initialiser in
  source that must stay byte-identical to 0.4's. `scale` now means 10^decimals
  and nothing else.
- **One suffix for a compiled worker: `_kernel`.** `_impl` is gone from the
  library; the D-templated specialisations keep `_scaled`, which says what makes
  them different.
- `pow10_bits` returned a bit count, not a power of ten: `bits_for_pow10`.
- `limit_for_bits` was the runtime-width sibling of `limit_magnitude_u256`;
  it is now an overload of that name, and the mixed path's 128-bit version is
  `limit_magnitude_u128`.
- `unsigned_for_impl` / `unsigned_for` follow the standard trait idiom:
  `unsigned_for` / `unsigned_for_t`.
- `to_chars` named its output `buffer` in one overload and `output` in four.
  `parse64` and `parse128` named their parameters `s` and `r` where the rest of
  the API spells words. `string.hpp` hard-coded `char buf[128]` beside a
  `text_capacity` that says 128. `from_float_impl` and `to_float_impl` are
  `float_to_raw` and `raw_to_float`, which is what they do. A local named `scale`
  shadowed the public `fixedwide::scale` in every consumer.
- `from_double<Target>` added, so `to_double` has the pair the other four
  conversions have. `from_integer` and `to_string` stay unpaired on purpose and
  their headers say why.
- README.md states the four rules the API follows, so the next name has
  somewhere to be checked against.

### Known open
- Fourteen rows still exceed the 3% gate on Clang 17, worst +13.1%. They are the
  2.4 ns 64-bit `div` and `mul_div` rows, and the gap is four instructions per
  operation, none of them arithmetic: this library inlines its narrow fast path
  into the caller, which makes the calling function large enough for Clang to
  give it a stack frame and `-fstack-protector-strong` to put a canary on it.
  Removing the inline path restores 0.4's call shape and measures worse.
- `Fixed256::quantize` is still about 27 ns, and `Fixed256` multiply and divide
  about 28 ns. Their divisor always fits one limb, and both the division and the
  multiply-back still run the general four-limb routines. The same fix that
  worked for the mixed rescale applies.
- `detail/constexpr_arith.hpp`, the compile-time evaluation path, is 9 ms of the
  15 ms that `arithmetic.hpp` costs over 0.4's. It cannot be dropped without
  dropping `constexpr` arithmetic.

## [0.5.0-alpha.4] - 2026-09-03

A narrow performance, portability and reproducibility pass on alpha.4's
predecessor. The type system and public API are unchanged.

### Fixed
- 128-bit kernels computed in the limb structs rather than the compiler's own
  `__int128`, so every shift, compare and add went through a member function on
  a 32-byte object passed in memory. They now compute in native width
  (`src/native.hpp`); `wide::int128` remains the public storage.
- `compute_pow10` ran a shift-add loop at runtime on every operation, because
  generalising the scale turned it into a function argument. Replaced with
  constexpr `pow10` tables.
- Every text conversion widened its value to `wide::int256` first, so formatting
  a `Fixed64<12>` pushed 32 bytes through memory. Split by storage width.
  Reduced-digit formatting is now 10-30% faster than 0.4.
- `quantize` issued three runtime divisions by the same divisor where one
  multiply with an overflow check suffices. Toward-zero quantize is now 36%
  faster than 0.4.
- `wide.hpp` declared explicit conversions to both `std::int64_t` and
  `long long`. Those are the same type on AArch64 and different on x86-64
  Linux, so the pair was a redefinition on one target and a missing conversion
  on the other. This was a real build break, found by compiling for AArch64.
- `std::is_signed_v<__int128>` and `std::make_unsigned_t<__int128>` are not
  available in strict `-std=c++23` on Clang 17. `detail/overflow.hpp` now probes
  the type's behaviour instead of the trait.

### Changed
- Public headers no longer use GNU overflow builtins directly; `detail::add_overflow`
  and `detail::sub_overflow` use them where available and a portable checked
  path elsewhere. This was the reason the library could not compile under MSVC.
- Removed the `_BitInt` conversion operators from the public `wide.hpp`. Two
  Clang-only inline fast paths still use `_BitInt(256)` as a local computation
  type, which has no ABI surface; the documentation says so rather than claiming
  the headers are free of it.
- `wide.hpp` no longer includes `<bit>` or `<concepts>` (about 15 ms of parse
  time per consumer) for one function and one predicate.
- The competitor benchmark fetches CNL 1.1.7 and fpm 1.1.0 at pinned tags,
  reports medians rather than the minimum of five trials, validates every timed
  loop's output outside the timed region, and groups rows by semantic class.

### Added
- `scripts/paired_bench.sh` and `scripts/compare_bench.py`: byte-identical
  benchmark sources, core-pinned, interleaved per seed, per-row output with no
  averaging across categories, and `SELF_CHECK=1` to measure the noise floor.
- `scripts/docker_bench.sh`: the paired gate on Clang 17 and 18 in a pinned
  `ubuntu:24.04` image.
- `scripts/run_aarch64.sh`: static cross build run on an adb-connected arm64
  device. Executed on a Pixel 6: 17/17.
- `scripts/verify_all.sh`: runs every claimed configuration and writes
  `reports/EXECUTION_MATRIX.csv`.
- `scripts/compile_time.sh`: header cost against 0.4, including instantiation.
- `tests/test_overflow.cpp`: differential test of the portable overflow path
  against the compiler builtins (111.7M comparisons). Test count 22 to 23.
- `examples/consumer`: a standalone `find_package` consumer used by CI.
- CI now builds and runs Linux x86-64 (Clang 17/18, GCC 14), Linux AArch64,
  macOS arm64 and x86-64, Windows MSVC and clang-cl, both sanitizer backends,
  the portable and no-`__int128` configurations, and the install consumer.

- Scale-specialised kernels: `mul128_scaled<D>`, `div128_scaled<D>`,
  `mul64_scaled<D>` and `div64_scaled<D>` are declared in the public header and
  explicitly instantiated per decimal count, so a compiled kernel sees the scale
  as a compile-time constant exactly as 0.4's did. `i128_max / scale` is a
  constant again rather than a `__udivti3` call per division.
- The rounding increment is branchless. Whether a rounding mode increments is a
  coin flip on real data, so branching on it mispredicted on nearly every
  operation: 17.3 million branch misses over 12.3 million wide multiplies, at an
  otherwise identical instruction count. The wide nearest-even multiply went
  from 11.05 ns to 6.07 ns, past 0.4's 6.25 ns.
- `parse_fixed_kernel` is templated on the destination width and explicitly
  instantiated, with a fast path for plain decimals whose kept digits fit 64
  bits. Parsing went from 6-11% slower than 0.4 to 17-19% faster, and is now
  faster than Boost.Decimal on the same contract.
- `pow10` tables are sized from the per-width decimal cap rather than
  `sizeof(T)`, which had left a silently wrapped `10^77` in the last slot of the
  signed 256-bit table. The table builder now rejects any entry that failed to
  exceed its predecessor, so a scale that wraps is a compile error.
- Mixed-width, mixed-scale arithmetic ran every operation through 1024-bit
  Knuth division, including comparison, which needs no division at all:
  `add_to` cost 418 ns against 0.54 ns for the same-type add. Narrow paths
  guarded by compile-time bounds make it 70x to 760x faster, with `add_to`,
  `fixed_cast` and comparison now at the same-type floor.
- `divmod64` ran one hardware division per limb unconditionally, so a `Fixed256`
  product occupying three of eight limbs paid for five divisions it did not
  need -- serially dependent ones. Together with bypassing the general Knuth
  divider for single-limb scales, `Fixed256` multiply, divide and `mul_div` are
  1.3x faster.
- Overflow now outranks inexact at every width. `mul(INT32_MAX, INT32_MAX)` on
  `Fixed32<4>` with `Rounding::exact` reported `inexact` where `Fixed64<12>`
  reported `overflow`, because the narrow widths range-check after the kernel
  rounds.

### Added
- `mul`, `div`, `mul_div`, `quantize` and `remainder` are `constexpr`. `add` and
  `sub` already were, so a caller building a table of constants hit the
  inconsistency immediately. `detail/constexpr_arith.hpp` is a second, simple
  implementation of the same contract selected by `if consteval`; the runtime
  paths are untouched.
- `tests/test_constexpr.cpp` (470,556 comparisons) and
  `tests/test_mixed_native.cpp` (157,250) hold the new implementations against
  the existing ones: same value, same error, every rounding mode. Both found
  real bugs on their first run.
- `benchmarks/mixed_bench.cpp` covers what the paired 0.4 comparison cannot:
  mixed-scale operations and `Fixed256`.

### Known limitations
- The performance gate against 0.4 does not pass. Wide `Fixed128` `mul_div` and
  some `div` rows remain up to +63% on Clang 17. Reported per row in
  `reports/BENCHMARK_VS_0_4.md`, not averaged away.
- No paired GCC performance row can exist: 0.4 requires C++ `_BitInt(256)` and
  will not configure under GCC.
- Windows and macOS are configured in CI but have not been executed.

## [0.5.0-alpha.3] - 2026-09-03

### Fixed
- Undefined behaviour negating signed minima.
- Mixed-domain signed limit construction that failed to propagate a carry.

### Added
- Compiler and architecture guards on the GNU inline assembly.

## [0.5.0-alpha.2] - 2026-09-03

### Added
- Generalized fixed-point template `basic_fixed<Bits, Decimals>` supporting widths 8, 16, 32, 64, 128, and 256 bits.
- Type aliases `Fixed8<D>`, `Fixed16<D>`, `Fixed32<D>`, `Fixed64<D>`, `Fixed128<D>`, `Fixed256<D>`.
- Cross-scale and mixed-precision operations: `mul_to<Dest>(a, b, rounding)`, `div_to<Dest>(a, b, rounding)`, `add_to<Dest>(a, b)`, `sub_to<Dest>(a, b)`.
- Explicit 3-way comparisons (`<=>`, `==`, `!=`, `<`, `<=`, `>`, `>=`) across heterogeneous scales and bit widths.
- High-performance portable wide arithmetic backend (`wide::uint128`, `wide::int128`, `wide::uint256`, `wide::int256`) with optional hardware acceleration (x86_64 inline assembly and `__int128`).
- Backward compatibility layer for 0.4 API: `FP64`, `FP128`, `i128`, `u128`, `mul_wide`, `divmod`, `divide_to_i128`, `mul_div`.
- Comprehensive text parsing and formatting (`to_chars`, `from_chars`, `to_string`, `parse64`, `parse128`, `std::formatter`, `std::ostream` / `std::istream`).
- Binary serialization (`to_bytes<endian>`, `from_bytes<T, endian>`).
- Floating-point conversions (`from_double`, `to_double`) with exact rounding.
- 8 independent differential audit test suites integrated into CTest.
- CMake presets (`CMakePresets.json`) for Debug, Release, Sanitizers, Coverage, and Benchmarks.
- Continuous Integration workflow (`.github/workflows/ci.yml`).

### Fixed
- Fixed FP128 wide arithmetic regression: restored single/dual `divq` fast-path for native and wide divisions, achieving performance parity and speedups over 0.4 across 92% of benchmark workloads.
- Fixed `test_oracle.cpp` to use always-active `ALWAYS_CHECK` assertion macros resistant to `-DNDEBUG`.
- Fixed implicit conversion ambiguity in `basic_fixed` widening constructor by marking it `explicit`.
- Resolved all Clang 17 / GCC 14 build warnings and portability constraints.

### Changed
- Reset library version from untrusted 1.0.0 claim to `0.5.0-alpha.2` pending stabilization and multi-platform validation.
