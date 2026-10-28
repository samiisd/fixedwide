# Checked arithmetic over SoA fields

`<fixedwide/batch.hpp>` operates directly on existing typed spans. A scalar
argument broadcasts; a span supplies one value per row. The API does not change
the stored representation, allocate, start threads, or require SIMD alignment.
It is also included by `<fixedwide/all.hpp>`.

The implementation first improves the arithmetic: exact products can recover a
smaller coefficient without division, and prepared broadcasts cancel decimal
factors once and reuse division setup. Optional blocks of at least four rows
also enable a checked AVX2 path for supported exact broadcasts. Commits retain
physical row order. The default maps and commits one row at a time; select
larger blocks using complete-loop measurements.

## Array operations

```cpp
#include <fixedwide/batch.hpp>
#include <fixedwide/literal.hpp>
#include <span>

namespace fw = fixedwide;
using Decimal = fw::Fixed128<12>;
using Price = fw::Fixed64<10>;

// output and slopes have equal sizes. Only ordinary type alignment is needed.
auto result = fw::batch::mul_to<Decimal>(output, slopes, Price{"0.00001"},
                                        fw::Rounding::exact);
if (!result) {
    const auto row = result.error().index;
    const auto cause = result.error().error; // unchanged ArithmeticError
    // output[0..row) was written; output[row..size) is unchanged.
}
```

Pass `std::span{vector}`, `std::span{array}` or an existing span. Mutable and
const spans, including fixed extents, are accepted. The output must be mutable.
Scalar inputs are copied, including a scalar taken from an output element.

| Function | Operation in each row | Default rounding |
|---|---|---|
| `batch::add(out, a, b)` | Checked same-type addition | None |
| `batch::sub(out, a, b)` | Checked same-type subtraction | None |
| `batch::mul_to<Dest>(out, a, b, mode)` | Multiply and round into `Dest` | nearest-even |
| `batch::add_to<Dest>(out, a, b, mode)` | Mixed-width/scale addition | nearest-even |
| `batch::sub_to<Dest>(out, a, b, mode)` | Mixed-width/scale subtraction | nearest-even |
| `batch::div_to<Dest>(out, a, b, mode)` | Mixed-width/scale division | nearest-even |
| `batch::mul_div_to<Dest>(out, a, b, c, mode)` | One final rounding of `a*b/c` | nearest-even |
| `batch::fixed_cast<Dest>(out, a, mode)` | Fixed-width/scale conversion | exact |

All six scalar rounding modes are supported: exact, toward-zero, floor, ceil,
nearest-even and nearest-away. The wrappers retain all supported scalar widths
and scales; an unoptimized combination delegates to the existing scalar API.

## Independent work and ordered updates

For dependent destinations, use `for_each(count, map, commit, inputs...)`.
`map` computes an independent contribution; `commit` applies it in row order.
Duplicate balance indices are permitted and are not treated as independent.

```cpp
using Status = std::expected<void, fw::ArithmeticError>;
const auto delta = fw::batch::prepare_mul_to<Decimal>(dx, fw::Rounding::exact);

auto result = fw::batch::for_each(slopes.size(), delta,
    [&](std::size_t row, Decimal contribution) noexcept -> Status {
        const auto b = balance[row];
        const auto after = fw::add(value[row], contribution);
        if (!after) return std::unexpected(after.error());
        const auto sum = fw::add(pnl[b], contribution);
        if (!sum) return std::unexpected(sum.error());
        value[row] = *after;
        pnl[b] = *sum;
        return {};
    }, slopes);
```

This checks both row updates before writing either, while retaining earlier
accepted rows. The caller still owns scope certification, sink/hole policy,
readiness, touched-balance bookkeeping and any wider refusal/recovery protocol.
No index array is required for contiguous output; no unique-destination
assumption is made for caller-controlled updates.

The prepared multiplier owns the broadcast value and rounding mode and can be
reused for several spans. Its preparation cost is paid once. For very short
spans, ordinary scalar calls can be cheaper; do not rebuild a plan for every
row when its operand is invariant.

## Four-row SIMD without changing storage

For an exact broadcast, select a block of at least four rows explicitly:

```cpp
const auto multiply = fw::batch::prepare_mul_to<Decimal>(dx, fw::Rounding::exact);
auto result = fw::batch::transform<4>(output, multiply, slopes);
// The same map works with for_each<4>(count, multiply, commit, slopes)
// when the contributions feed ordered updates to duplicate destinations.
```

