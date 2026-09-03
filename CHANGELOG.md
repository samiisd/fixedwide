# Changelog

All notable changes to the `fixedwide` library are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
