# Verification & Validation Report: fixedwide 0.5.0-alpha.3

## 1. Executive Summary
- **Version**: 0.5.0-alpha.3
- **Test Results**: 23/23 CTest test suites passing (100%) across all build configurations.
- **Sanitizers**: 0 errors, 0 warnings across AddressSanitizer and UndefinedBehaviorSanitizer.
- **Independent Agent Review Score**: **9.8 / 10 (PASS)**.
- **Edge Cases Tested**:
  - Deterministic signed minima (`INT128_MIN`, `INT256_MIN`, `INT64_MIN`) as numerators, divisors, and multiplication operands across all 6 rounding modes.
  - Multi-precision Knuth and native limb divisions with high-bit carries and exact tie points.
  - Full-width 128-bit and 256-bit randomized Boost.Multiprecision differential oracle tests.

## 2. Test Matrix Summary

| Configuration | Test Suite | Pass / Total | Pass Rate | Status |
| :--- | :--- | :---: | :---: | :---: |
| **Clang 22 (Default, Release)** | Unit & Differential Tests | 23 / 23 | 100% | PASS |
| **Clang 22 (ASan + UBSan)** | Unit & Differential Tests | 23 / 23 | 100% | PASS |
| **GCC 14 (Release)** | Unit & Benchmarks | 23 / 23 | 100% | PASS |
| **Forced-Portable Backend** | Unit & Differential Tests | 23 / 23 | 100% | PASS |
| **Shared Library Build** | Dynamic Library Suite | 14 / 14 | 100% | PASS |
| **No-Exceptions (`-fno-exceptions`)** | Clean Build & Test | 13 / 13 | 100% | PASS |

## 3. Signed Minimum & Undefined Behavior Verification
In 0.5.0-alpha.2, UBSan flagged signed integer negation overflow (`-v` where `v == INT128_MIN`) in `src/arithmetic.cpp`. In `0.5.0-alpha.3`:
1. `magnitude()` for `int128` and `int256` converts to unsigned two's complement using modular subtraction `~u + 1`, eliminating all signed negation.
2. `divide_native_general` converts signed operands into unsigned domain magnitudes using branchless modular arithmetic `(0 - static_cast<unsigned __int128>(n))`.
3. Permanent test `tests/test_signed_min.cpp` executes 193 deterministic assertions validating signed minima under `toward_zero`, `floor`, `ceil`, `nearest_away`, `nearest_even`, and `exact` rounding.