On GCC/Clang x86-64 native builds, a runtime CPU check selects AVX2 once per
invocation. The library and caller need no global `-mavx2` or `-march=native`.
Other CPUs, portable builds, other rounding modes, unsupported types and short
spans retain the scalar implementation. The supported shape is a built-in
prepared broadcast, one input span of `Fixed128`, a `Fixed128` destination,
a factor of at most 64 bits, and a decimal scale reduction from 1 through 19.
An arbitrary user lambda is still evaluated through the generic scheduler.

Four existing 128-bit values are transposed into low/high limb vectors in
registers. For `d = 10^k`, the kernel recovers a candidate coefficient modulo
`2^32` by shifting out `2^k` and multiplying by the modular inverse of `5^k`.
It reconstructs the **entire 128-bit input** as `q*d` and compares both limbs.
Only a matching lane uses the packed multiply. A failed match invokes the
ordinary prepared multiplier for that lane; it is not an arithmetic error.
In particular, a non-grid input can still produce an exact result through
cancellation by the broadcast factor.

The general vector path admits coefficient magnitudes through `UINT32_MAX`
and full signed64 factors. Their product fits 128 bits. For a scale reduction
of ten and a signed32 factor, a shorter signed32-coefficient path produces a
signed64 product and sign-extends it into the full 128-bit result. Other
coefficients use scalar fallback. These are checks of each actual value,
not restrictions on the API's supported range.

Successful maps are committed in order, including duplicate destinations.
An earlier commit error still outranks a later lane's arithmetic error. The
transform writes only the successful prefix, supports exact in-place use,
and requires only ordinary type alignment. There is no allocation, extra
array pass, persistent packed copy, or floating-point approximation.

SIMD is not uniformly faster: wide or non-grid coefficients can reject every
candidate, and dependent checked additions may dominate a complete update.
Use block size 1 for those workloads when measurement favors it.

### When the caller can choose a primary coefficient representation

A cent-grid `Fixed128<12>` slope has `raw = q * 10^10`. If that grid is an
invariant of the field, storing the slope directly as `Fixed128<2>` makes
`Fixed128<2> * Fixed64<10> -> Fixed128<12>` require no decimal rescale.
This uses the same 16 bytes per input and can represent every cent-grid value
of the original type. Preparation belongs where the coefficient changes,
and conversion failure needs the caller's ordinary wide/non-grid path.

`Fixed32<2>` reduces input bandwidth further and exposes ordinary packed
integer multiplication when the factor also fits signed32. It requires a
separately checked range contract; a general cent-grid coefficient from the
original type needs up to 95 signed bits. Changing the primary field is a
caller design choice. Converting a complete array on every quote adds a pass
and scratch traffic, and can cost more than the arithmetic it removes.

## Composing a row kernel without an array temporary

`batch::mul_to<Dest>(mode)` with no output span returns an optimized binary
operation. `prepare_mul_to<Dest>(factor, mode)` returns a unary operation.
Both return the same `expected<Dest, ArithmeticError>` as scalar multiplication.
They also work in an ordinary scalar loop, making it possible to separate the
benefit of prepared arithmetic from the benefit of blocking.

Use them in `transform(out, map, inputs...)` or the map stage of `for_each`:

```cpp
const auto product = fw::batch::mul_to<Decimal>(fw::Rounding::nearest_even);
const auto convert = fw::batch::prepare_mul_to<Decimal>(fx, fw::Rounding::nearest_even);

auto result = fw::batch::transform(next,
    [&](Decimal s, Decimal c, Price anchor) noexcept
        -> std::expected<Decimal, fw::ArithmeticError> {
        // Caller has validated the economic-price range for this subtraction.
        const auto dx = Price::from_raw(mark.raw() - anchor.raw());
        const auto amount = product(s, dx);
        if (!amount) return std::unexpected(amount.error());
        const auto subtotal = fw::add(*amount, c);
        if (!subtotal) return std::unexpected(subtotal.error());
        return convert(*subtotal);
    }, slopes, residuals, anchors);
```

Each multiplication rounds separately. The product and the addition must both
fit `Decimal` before FX is applied. A small FX rate cannot rescue an earlier
overflow. A map can return a small aggregate if commit needs several results.

For replacement, keep the required wide temporary inside the ordered commit:

```cpp
using WideMoney = fw::Fixed256<12>;
auto removed = fw::sub_to<WideMoney>(pnl[b], previous[row], fw::Rounding::exact);
if (!removed) return std::unexpected(removed.error());
auto replaced = fw::add_to<WideMoney>(*removed, next, fw::Rounding::exact);
if (!replaced) return std::unexpected(replaced.error());
auto total = fw::fixed_cast<Decimal>(*replaced, fw::Rounding::exact);
if (!total) return std::unexpected(total.error());
pnl[b] = *total;
previous[row] = next;
```

