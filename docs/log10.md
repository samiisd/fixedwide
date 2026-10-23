# Exact decimal exponents

`<fixedwide/log10.hpp>` adds one query, also included by `all.hpp`:

```cpp
using F = fixedwide::Fixed64<2>;
static_assert(*fixedwide::log10_floor(F::from_raw(999)) == 0);   // 9.99
static_assert(*fixedwide::log10_floor(F::from_raw(1000)) == 1);  // 10.00
static_assert(*fixedwide::log10_floor(F::from_raw(99)) == -1);   // 0.99
static_assert(*fixedwide::log10_floor(F::from_raw(1)) == -2);    // 0.01
```

The result is `std::expected<int, ArithmeticError>`. A positive value returns the
unique exponent `e` for which `10^e <= value < 10^(e+1)`. Zero and negative
values, including signed minima, return `ArithmeticError::invalid_value`.
This is not the logarithm of an absolute value and is **not a fractional log10**.
There is no rounding option: the mathematical floor is exact.

The same API supports every `basic_fixed<Bits, D>` width and scale. It is
`constexpr`, `noexcept`, allocation-free, and header-only. Neither the input nor
the result passes through floating point. Existing arithmetic, rounding,
conversion, storage, error values and performance baselines are unchanged.

## Algorithm and limits

For positive raw storage `r`, the answer is `floor(log10(r)) - D`.
Let `b = bit_width(r)` and `t = (b * 1233) >> 12`. For every supported positive
raw bit width, `1 <= b <= 255`, the true raw decimal exponent is either `t` or
`t-1`. Comparing the integer with an exact, compile-time table entry `10^t`
selects the answer. Wide inputs locate their most significant nonzero limb
before the same estimate and one wide comparison. There are no runtime
floating-point operations, divisions, decimal-conversion loops or allocations.

This bound is intentionally limited to the library's signed storage widths; it
is not an unbounded approximation formula. Tests check both endpoints of every
binary bin, every representable power of ten and its neighbours, every scale,
signed limits, exhaustive positive 8/16-bit raw values, and deterministic random
values against an independent repeated-division integer oracle. Error paths and
constant evaluation are tested separately. Do not use `floor(log10(double(x)))`
as the correctness oracle: both conversion and the logarithm can round across
a decimal boundary.

## Reproduce the comparison

```bash
CXX=g++-14 ITERATIONS=262144 REPETITIONS=11 \
  bash scripts/bench_log10.sh reports/log10-local
```

The script builds native and forced-portable Release configurations, compiles
and runs the focused test without linking the library, saves the compiler/CPU/
libc environment, pins timing to one available CPU when possible, and retains
raw timing samples, mismatch counts, and generated assembly. `source.tar.gz`
in the output is the committed source snapshot; consult `git status` in the
metadata before treating a dirty local run as a measurement of that snapshot.

Four widths (32, 64, 128 and 256 bits) and three 1,024-value, deterministic,
shuffled distributions are measured: full-width uniform raw values,
log-uniform bit widths, and powers of ten with immediate neighbours. Checked
fixedwide results are validated against an integer oracle before timing.
Floating-point discrepancies are counted, not hidden or asserted away.

The comparison distinguishes three questions:

| Suffix | Work inside the timed loop |
| --- | --- |
| `fixedwide` | Checked exact `log10_floor(fixed)` and consume its integer result |
| `double_e2e`, `long_double_e2e` | Public fixed-to-float conversion, `std::log10`, `std::floor`, integer result |
| `double_floor_preconverted`, `long_double_floor_preconverted` | `std::log10`, `std::floor`, integer result; conversion excluded |
| `double_log_only`, `long_double_log_only` | Full fractional `std::log10` on preconverted input; no floor or integer conversion |

Only the end-to-end rows start from the same fixed-point representation. The
floating baselines are approximations, not exact substitutes. The log-only rows
do more mathematical work than an integer exponent query. Timing does not make
those contracts interchangeable.

The shared `measurement.hpp` harness emits every sample and min/median/p95/max.
These are scalar, hot-corpus throughput measurements including loop, indexing
and observable-sink overhead, not dependency-chain latency. Vectorisation is
disabled, fast-math is not enabled, setup and validation are outside timing, and
runtime input plus a per-iteration compiler barrier prevents constant folding
and repeated-corpus hoisting. Implementations run sequentially, not as a paired
randomised experiment. Shared-runner timings are descriptive; rerun on the
deployment CPU for a performance decision.

The existing instruction-count regression job also collects this comparison.
No new workflow, relaxed threshold, or historical baseline replacement is
needed. Wall-clock results never decide the instruction-count gate.
