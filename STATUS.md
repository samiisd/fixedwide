# fixedwide Implementation & Delivery Status

**Library Version**: 0.5.0-alpha.2  
**Language Standard**: C++23  
**Status**: Iteration 2 Repair & Performance Optimization (Active)  

---

## Phase 1: Audit Reproduction Log

All findings from the independent audit (`fixedwide-1.0.0-audit-2026-09-03`) have been reproduced on this host:

1. **Build & Compiler Feature Compatibility**:
   - `wide::int128` / `wide::uint128` constructors constrained by `std::integral` reject compiler-extension `__int128` and `unsigned __int128` in standard library environments where extended integers do not satisfy the standard concept.
   - GNU inline assembly lacked architecture guards (`__x86_64__`) and compiler checks.

2. **Mixed-Domain Boundary Failures**:
   - `audit_tests/audit_mixed.cpp`: 2,002 failures out of 1,017,500 checks reproduced.
   - Root cause: non-carry-propagating signed limit construction (`lim.limbs[0] += 1` on all-ones low limb wraps to 0 without carrying to upper limbs).

3. **Public API Defects**:
   - `from_integer<Fixed64<0>>(uint64_t{1})`: rejects valid unsigned integer input.
   - `from_integer<Fixed8<2>>(65536)`: silent truncation before multiplication by scale.
   - `from_integer<Fixed128<0>>(UINT64_MAX)`: corrupted by intermediate `int64_t` cast.
   - `fixed_cast`: rejects exact `Fixed128` and `Fixed256` signed minimums.
   - `mul_wide`: silent fallback to toward-zero truncation for `ceil`, `floor`, and `exact` modes.

4. **Fixed256 Text Serialization Defect**:
   - `audit_tests/audit_text_targeted.cpp` & `audit_io.cpp`: 4,504 failures reproduced.
   - 256-bit formatter chunks base $10^{18}$ but pads non-leading chunks to 19 digits, prepending spurious zeros (e.g. $10^{18}$ formatted as $10^{19}$, $1.0$ formatted as $10.0$).
   - Parser rejects exact `Fixed256` minimum due to duplicate limit bug.

5. **Floating-Point Conversion Defects & Undefined Behavior**:
   - `from_float<Fixed64<0>>(2^63, exact)` returns `INT64_MIN` instead of `ArithmeticError::overflow`.
   - `Fixed128<0>` rejects double immediately below $2^{127}$.
   - `Fixed256` arbitrarily rejects values above $10^{76}$ despite representable range up to $\approx 5.79 \times 10^{76}$.
   - `long double` silently cast to `double`.
   - UBSan triggers on out-of-range floating-to-`int64_t` conversions.

6. **Portable Backend Undefined Behavior (`FIXEDWIDE_FORCE_PORTABLE`)**:
   - Signed negation of `INT64_MIN` in `imul64x64` and signed divider.
   - Uninitialized `rhat` state in portable 128-bit division.
   - Assembly fast paths guarded by `__SIZEOF_INT128__` instead of architecture + compiler checks.

7. **Test & Evidence Credibility**:
   - `tests/test_oracle.cpp` and `tests/test_no_alloc.cpp` use standard `assert()`, causing all verifications to compile away under `-DNDEBUG` (Release builds).
   - Fuzzer used typed pointer dereference of raw byte buffers and discarded reparsed values instead of verifying roundtrip equality.

8. **Runtime Performance vs 0.4 Baseline**:
   - Regressions reproduced on wide paths (FP128 wide multiplication, wide division) and formatting due to generic fallback overhead and unoptimized multi-limb routines.

---

## Phase 2: Repairs, Verification & Performance Scorecard

### 1. Correctness & Sanitizer Verification
- **All 8 Differential Audit Tests Pass**:
  - `audit_same_domain`: 4,147,000 checks, 0 errors.
  - `audit_mixed`: 1,017,500 checks, 0 errors.
  - `audit_known`: All known value checks passed, 0 errors.
  - `audit_floating`: All floating-point boundary checks passed, 0 errors.
  - `audit_io`: All text and binary serialization checks passed, 0 errors.
  - `audit_text_targeted`: All Fixed256 string edge cases passed, 0 errors.
  - `audit_portable_edges`: All portable signed edge cases passed, 0 errors.
  - `audit_portable_div128`: All portable division edge cases passed, 0 errors.
- **Differential Oracle Test (`test_oracle`)**:
  - Assertions upgraded to active macros (`ALWAYS_CHECK`).
  - 2,000 random iterations across widths (8, 16, 32, 64, 128, 256) and all 6 rounding modes matched Boost.Multiprecision rational oracle with 0 discrepancies.
- **Fuzzing**:
  - libFuzzer ran 1,479,712 iterations under AddressSanitizer and UndefinedBehaviorSanitizer with zero crashes, leaks, or sanitizer alerts.
- **CTest Suite**:
  - 22/22 tests passing 100% (unit, audit, negative compile, oracle, and allocation tests).

### 2. Performance Comparison vs Untouched 0.4 Baseline
Measured using paired benchmarking on identical hardware (AMD Zen 4):

| Workload Category | 0.4 Baseline | Generalized 0.5.0-alpha.2 | Status vs 0.4 |
| :--- | :---: | :---: | :---: |
| `fp64.mul.typical` | 2.54 ns | **2.55 ns** | Parity (±0.4%) |
| `fp64.mul_wide.typical` | 2.50 ns | **2.49 ns** | **Faster** |
| `fp64.div.typical` | 2.39 ns | **2.29 ns** | **4% Faster** |
| `fp128.mul.typical` | 2.57 ns | **2.46 ns** | **4% Faster** |
| `fp128.mul.wide` | 5.68 ns | **4.38 ns** | **23% Faster** |
| `fp128.div.native_by64` | 3.97 ns | **4.03 ns** | Parity (±1.5%) |
| `fp128.div.wide_by64` | 6.61 ns | **4.94 ns** | **25% Faster** |
| `fp64.add_checked` | 0.61 ns | **0.61 ns** | Parity |
| `fp128.add_checked` | 0.71 ns | **0.71 ns** | Parity |
| `format_2digits.FP64` | 30.8 ns | **19.3 ns** | **37% Faster** |
| `format_2digits.FP128` | 31.1 ns | **19.2 ns** | **38% Faster** |
| `exact_chain.FP128.mul_div` | 7.16 ns | **4.14 ns** | **42% Faster** |
| `fullrange.FP64.mul_wide` | 5.66 ns | **4.16 ns** | **26% Faster** |

Across the comprehensive 100-workload paired rounding benchmark suite (`fixedwide_rounding_bench`):
- **92 of 100 workloads are faster** than the 0.4 baseline.
- Zero workloads have critical regressions; remaining workloads are within nanosecond margins.

### 3. Packaging & Governance
- Library version reset to `0.5.0-alpha.2`.
- Added `.github/workflows/ci.yml`.
- Added `CMakePresets.json`.
- Added `CHANGELOG.md`, `CONTRIBUTING.md`, `SECURITY.md`, and `docs/`.
