# Checked or unchecked: choose at the call site

fixedwide separates decimal representation from error-handling policy. The same
`Fixed64<8>` can be used with checked functions at an input boundary and ordinary
arithmetic inside a domain whose bounds are already established. There is no
second storage type, wrapper, runtime policy flag or conversion between modes.

This API is available since v0.6.4; earlier releases retain the checked API only.

```cpp
#include <fixedwide/all.hpp>
namespace fw = fixedwide;
using Price = fw::Fixed64<8>;

constexpr Price initial = "125.50";
constexpr Price delta = "0.0025";
Price price = initial;
price += delta;                         // caller guarantees the sum fits
const Price change = price - initial;   // a value, not std::expected

auto checked = fw::add(Price::max(), delta);
// checked.error() == fw::ArithmeticError::overflow
```

Use the checked APIs when failure is possible and must be handled. Use operators
or `fw::unchecked` when the application establishes their preconditions. Neither
choice requires changing the representation, decimal scale or serialization.
A lack of past production failures is not a bound.

## The contract

For every unchecked call, the caller guarantees:

- The mathematical result, after the requested rounding, fits the destination.
  Negation and absolute value therefore exclude `T::min()`.
- Every divisor is nonzero. Precision arguments are valid for the operation, and
  the rounding argument is one of the six supported `Rounding` enumerators.
- `Rounding::exact` is used only when no nonzero digit/remainder is discarded.

Violating a precondition is **undefined behaviour**. There is no promise of
wrapping, saturation, an exception, an error value, or even a deterministic
failure. Do not use unchecked arithmetic to implement a fallback on overflow.
Use the existing checked function instead.

Debug builds with assertions enabled diagnose many violations. With `NDEBUG`,
these diagnostics disappear. Assertions are not input validation and do not
establish a recoverable error contract. Valid inputs have the same numerical
result in both configurations. Keep `NDEBUG` consistent across translation units
that instantiate these inline functions, as with other assertion-bearing headers.

Same-type constant evaluation uses the checked constexpr implementation; invalid
constant expressions are rejected rather than establishing a wrapping contract.
Mixed helpers retain the constexpr availability of their checked counterparts.

## Operators and named functions

Include `<fixedwide/unchecked.hpp>` or `<fixedwide/all.hpp>`. Including only
`fixed.hpp` or `arithmetic.hpp` does not opt a translation unit into operators.
No `using namespace` directive is needed for operator lookup.

| Intent | Checked, returning `std::expected` | Caller-responsibility, returning a value |
|---|---|---|
| Add / subtract | `fw::add(a, b)`, `fw::sub(a, b)` | `a + b`, `a - b`; `fw::unchecked::add/sub` |
| Multiply / divide | `fw::mul(a, b)`, `fw::div(a, b)` | `a * b`, `a / b`; `fw::unchecked::mul/div` |
| Fused multiply-divide | `fw::mul_div(a, b, c, mode)` | `fw::unchecked::mul_div(a, b, c, mode)` |
| Remainder | `fw::remainder(a, b)` | `a % b`; `fw::unchecked::remainder` |
| Negation / magnitude | `fw::negate(a)`, `fw::abs(a)` | `-a`; `fw::unchecked::negate/abs` |
| Integer construction | `fw::from_integer<T>(n)` | `fw::unchecked::from_integer<T>(n)` |
| Quantize / midpoint | `fw::quantize(...)`, `fw::midpoint(...)` | `fw::unchecked::quantize/midpoint(...)` |
| Convert / mixed arithmetic | `fw::fixed_cast<T>(...)`, `fw::mul_to<T>(...)`, etc. | Corresponding names under `fw::unchecked` |

Unary `+` and `+=`, `-=`, `*=`, `/=`, `%=` are also provided. Compound assignments
return the left operand by reference and have the same preconditions as the
corresponding binary operation. The checked functions keep their existing names,
return types, errors and behaviour.

Operators require the same fixed-point type on both sides. Width and scale
changes still need an explicit destination. No implicit integer or floating-point
conversion is added. Write `Price{"0"}`, `Price{}`, or a typed constant rather than
expecting `price += 1` to choose whether `1` means one raw unit or one whole unit.
Prefer qualified `fw::unchecked::...` names over importing both function families.

