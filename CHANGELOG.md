# Changelog

All notable changes to the `fixedwide` library are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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

### Known limitations
- The performance gate against 0.4 does not pass. Wide `Fixed128` operations
  remain up to +67% on Clang 17. Reported per row in
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
