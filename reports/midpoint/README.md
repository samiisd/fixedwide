# Checked midpoint: implementation and local evidence

`midpoint(a, b, rounding = Rounding::nearest_even)` computes `(a + b) / 2`
at the shared operand scale. It is symmetric, `constexpr`, `noexcept`, and
returns `std::expected<Fixed, ArithmeticError>`. All six widths and all six
rounding modes are supported. Only `exact` can fail, with `inexact` for an odd
raw sum. There is no mixed-type overload or implicit rounding toward the first
operand.

## Why the implementation is small

For signed two's-complement raw integers, with an arithmetic right shift:

```text
lower = (a & b) + ((a ^ b) >> 1) = floor((a + b) / 2)
odd   = ((a ^ b) & 1) != 0
```

The sum itself is never formed. `lower` is between the operands, so it fits;
when `odd` is true it is strictly less than the maximum raw value. Rounding is
therefore a safe zero-or-one increment, or an `inexact` error. The same code
works with the existing signed wide types: no wider storage, division, scale
conversion, native extension, allocation or extra header is required.

The function has 35 added lines including its API/proof comments. It does not
change any existing arithmetic path or previously recorded instruction-count baseline.

## Executed local validation

GCC 14.2.0, C++23, x86-64 Linux; both release builds include examples and the
optional Boost oracle suite.

| Configuration | Result |
|---|---|
| Release, native backend | 45/45 CTest entries passed |
| Release, forced-portable backend | 45/45 CTest entries passed |
| Debug, native ASan + UBSan, focused midpoint tests | 2/2 CTest entries passed |
| New unit test, oracle and benchmark compiled separately with strict warnings and `-Werror` | Passed |

Each midpoint test run executes **2,453,409** dependency-free checks and
**6,778,197** Boost oracle checks. Coverage includes exhaustive pairs for every
Fixed8 scale, all rounding modes, constexpr checks, rejected mixed types,
negative ties, equal endpoints, full-range cancellation, every limb carry
boundary, full-width deterministic random inputs, and the motivating
`1.00000001, 1.00000003 -> 1.00000002` regression. The oracle forms the exact
arbitrary-precision sum and uses quotient/remainder rounding, not the production
bit identity.

The new benchmark performs **64,767** independent oracle/checksum checks per
backend. Every measured result contributes to a checksum, including errors.

These are local results, not a claim that the PR's cross-platform CI has
completed. MSVC, Clang, AArch64, the pinned clang-format 22.1.8 check, and the
existing Valgrind regression gate remain CI checks; those tools were unavailable
in the local environment.

## Measured throughput

AMD EPYC 9V74, shared KVM container, one process pinned to CPU 0. GCC 14.2.0,
CMake Release (`-O3 -DNDEBUG`), no vectorization; 1,048,576 operations per sample,
11 repetitions, 256 runtime-generated operand pairs. Both backends were run
sequentially. Numbers include loop/checksum overhead and are independent-operation
**throughput, not dependency-chain latency**. Wall-clock timings are diagnostic,
not a portable speed guarantee or a new CI threshold.

`selected.csv` retains min/median/p95/max and all 11 raw samples for the following
nearest-even quote rows from the local runs. The executable additionally covers
all six compile-time policies and a runtime mixed-policy row, for quote and
full-width mixed-sign fixtures: 126 rows per backend on this compiler.

| Type / quote / nearest-even | Native midpoint (ns/op) | Native raw `__int128` reference | Native widened fixedwide reference | Portable midpoint | Portable widened fixedwide reference |
|---|---:|---:|---:|---:|---:|
| Fixed64<8> | 0.957842 | 1.721431 | 32.509345 | 0.899076 | 85.216256 |
| Fixed64<12> | 0.871236 | 1.692903 | 31.022901 | 0.866450 | 88.560142 |
| Fixed128<12> | 1.990611 | n/a | 33.651365 | 2.003641 | 90.801778 |
| Fixed256<12> | 12.401083 | n/a | n/a | 12.694468 | n/a |

Both reference implementations form a wider sum and round **once**. Neither
uses the incorrect separately rounded halves. The raw `__int128` comparator is
benchmark-only and available only when the compiler provides it, even when the
library itself is forced portable. There is no wider fixedwide type to use as
a Fixed256 reference; its answers are still independently checked with Boost.

GCC assembly inspection of Fixed64<8> wrappers that unwrap guaranteed-success
modes found no branches, divisions or calls: nearest-even is 9 instructions plus
`ret`, floor 5 plus `ret`, and toward-zero 10 plus `ret`. These counts describe
those wrappers, not a claim about every `std::expected` calling convention.

## Reproduce

```sh
cmake -B build-midpoint -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DFIXEDWIDE_BUILD_TESTS=ON -DFIXEDWIDE_BUILD_EXAMPLES=ON \
  -DFIXEDWIDE_BUILD_ORACLE_TESTS=ON -DFIXEDWIDE_BUILD_BENCHMARKS=ON
cmake --build build-midpoint
ctest --test-dir build-midpoint --output-on-failure
./build-midpoint/benchmarks/fixedwide_midpoint_bench 1048576 11 > native.csv

# Repeat in a separate build with -DFIXEDWIDE_FORCE_PORTABLE=ON.
# For sanitizers, configure Debug with -DFIXEDWIDE_SANITIZE=ON, build
# test_midpoint and audit_midpoint, then run ctest -R midpoint.
```

## Regression protection

Midpoint uses the existing **instruction-count regression gate** in `ci.yml`,
not a separate workflow. `fixedwide_icount` now includes nine midpoint rows:
all six rounding policies at scale 8, nearest-even at scale 12, and nearest-even
for Fixed128/Fixed256. Full-width, mixed-sign fixtures include equal minimum
and maximum endpoints. Each value or error feeds the existing digest/sink.

The original 34 workload implementations, fixtures and baseline rows are
unchanged, as are `scripts/icount.sh`, `scripts/compare_icount.py` and the 1%
threshold. New midpoint baselines are recorded from the same GCC 14/Valgrind
job; its existing `instruction-counts` artifact retains the measured CSV.

The standalone wall-clock executable above remains an optional local diagnostic.
Correctness, including native/portable runs, stays in the existing test matrix.
There is no midpoint-specific CI job.


The nine new baseline rows were recorded from the existing job in
[CI run 34624169260](https://github.com/samiisd/fixedwide/actions/runs/34624169260/job/103345115837),
using GCC 14 on ubuntu-24.04 and the unchanged default 20,000/40,000-iteration
Callgrind measurement. All 34 historical workloads passed against their original
baselines. Only the nine new rows were appended; none of the old numbers changed.
Applying the updated baseline to the retained measurement passes all 43 rows.
A synthetic +2% midpoint measurement correctly fails the unchanged 1% comparator.

The added counter workloads were also checked locally: 90 checksum comparisons
against an independent Python exact-integer oracle, 102 comparisons confirming
unchanged results for the old workloads, and a strict GCC 14 warnings build.