For a same-scale midpoint with a compile-time, non-exact rounding policy, the
existing `fw::midpoint<fw::Rounding::nearest_even>(a, b)` is already value-returning
and infallible over the full input range. It does not need an unchecked assumption.

## Rounding and intermediate precision do not change

`*` and `/` round once to the existing type's scale using nearest-even, just like
`fw::mul` and `fw::div` with their default policy. Named unchecked functions accept
an explicit rounding mode. `unchecked::fixed_cast` defaults to exact, like its
checked counterpart; other rounding defaults mirror the checked arithmetic API.

```cpp
using Price = fw::Fixed64<8>;
using Integer = fw::Fixed64<0>;
constexpr Price price = "125.50";
constexpr Integer buffer_bps = "25";
constexpr Integer basis_points = "10000";
auto buffer = fw::unchecked::mul_div_to<Price>(
    price, buffer_bps, basis_points, fw::Rounding::ceil);
```

Unchecked does **not** mean multiplying raw integers at the storage width. A
`Fixed64<12>` product still retains the full 128-bit intermediate before rescaling.
For example, `123.456789012345 * 2.000000000000` is valid even though the product of
its two raw integers does not fit in 64 bits. The rounded destination must fit.

`a * b / c` performs two operations and can round twice or overflow at the first
result. Use `unchecked::mul_div(a, b, c, mode)` for one final rounding and a widened
product. Likewise, mixed `*_to<Dest>` operations preserve their existing one-rounding
semantics. Disabling overflow reporting does not disable rounding decisions.

Remainder has the sign of the dividend. `T::min() % T::from_raw(-1)` is explicitly
zero, not the native signed-integer overflow corner case.

## Establish bounds once; keep the calculation readable

Suppose validated input prices satisfy `abs(price) <= 1,000,000` and a validated
shift satisfies `abs(delta) <= 10,000`. One application of the shift satisfies
`abs(price + delta) <= 1,010,000`, well within `Fixed64<8>`'s range. Every level can
then use the straightforward loop:

```cpp
for (auto& level : levels)
    level.price += delta;
```

The proof covers every level, not just the best bid or ask. It covers one shift;
repeated accumulation needs its own bound or renewed validation. Changing the
accepted range, scale, or number of iterations requires revisiting the proof.
For an unbounded accumulation, keep `fw::add` and handle its error.

[Example 09](../examples/09_bounded_arithmetic.cpp) validates runtime text and
application bounds before entering this loop. Its validation returns an error
before mutation. No per-level overflow result handling is needed in the loop.

Positivity, crossed books, ordering, missing sides and removal of levels are
separate application rules. Neither checked nor unchecked arithmetic decides
those policies. A numeric migration should not silently change them.

## Performance: what is and is not check-free

With `NDEBUG`:

| Path | Implementation |
|---|---|
| Same-type add, subtract, negate and abs; every width | Direct raw arithmetic, no overflow-result construction/check |
| Same-type mul, div and mul_div; 8/16/32-bit storage | Full signed 64-bit intermediate, no overflow-result construction/check |
| Same operations; 64-bit storage on x86-64 GCC/Clang native backend | Full signed 128-bit product and division via assembly, no overflow guard |
| Native-width remainder | Direct remainder with the signed-min/-1 special case |
| Portable 64-bit and 128/256-bit scaled products/divisions | Value-returning adapters over existing checked numerical kernels |
| Mixed operations, conversions, quantize and midpoint | Value-returning adapters over existing checked numerical kernels |

The last two rows remove call-site error handling, **not necessarily the kernel's
internal checks or `std::expected` machinery**. Inlining may remove some overhead;
this is not guaranteed, especially across a compiled-library boundary. No universal
speedup is claimed. Benchmark the chosen width, scale, backend, rounding mode and
workload. Debug assertion builds deliberately do extra validation and are not the
check-free performance configuration.

The existing checked instruction-count regression gate and its committed baseline
remain unchanged. Unchecked convenience is not a reason to weaken checked safety,
rounding guarantees, portable correctness, or the performance regression threshold.