Thus `h - (-h) + (-h) == h` succeeds even when the removed subtotal does not fit
`Decimal`. The batch API does not replace the wide expression with two narrow
checked operations.

## Ordered reductions and position boundaries

`reduce_add(values, initial)` adds in input order. `transform_reduce(count,
initial, map, inputs...)` first computes independent terms and adds each term in
that same order. Both return `expected<T, batch::Error>` and leave the caller's
initial value and input storage unchanged. Every intermediate sum is checked;
these functions do not reassociate into a tree reduction.

For publication, invoke the reduction on one position's span, then apply FX:

```cpp
const auto entry_product = fw::batch::mul_to<Decimal>(fw::Rounding::nearest_even);
auto sum = fw::batch::transform_reduce(position_slopes.size(), Decimal{},
    [&](Decimal slope, Price execution) noexcept {
        return entry_product(slope, Price::from_raw(mark.raw() - execution.raw()));
    }, position_slopes, position_executions);
if (!sum) return std::unexpected(sum.error());
auto position_pnl = convert(*sum);
```

Round each entry before summing. Two half-quantum products can each round to
zero even when multiplying a summed slope would produce one quantum. Preserve
position boundaries: the caller clips the final converted position amount and
then adds the resulting contribution to its balance. Clipping, threshold
caching, disabled limits and other business policies stay with the caller.

## Comparisons, masks and floating conversions

A map may return a plain value for an infallible operation, or an
`expected<T, ArithmeticError>` for a checked operation. This also covers
comparison, select, min/max and existing floating conversion contracts without
introducing another arithmetic policy:

```cpp
auto compared = fw::batch::transform(mask,
    [](Decimal a, Decimal b) noexcept { return a <= b; }, equity, threshold);
auto selected = fw::batch::transform(selected_values,
    [](bool choose, Decimal a, Decimal b) noexcept { return choose ? a : b; }, mask, a, b);
auto converted = fw::batch::transform(doubles,
    [](Decimal x) noexcept { return fw::to_double(x); }, amounts);
auto restored = fw::batch::transform(amounts,
    [](double x) noexcept { return fw::from_double<Decimal>(x, fw::Rounding::nearest_even); }, doubles);
```

Include `<fixedwide/floating.hpp>` for floating conversions. A boolean mask
must be contiguous actual `bool` storage, such as `std::array<bool, N>`;
`std::vector<bool>` is not a span. An integer mask works with a correspondingly
typed lambda. These adapters preserve existing scalar conversion guarantees;
they do not extend Fixed256's floating guarantees or add a SIMD conversion.
Infallible transforms use a direct loop with no error-result objects, allowing
the compiler to vectorize suitable operations. For application-specific tagged
wrappers, use a map that reads `amount.value()` and a commit that writes the
wrapper through its existing API; the stored fields need no conversion pass.

## Failure, aliasing and scratch contract

* Each input span has exactly the requested count; this is a precondition,
  asserted in debug builds. Shape errors are not `ArithmeticError` values.
* An empty batch succeeds and does not invoke map or commit. A reduction returns
  its initial value. Tail rows use the same semantics as full blocks.
* At the first failing row `i`, `Error{index, error}` names its zero-based index
  within this call and the
  original scalar cause. Mapping precedes committing within the same row.
  An earlier commit error outranks a later speculative map error.
* `transform` writes only `[0,i)` on failure. `for_each` preserves all successful
  earlier commits and stops before invoking later commits. A commit callback
  owns the side effects it performs in its failing row; check before writing
  when that row needs atomicity. There is no whole-array transaction.
* Maps must be free of externally observable side effects, and each row's
  result must be independent of earlier commits. Mapping can run ahead by one
  block. In particular,
  mapping must not look up a running balance that earlier commits change.
  Any additional map preconditions must hold even for rows mapped after an
  earlier failure. Checked fixedwide operations safely return their errors.
* `transform` permits exact same-type in-place aliasing with any input span.
  Shifted/partial input-output overlap is forbidden. Input spans may alias each
  other. A `for_each` commit for row `i` may update input element `i` after
  mapping it, provided the updated storage cannot be read by another row.
  This supports a maintenance pass that maps the old row value and commits its
  replacement alongside a shared balance update. Shared balances are read in
  commit. Output and all external state remain owned by the calling thread.
