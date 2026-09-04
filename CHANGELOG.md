# Changelog

All notable changes to the `fixedwide` library are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
