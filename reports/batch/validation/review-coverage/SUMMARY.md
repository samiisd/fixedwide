# Coverage: native

Profiles are collected from one comprehensive exercise executable. The full CTest suite is run separately as a correctness gate.
No profiles from different executables or backends are merged.

| Metric | Covered / total | Percent |
|---|---:|---:|
| Distinct instrumented source lines (LCOV) | 4788 / 4831 | 99.1099% |
| Distinct source branch outcomes | 2081 / 2206 | 94.3336% |
| LLVM summary lines (unmodified) | 4288 / 4476 | 95.7998% |
| LLVM summary branches (unmodified) | 1867 / 2033 | 91.8347% |
| LLVM summary functions (unmodified) | 513 / 517 | 99.2263% |
| LLVM summary regions (unmodified) | 3212 / 3380 | 95.0296% |

The two line metrics are not interchangeable. LLVM summary totals include overlapping template mappings; LCOV counts each physical instrumented source line once.
These figures do not prove every template instantiation, platform-specific branch, or possible input is covered.

## Files and remaining gaps

| File | Source lines | Percent | Uncovered line numbers |
|---|---:|---:|---|
| `include/fixedwide/arithmetic.hpp` | 622 / 622 | 100.000% | none |
| `include/fixedwide/batch.hpp` | 135 / 135 | 100.000% | none |
| `include/fixedwide/binary.hpp` | 144 / 144 | 100.000% | none |
| `include/fixedwide/chars.hpp` | 37 / 37 | 100.000% | none |
| `include/fixedwide/checked.hpp` | 119 / 119 | 100.000% | none |
| `include/fixedwide/detail/batch_multiply.hpp` | 148 / 151 | 98.013% | 75, 76, 139 |
| `include/fixedwide/detail/constexpr_arith.hpp` | 244 / 247 | 98.785% | 190, 195, 196 |
| `include/fixedwide/detail/decimal_scan.hpp` | 51 / 51 | 100.000% | none |
| `include/fixedwide/detail/from_float.hpp` | 85 / 85 | 100.000% | none |
| `include/fixedwide/detail/mixed_native.hpp` | 292 / 298 | 97.987% | 141, 143, 220, 259, 309, 436 |
| `include/fixedwide/detail/overflow.hpp` | 12 / 12 | 100.000% | none |
| `include/fixedwide/detail/to_float.hpp` | 138 / 138 | 100.000% | none |
| `include/fixedwide/fixed.hpp` | 164 / 164 | 100.000% | none |
| `include/fixedwide/floating.hpp` | 29 / 29 | 100.000% | none |
| `include/fixedwide/format.hpp` | 53 / 53 | 100.000% | none |
| `include/fixedwide/hash.hpp` | 41 / 41 | 100.000% | none |
| `include/fixedwide/iostream.hpp` | 20 / 20 | 100.000% | none |
| `include/fixedwide/literal.hpp` | 39 / 51 | 76.471% | 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84 |
| `include/fixedwide/mixed.hpp` | 182 / 193 | 94.301% | 158, 251, 252, 288, 289, 328, 329, 354, 355, 384, 385 |
| `include/fixedwide/string.hpp` | 30 / 30 | 100.000% | none |
| `include/fixedwide/wide.hpp` | 509 / 509 | 100.000% | none |
| `src/arithmetic.cpp` | 392 / 394 | 99.492% | 69, 70 |
| `src/bigint.cpp` | 68 / 68 | 100.000% | none |
| `src/chars.cpp` | 352 / 357 | 98.599% | 269, 270, 271, 272, 273 |
| `src/detail.hpp` | 74 / 74 | 100.000% | none |
| `src/division.hpp` | 79 / 79 | 100.000% | none |
| `src/floating.cpp` | 74 / 74 | 100.000% | none |
| `src/integer_chars.cpp` | 133 / 134 | 99.254% | 88 |
| `src/limbs.hpp` | 271 / 271 | 100.000% | none |
| `src/mixed.cpp` | 139 / 139 | 100.000% | none |
| `src/native.hpp` | 112 / 112 | 100.000% | none |
