# Coverage and numerical boundary tests

Coverage is a way to find missing tests, not a proof of numerical correctness.
The regression suite uses independent Boost.Multiprecision integer/rational
oracles and always-active `CHECK` assertions, including in Release builds.

## Reproduce

Use matching Clang, `llvm-cov`, and `llvm-profdata` releases. For example:

```sh
cmake -B build-native -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=clang++-20 -DFIXEDWIDE_COVERAGE=ON \
  -DFIXEDWIDE_BUILD_TESTS=ON -DFIXEDWIDE_BUILD_ORACLE_TESTS=ON
cmake --build build-native
LLVM_COV=llvm-cov-20 LLVM_PROFDATA=llvm-profdata-20 \
  scripts/coverage.sh build-native coverage/native native
```

Repeat in a different build/output directory with
`-DFIXEDWIDE_FORCE_PORTABLE=ON`, passing `portable` as the last argument.
Boost headers are test-only dependencies; they do not enter installed headers.

The driver first runs **all CTest tests**. It then runs the comprehensive
`test_coverage all` executable to collect a coherent coverage map. Its five
suites also run as separate CTest cases. A unity translation unit avoids
incompatible duplicate template maps across independently linked tests. The
implementation library remains separately compiled as usual.

Native and portable profiles are **never merged with each other**. Each backend
has a separate JSON/LCOV export, summary, gate, executable checksum, tool version,
source commit, working-tree status, CTest log and assertion count. Missing source
translation units, empty reports, malformed counts, and mismatched coverage maps
are errors, not successful zero-coverage runs.

## Two different line metrics

We publish both metrics, without replacing LLVM's totals:

* **Distinct instrumented source lines:** union of LLVM LCOV `DA` records by
  source filename and line number. Each physical instrumented line is counted
  once. The gate is **99%** independently for each backend.
* **LLVM summary lines:** the unmodified `llvm-cov report` / JSON total. LLVM's
  template/overlapping region maps can produce a different denominator and
  covered count from its LCOV source-line records. The gate is **95%**, and the
  coverage badge continues to show this conservative native summary, not 99%.

The unmodified LLVM branch gate is **90%**. The report additionally lists
source-location branch outcomes. Neither branch figure is claimed to reach 99%.
Function and region counts are retained as well.

The initial local Clang 17 measurement after this work exceeded 99% distinct
source lines, while the LLVM summaries remained below 99%. This is **not** a
claim of 99% coverage under every metric, nor of 100% coverage. See the exact
per-backend output and remaining uncovered lines in the CI artifacts. The
previous multi-executable, multi-backend merged report emitted incompatible-map
warnings, so its old percentage is not an apples-to-apples denominator.

No per-function, per-branch or `LCOV_EXCL` exclusions are used. All instrumented
library files under `src/` and `include/fixedwide/` belong in the report;
standard-library, test, example, fuzz and benchmark code does not. Compile-time
only helpers are also exercised at runtime against independent oracles where
that is meaningful. Uninstantiated template combinations, other platforms and
uncompiled preprocessor branches are not magically covered by a percentage.

## What the comprehensive suite checks

- Wide signed and unsigned arithmetic, limb carries, borrows, all shift
  boundaries, signed minima and normalized quotient estimation.
- Public same-domain operations, all six rounding policies, extrema, randomized
  operands, checked construction, scaling and overflow after rounding.
- Explicit-target mixed arithmetic and cross-scale comparison, including the
  512/1024-bit paths and representable tiny results with directed rounding.
- Full-precision and reduced-precision text, canonical output, scientific
  notation, sticky digits, malformed input, buffer boundaries and huge exponents.
- Endian-defined binary round trips, unaligned input, hashing, stream errors,
  format-specifier errors and exact binary-float ingress.

Each assertion is evaluated; the test driver prints the executed assertion
count. Coverage does not replace the existing sanitizer, fuzz, instruction-count
or platform gates.

## Correctness defects exposed by these tests

1. Signed wide `divmod(min, -1)` did not reject quotient overflow.
2. Native reduced-digit `Fixed128` formatting discarded the division remainder
   before deciding how to round.
3. Fixed-to-`float` assembled zero high limbs using `0 * infinity`, returning NaN
   even for zero. Assembly now uses at least `double` before narrowing.
4. Very small nonzero floating inputs lost their remainder, breaking `exact`,
   `floor` and `ceil`. Significand extraction also now preserves more than 64
   bits on platforms with binary128 `long double`.
5. The generic Knuth quotient-correction loop reused a stale `qhat * v2`
   product after decrementing `qhat`, occasionally returning a quotient one unit
   too small. A deterministic mixed-scale regression and direct divider case
   reproduce this issue.

The portable arithmetic path also no longer selects `_BitInt(256)` internally
when `FIXEDWIDE_FORCE_PORTABLE` is requested. Obsolete helpers hidden behind
contradictory preprocessor conditions were removed, not excluded from coverage.