* `transform`, `for_each` and `transform_reduce` accept an optional first
  template argument for block size: `for_each<1>(...)` disables mapping ahead;
  the default is 1 and the allowed range is 1 through 64. `for_each<4>(...)`
  requests four-row blocking; measure the complete kernel before selecting it.
  There is one pass and at most `BlockSize * sizeof(expected<T, ArithmeticError>)` of lane-result
  scratch, plus bounded local arithmetic and plan state. No retained per-row
  statuses or allocation are required. Compiler optimization may remove the
  scratch entirely. Larger blocks are not necessarily faster.
* Library arithmetic is nonthrowing. Generic transforms are conditionally
  `noexcept` according to their callables. A user callback that throws has
  ordinary C++ exception behavior; it is not converted into an arithmetic
  error. Use nonthrowing callbacks for checked numerical kernels.
  The scheduler owns no heap allocation; caller-supplied callbacks and value
  operations retain responsibility for their own allocation behavior.

## Native multiplication algorithm and fallback scope

The scalar `mul_to` and batch multiplier first attempt a division-free exact
shortcut for a 128-bit operand and an operand of at most 64 bits. With
`d = 10^k`, `k` in 1 through 19, they recover a signed64 candidate using the
inverse of `5^k` modulo `2^64`, then check `candidate*d == original_raw` in
signed128. This proves both divisibility and coefficient range. Multiplying
two signed64 values then fits signed128, followed by the destination's range
check. A failed certificate falls through to the complete arithmetic path.
Rounded operations and constant evaluation keep their existing dispatch.

For signed input magnitudes `a`, `b` and a decimal rescale divisor `d`, write
`a = q*d + r`. Then the exact magnitude is

```
floor(a*b/d) = q*b + floor(r*b/d),   remainder = (r*b) % d.
```

When `b` and `d` fit unsigned64, the remainder product fits unsigned128.
The quotient product is checked with unsigned limbs. Its overflow cannot be
rescued by cancellation because every term is nonnegative. The final range
check precedes exact-mode refusal, and rounding uses the exact remainder and
quotient parity. Unsigned magnitudes handle both signed minima safely.

For non-exact rounding, if both actual magnitudes fit unsigned64, their product
fits unsigned128 even when it exceeds signed128. This case forms the product
directly, divides once, and applies the same final range and rounding checks.
It avoids the split path's second division for ordinary rounded price and FX
products. Exact mode retains the split because scale-certified slopes commonly
have a zero remainder after the first division.

For a broadcast, cancel `gcd(|factor.raw()|, 10^factor_digits)` once and prepare
the remaining divisor. A one-limb numerator uses a multiply-high reciprocal
with one exact correction; a wider numerator retains exact limb division.
Certified two-decimal `Fixed128<12>` slopes have no remainder when divided by
`10^10`; the exact delta case consequently avoids a wide pre-rescale product.
No caller certification is trusted by the API: exactness is checked either by
reconstruction or by the exact remainder.

Prepared cancellation applies when both input types and the destination have at
most 128 bits, the varying input's scale equals the destination scale, and the
broadcast's scale is at most 19. Its reduced factor must fit unsigned64. Other
combinations use the binary multiplier's checked dispatch and scalar fallback.

| Workload | Coverage and optimized scope |
|---|---|
| `128/12 * 64/10 -> 128/12` | Full scalar domain; certified exact coefficient, split or direct unsigned product; optional AVX2 exact broadcast tiles |
| `128/12 * 128/12 -> 128/12` | Native when one actual magnitude fits unsigned64; broadcast cancellation can make its factor fit |
| Other products with both inputs and destination at most 128 bits | Native for nonnegative scale gap up to 19 and an actual unsigned64 operand magnitude |
| Wide products, scale expansion, larger gaps | Existing checked scalar multiplication, exposed through the same batch interface |
| Add/sub, wide replacement, casts | Existing scalar arithmetic and all its cancellation/range rules; independent rows can be blocked |
| Division and mixed-width preparation chains | Existing scalar kernels; compose them in a map without changing intermediate boundaries |
| Ordered sums, scatter updates, position reductions | Independent mapping plus strictly ordered checks/commits; no reassociation |
| Comparisons, select and floating conversions | Generic transform of the unchanged scalar operations |
| `FIXEDWIDE_FORCE_PORTABLE` | Same API and results, existing portable scalar arithmetic; no native reciprocal/assembly path |

General full-width SIMD multiplication/division, specialized segmented kernels,
256-bit batch division and new floating conversion algorithms are deferred.
Performance is workload- and compiler-dependent.
See [the executable example](../examples/11_batch_soa.cpp) and
[benchmark reproduction instructions](benchmarks.md#soa-batch-benchmark).
