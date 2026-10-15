# API reference

Everything is in namespace `fixedwide`, with caller-responsibility named functions
in `fixedwide::unchecked`. Public headers carry `///` doc comments for editor
hover help; a top-level build writes `build/compile_commands.json`, which is
where `clangd` looks.

| Header | What it gives you |
|---|---|
| `<fixedwide/fixed.hpp>` | `basic_fixed<Bits, Decimals>` and the `FixedN<D>` aliases |
| `<fixedwide/arithmetic.hpp>` | Checked same-type arithmetic and the infallible midpoint overload |
| `<fixedwide/mixed.hpp>` | Cross-width, cross-scale operations and comparison |
| `<fixedwide/checked.hpp>` | `Checked`: accumulated checked arithmetic with a sticky first error |
| `<fixedwide/bigint.hpp>` | Checked unscaled wide-integer division and multiply/divide |
| `<fixedwide/unchecked.hpp>` | Ordinary arithmetic operators and value-returning caller-responsibility functions |
| `<fixedwide/chars.hpp>` | `parse`, `to_chars`, `from_chars`, `FormatOptions` |
| `<fixedwide/literal.hpp>` | `literal<T>("...")` — exact decimal constants checked during compilation |
| `<fixedwide/string.hpp>` | `to_string` — the one function here that allocates |
| `<fixedwide/binary.hpp>` | `to_bytes` / `from_bytes` with an explicit byte order |
| `<fixedwide/floating.hpp>` | Explicit conversion to and from `float` / `double` |
| `<fixedwide/format.hpp>` | `std::formatter`, so `std::format("{}", v)` works |
| `<fixedwide/iostream.hpp>` | `operator<<` / `operator>>` |
| `<fixedwide/hash.hpp>` | `std::hash`, for the unordered containers |
| `<fixedwide/wide.hpp>` | `wide::int128`, `uint128`, `int256`, `uint256` |
| `<fixedwide/all.hpp>` | Everything above **except** the three standard-library adapters |

`all.hpp` deliberately omits `format.hpp`, `iostream.hpp` and `hash.hpp`.
Historical measurements on clang 22 at `-O2`, before `literal.hpp` and
`unchecked.hpp` were added:

| translation unit | |
|---|---:|
| `#include <fixedwide/all.hpp>` | **187 ms** |
| the same, with the three adapters back | 558 ms |
| `#include <fixedwide/arithmetic.hpp>` alone | 42 ms |

In those measurements, `<format>` cost 435 ms on its own and `<iostream>`
450 ms. They are adapters to standard facilities rather than part of the
numeric API, so you ask for them:

```cpp
#include <fixedwide/all.hpp>
#include <fixedwide/iostream.hpp>   // only in the file that actually streams
```

---

## Primary types

```cpp
template<std::size_t Bits, unsigned Decimals> struct basic_fixed;
```

An integer scaled by `10^Decimals`. Nothing else: no exponent, no allocation, no
vtable. `sizeof(Fixed64<2>) == 8`, and the object representation is the scaled
integer.

| Alias | Storage | Width | Decimals | Size | Align |
|---|---|---:|---|---:|---:|
| `Fixed8<D>` | `std::int8_t` | 8 | 0–2 | 1 | 1 |
| `Fixed16<D>` | `std::int16_t` | 16 | 0–4 | 2 | 2 |
| `Fixed32<D>` | `std::int32_t` | 32 | 0–9 | 4 | 4 |
| `Fixed64<D>` | `std::int64_t` | 64 | 0–18 | 8 | 8 |
| `Fixed128<D>` | `wide::int128` | 128 | 0–38 | 16 | 8 |
| `Fixed256<D>` | `wide::int256` | 256 | 0–76 | 32 | 8 |

Two types with different `Bits` or different `Decimals` are distinct types.
Values carrying different scales cannot be combined implicitly without
specifying the destination scale (e.g. via `mul_to` or `fixed_cast`).
Note that `basic_fixed` enforces scale and bit-width distinctness, not
semantic unit typing: values with identical width and scale are the same type.

Members:

| | |
|---|---|
| `Fixed::bits`, `Fixed::fractional_digits` | the template parameters |
| `Fixed::scale()` | `10^fractional_digits`, as `raw_type` |
| `Fixed::min()`, `Fixed::max()` | the representable range |
| `Fixed::from_raw(r)` / `v.raw()` | wrap and unwrap the scaled integer, unchanged |

> See [`examples/01_quick_start.cpp`](../examples/01_quick_start.cpp) and
> [`examples/07_money_ledger.cpp`](../examples/07_money_ledger.cpp).

---

<a id="checked-or-unchecked"></a>
## Checked, accumulated checked, or unchecked

All three policies use the same fixed-point storage and numerical rounding rules.
Choose error handling at the call site, not in the stored value's type.

| Policy | Spelling | Failure handling |
|---|---|---|
| Checked | `fixedwide::add(a, b)` | `std::expected` per call; the caller may stop immediately. |
| Accumulated checked (0.6.8) | `c.add(a, b)` with a local `Checked c` | Every call is checked; the first error sticks; validate once before publication. |
| Unchecked (0.6.4) | `a + b` or `unchecked::add(a, b)` | Caller-proved bounds and preconditions; violations are undefined behaviour. |

No per-value storage wrapper or global runtime policy flag is added. `Checked`
is an explicit local computation context with its own error status, not a
storage wrapper. It does not change the behaviour of operators or free functions.
See [unchecked.md](unchecked.md) for the unchecked contract.

## Accumulated checks

Include `<fixedwide/checked.hpp>` or `<fixedwide/all.hpp>`:

```cpp
using Money = fixedwide::Fixed128<10>;
using Percent = fixedwide::Fixed64<6>;
using Integer = fixedwide::Fixed64<0>;

// Inside a function returning std::expected<Percent, ArithmeticError>:
fixedwide::Checked c;
const auto base = c.add(balance, credit);
const auto equity = c.add(base, pnl);
const auto available = c.sub(equity, blocked);
const auto ratio = c.mul_div_to<Percent>(available, Integer::from_raw(100), used_margin);
return c.result(ratio);
```

All members below have the same operands, template destination, defaults and
rounding as their checked free functions. They return the successful value
rather than an expected. There is no new arithmetic kernel.

| Members | Default rounding | Constexpr |
|---|---|---|
| `add(a,b)`, `sub(a,b)`, `negate(a)`, `abs(a)`, `remainder(a,b)` | None | Yes |
| `mul(a,b,r)`, `div(a,b,r)`, `mul_div(a,b,c,r)` (fixed point) | `nearest_even` | Yes |
| `quantize(a,decimals,r)` | `nearest_even` | Yes |
| `midpoint(a,b,r)`, `midpoint(a,b,decimals,r)` | `nearest_even` | Yes |
| `from_integer<Target>(integer)`, `narrow(FP128)` | None | Yes |
| `add_to<Dest>`, `sub_to<Dest>`, `mul_to<Dest>`, `div_to<Dest>`, `mul_div_to<Dest>` | `nearest_even` | No, like the free functions |
| `fixed_cast<Dest>(a,r)` | `exact` | No |
| `from_float<Target>(x,r)`, `from_double<Target>(x,r)` | `nearest_even` | No |
| `from_double64(x,r)`, `from_double128(x,r)`, `mul_wide(FP64,FP64,r)` | `nearest_even` | No |
| `divmod(u256,u128)`, `divmod(i256,i128)` | None | No |
| `divide_to_i128(i256,i128,r)`, `mul_div(i128,i128,i128,r)` | Required, no default | No |

The current API has one checked `quantize` overload and two checked `midpoint`
overloads. Its infallible `midpoint<Rounding>(a,b)` spelling stays a free
function. All six public storage widths (8/16/32/64/128/256) and every valid
scale remain supported; internal 512/1024-bit mixed intermediates are unchanged.
There is no public fixed-point storage type wider than 256 bits in this release.
The [complete operation/error inventory](../reports/CHECKED_0_6_8.md#operation-inventory)
includes the boundaries deliberately outside this context.

### Status and generic composition

| Member | Meaning |
|---|---|
| `Checked{}` | Initially successful; two bytes in this implementation, trivially copyable. |
| `explicit operator bool()` | True until the first failed operation. |
| `error()` | `std::optional<ArithmeticError>`: empty on success, first error otherwise. |
| `take(std::expected<T,ArithmeticError>)` | Unwrap a successful value, or record the error and return `T{}`. |
| `take(std::expected<void,ArithmeticError>)` | Absorb status without a value. |
| `result(value)` | Return `std::expected<remove_cvref_t<T>,ArithmeticError>`; never return a value after failure. |
| `result()` | Return the accumulated `std::expected<void,ArithmeticError>`. |

All these functions are `noexcept`, and constexpr when their value operations
permit constant evaluation. `take` copies const/mutable lvalue expecteds and
moves rvalues when that transfer is nonthrowing. Its value type must support
nonthrowing default construction and the chosen copy/move. This explicitly
rejects throwing value operations rather than silently terminating inside a
blanket `noexcept`. Move-only nonthrowing types are supported. `result` needs a
nonthrowing selected copy/move, but not default construction. Ordinary custom
functions returning `expected<T,ArithmeticError>` join through `take`; constructing
or evaluating their arguments remains the caller's responsibility.

### Failure, evaluation order, and publication

The first failed operation wins. A failed step returns `T{}` (zero for numeric
values, zero quotient/remainder for division records). Later successful calls
still return their own computed values; later failed calls return zero and
cannot replace the first error. There is no sticky-state short circuit. In
particular, a zero placeholder used as the next divisor reaches the existing
checked divide, never an unchecked divide.

**Intermediate values after any failure are unusable calculation results.**
They are ordinary values, not tagged poison. The library cannot stop the caller
from reading them, using an unchecked operator, storing them, or performing an
external side effect. Check `if (!c)` before such use, or publish through
`c.result(...)`. `result` refuses even a value computed *before* the failure and
does not move from its argument when returning an error. It does not roll back
side effects already performed by caller-defined functions.

Write fallible steps in separate statements. In `c.add(c.div(...),c.mul(...))`,
C++ does not specify which sibling argument runs first; the sticky error follows
the actual evaluation order, not left-to-right spelling. A context is local to
one chain/thread; it is not synchronized. Copies are independent snapshots.
Queries and `result` do not reset it; construct a new context for a new chain.

Parsing (`ParseError`), formatting (`FormatError`) and binary decoding
(`BinaryError`) stay separate boundary operations. Infallible outputs and
comparisons have no error to accumulate. Operators on a wrapped value type are
not introduced: destination types for `mul_to`/`mul_div_to` and explicit rounding
belong to each operation, not to an operand wrapper. Plain operators remain
unchecked; writing `c.result(a + b)` does **not** make that addition checked.

The context itself adds no allocation, exceptions, thread-local/global state or
runtime policy selection. Caller-defined value constructors/functions can still
allocate or perform side effects; `noexcept` alone does not forbid allocation. Eager continuation may do more work than an early-return chain
on failure. Happy-path cost is measured, not promised identical for every
compiler or expression; see the [benchmark evidence](benchmarks.md#accumulated-checked-chains).


## Compile-time constants

In `<fixedwide/literal.hpp>`, also included by `all.hpp`:

```cpp
#include <fixedwide/literal.hpp>

using Money = fixedwide::Fixed64<2>;
constexpr auto price = fixedwide::literal<Money>("19.99");
constexpr Money same_price = "19.99";
auto threshold = Money{"19.99"};
static_assert(price.raw() == 1999);
```

`literal<T>(text)` is a `consteval`, `noexcept` function returning `T` directly.
`T` must be one of the library's fixed-point types; all supported widths and
scales use the same API. The destination type supplies the scale. There is no
UDL suffix, conversion wrapper, inferred scale or rounding argument.

The implicit character-array constructor is also `consteval` and `noexcept`,
delegates to `literal<T>`, and has the same input contract and diagnostics. Its
definition requires `literal.hpp` or `all.hpp`; `fixed.hpp` stays lightweight.
Direct calls with a concrete destination type, such as `set_price("19.99")`,
convert during compilation. Use `Money{"19.99"}` in deduced templates and test
assertions, where a bare string would reach the conversion as a runtime parameter.
Pointers, strings, string views and runtime array contents are not accepted;
use `parse<T>` for runtime input. Numeric constructors remain unavailable.
The constructor is available since v0.6.3; v0.6.1 and v0.6.2 use `literal<T>` instead.

Clang 22 incorrectly emits an immediate-function call for a nondependent
temporary such as `Money{"1.25"}` inside a generic lambda. In that context,
use a typed initializer (`Money value = "1.25"`) or pass the string directly
to a function taking `Money`. Both keep conversion at compile time; no runtime
parsing fallback is provided.

| Expression | Result |
|---|---|
| `literal<Money>("1.25")` | exactly 1.25 |
| `literal<Money>("+001.2500")` | exactly 1.25; extra zero digits are harmless |
| `literal<Money>("125e-2")` | exactly 1.25 |
| `literal<Money>("-0.00")` | zero |
| `literal<Fixed8<2>>("-1.28")` | the signed minimum, accepted |
| `literal<Money>("1.28")` | exactly 1.28 |
| `literal<Money>("1.251")` | compile error: not exactly representable |
| `literal<Fixed8<2>>("1.28")` | compile error: overflow |
| `literal<Money>("1.25xyz")` | compile error: invalid text |

The input is a string literal or a null-terminated character array whose
contents can be evaluated at compile time. Conversion is immediate even when
the result initializes an ordinary `auto` variable; runtime input is rejected.
No compiled-library linkage is needed for literal-only consumers, and the API
works without exceptions. Decimal conversion does not pass through `float`,
`double` or `long double`.

The shared decimal grammar accepts an optional sign, a mantissa with at least
one digit and at most one decimal point, and an optional signed decimal
exponent. `.5` and `1.` are accepted. The entire input is validated: whitespace,
digit separators, hexadecimal notation, special floating values, embedded NULs
and trailing junk are rejected. The maximum is 4096 characters, excluding the
terminating NUL. A nonterminated array is rejected rather than losing its last
character. Diagnostics identify empty, invalid, inexact or overflowing input;
the compiler controls their precise wording.

Use `parse<T>(text, rounding)` for runtime text and its `std::expected<T,
ParseError>` result. Literals are always exact; subsequent arithmetic can be
checked or unchecked without changing its numerical rounding policy. Use
`T::from_raw` only when a value is already a scaled storage integer, not as an
alternative spelling for a human-readable decimal.

> See [`examples/08_constexpr.cpp`](../examples/08_constexpr.cpp) for constants
> composed with checked compile-time arithmetic.

---

## Rounding modes

Named operations that can lose information accept `Rounding`: arithmetic defaults
to `nearest_even`, parsing and `fixed_cast` default to `exact`. Operators `*` and
`/` use nearest-even. Literals never round. The error results below describe the
checked APIs; exactness is a precondition in the unchecked API.

| Mode | 0.5 → | −0.5 → | Use |
|---|---|---|---|
| `toward_zero` | 0 | 0 | Truncation; the machine's own division |
| `floor` | 0 | −1 | Directed, toward −∞ |
| `ceil` | 1 | 0 | Directed, toward +∞ |
| `nearest_even` | 0 | 0 | Banker's rounding; reduces systematic tie-breaking bias |
| `nearest_away` | 1 | −1 | Commercial rounding |
| `exact` | error | error | Refuses to round: `ArithmeticError::inexact`, or `ParseError::too_precise` |

> See [`examples/02_rounding_modes.cpp`](../examples/02_rounding_modes.cpp).

---

## Arithmetic functions

In `<fixedwide/arithmetic.hpp>`, `constexpr` functions on operands of the **same**
type. They return `std::expected<T, ArithmeticError>`, except for the infallible
compile-time-policy midpoint overload noted below. Unchecked counterparts are
in `unchecked.hpp`, not a change to these functions' return types or errors.

| Function | Notes |
|---|---|
| `add(a, b)` / `sub(a, b)` | Exact; the only failure is overflow |
| `negate(a)` / `abs(a)` | Fail only for `min()` |
| `midpoint(a, b, rounding = nearest_even)` | `(a+b)/2`, rounded once; no intermediate overflow, at any width |
| `midpoint<rounding>(a, b)` | Returns `T` directly; compile-time non-exact policy guarantees no failure |
| `midpoint(a, b, decimals, rounding = nearest_even)` | Rounds once onto the requested decimal grid, without a wider intermediate; coarser grids can overflow at storage limits |
| `mul(a, b, rounding = nearest_even)` | Product formed at twice the width, then rescaled once |
| `div(a, b, rounding = nearest_even)` | Quotient carries every digit the type can hold |
| `mul_div(a, b, c, rounding = nearest_even)` | `a*b/c` with **one** rounding, not two |
| `remainder(a, b)` | Exact, takes the sign of `a`, so no rounding mode |
| `quantize(a, decimals, rounding)` | Same type, coarser grid: `quantize(1.2345, 2)` is `1.2300` |
| `from_integer<Target>(n)` | An ordinary integer to a fixed-point value |

`add`, `sub`, `mul`, `div` and `mul_div` are **deleted** for operands of
different types. Calling one is a compile error naming the deleted overload and
pointing at `mul_to<Dest>` and friends — never a silent conversion.

`midpoint(a, b, rounding)` takes two values of the **same** width and scale, and
returns `std::expected<T, ArithmeticError>`. It is symmetric in its operands,
including half-unit ties. All six rounding modes apply at the raw integer's last
decimal place; `exact` rejects an odd raw sum with `inexact`. It cannot overflow,
including `midpoint(T::min(), T::min())` and `midpoint(T::max(), T::max())`.
Unlike integer `std::midpoint`, tie-breaking does not depend on operand order.

The precision overload avoids double rounding: for example, averaging
`1.23449999` and `1.2345` directly to three decimals gives `1.234`, whereas
rounding first at eight decimals and then quantizing can incorrectly give `1.235`.
Its errors are `invalid_precision`, `inexact` and final-grid `overflow`.

For `Fixed64<8>`, the midpoint of `1.00000001` and `1.00000003` is
`1.00000002`, not the `1.00000001` obtained by truncating each half first.
See [midpoint validation and benchmarks](../reports/midpoint/README.md).

> See [`examples/08_constexpr.cpp`](../examples/08_constexpr.cpp).

---

## Mixed-scale operations

In `<fixedwide/mixed.hpp>`. Two different scales have no single obvious result
scale, so the destination is named. The exact rational result is formed at full
width and rounded **once**, directly into `Dest`.

| Function | |
|---|---|
| `mul_to<Dest>(a, b, rounding)` | |
| `div_to<Dest>(a, b, rounding)` | |
| `add_to<Dest>(a, b, rounding)` / `sub_to<Dest>(a, b, rounding)` | |
| `mul_div_to<Dest>(a, b, c, rounding)` | `a*b/c` across three types, one rounding |
| `fixed_cast<Dest>(v, rounding = exact)` | Convert; defaults to refusing to lose a digit |

`operator==` and `operator<=>` work across any two widths and scales, need no
destination, and are exact: both sides are lifted to a common exponent and the
integers compared. No division happens and nothing is rounded, so the answer is
never a rounding artefact. Both are `constexpr` for every width.

> See [`examples/04_mixed_scales.cpp`](../examples/04_mixed_scales.cpp), and
> [benchmarks.md](benchmarks.md#the-cost-of-a-mixed-operation) for what each of
> these costs and when the narrow path applies.

---

## Error types

Checked core arithmetic, runtime parsing, and serialization functions do not
throw exceptions, do not set `errno`, and communicate failures through
`std::expected`. Source constants built by `literal<T>()` instead fail during
compilation. Unchecked calls do not report errors: their preconditions must hold.

`ArithmeticError`: `overflow`, `division_by_zero`, `inexact`,
`invalid_precision`, `invalid_value`.

`ParseError`: `empty`, `invalid`, `too_precise`, `overflow`.

`FormatError`: `buffer_too_small`, `invalid_precision`, `inexact`.

`BinaryError`: `wrong_size` (returned when buffer size does not match representation size; `invalid_encoding` is reserved).

**Precedence.** When more than one could apply, a result that does not fit the
destination is `overflow` — even under `Rounding::exact`, and even when it is
also inexact. Reporting `inexact` there would invite a retry with a rounding
mode that cannot succeed. The full order is in `<fixedwide/error.hpp>`, and
`tests/audit_mixed.cpp` asserts it exactly, with no substitution allowed, over
both backends.

Three of these are easy to confuse, and they are three different things:

| | |
|---|---|
| `invalid_precision` | the **caller** asked for more decimals than the type has |
| `too_precise` | the **data** carries more decimals than the type can hold |
| `inexact` | an operation asked to be exact had a remainder |

> See [`examples/03_error_handling.cpp`](../examples/03_error_handling.cpp).

---

## Text conversion

In `<fixedwide/chars.hpp>`; `to_string` is in `<fixedwide/string.hpp>`.
For source constants, see [compile-time constants](#compile-time-constants).

| Function | |
|---|---|
| `parse<T>(text, rounding = exact)` | `std::string_view` in, `std::expected<T, ParseError>` out |
| `from_chars<T>(first, last, rounding)` | Pointer-range form |
| `to_chars(buffer, capacity, v, options = {})` | Writes into your buffer; never allocates |
| `to_string(v, options = {})` | Convenient, and the one thing here that allocates |
| `text_capacity` | `char[text_capacity]` is always enough for any `basic_fixed` |

`FormatOptions`: `digits` (decimals to print; `0` means *all of them* unless
`explicit_digits` is set), `trim_trailing_zeros`, `rounding`, `explicit_digits`.

`std::format("{}", v)` works after including `<fixedwide/format.hpp>`. The spec
is `[[fill]align][width][.precision][f]`, and **precision means decimals**:

| | |
|---|---|
| `{}` | every decimal the type carries |
| `{:.2}` / `{:.2f}` | two decimals, rounded nearest-even |
| `{:>12}` | right-aligned in a field of twelve (numbers right-align by default) |
| `{:*^14.2}` | fill, centre, width and decimals together |

A precision larger than the type's own decimals is a `std::format_error`, not a
padded lie. This formatter used to inherit `std::formatter<std::string_view>`,
which made `{:.2}` truncate the *string*: `123.4567` printed as `12`.

> See [`examples/05_text_io.cpp`](../examples/05_text_io.cpp).

---

## Binary serialization

In `<fixedwide/binary.hpp>`. The byte order is never guessed — you name it, so
what one machine writes another reads.

| Function | |
|---|---|
| `to_bytes<Endian>(v)` | `std::array<std::uint8_t, Bits/8>` |
| `from_bytes<Target, Endian>(span)` | `std::expected<Target, BinaryError>`; a wrong-length span is rejected |
| `load_unaligned<Target, Endian>(ptr)` / `store_unaligned<Endian>(ptr, v)` | Straight out of a packet buffer |

`Endian` is `endian::little` (the default) or `endian::big`.

> See [`examples/06_binary_roundtrip.cpp`](../examples/06_binary_roundtrip.cpp).

---

## Floating point

In `<fixedwide/floating.hpp>`. Both directions are explicit, because both can
lose information and the loss should be in the source.

`from_float<Target>(value, rounding = nearest_even)` returns
`std::expected<Target, ArithmeticError>` — NaN and infinity are
`invalid_value`. `to_float<Float>(v)` and `to_double(v)` go the other way and
cannot fail. `v.to_double()` is the equivalent explicit member spelling, also
defined in this header; including only `fixed.hpp` remains lightweight.

---

## Naming rules

Every public name obeys four rules:

1. **Types.** A name the standard library also has is spelled its way
   (`basic_fixed`, `wide::uint128`, `endian`); a name of this library's own is
   `PascalCase` (`Rounding`, `FormatOptions`, `ArithmeticError`, `Fixed64`).
2. **Functions, constants and enumerators** are always `snake_case`.
3. **Conversions come in `from_X` / `to_X` pairs**: `from_chars`/`to_chars`,
   `from_bytes`/`to_bytes`, `from_float`/`to_float`, `from_raw`/`raw`. Two names
   sit outside the pattern on purpose. `to_string`'s inverse is `parse` — one
   name for runtime text-to-value is enough, and it already takes a `string_view`.
   `from_integer` has no inverse, because going back has to choose a rounding:
   that is `quantize(v, 0)` or `fixed_cast<FixedN<0>>`. `literal` is the separate
   compile-time constant entry point, not another runtime conversion spelling.
4. **One word per concept.** The count of fractional digits is `decimals`
   everywhere. Two older spellings survive because callers write them:
   `basic_fixed::fractional_digits` (the member) and `FormatOptions::digits` (a
   designated initialiser). Both are marked in their headers as the same
   quantity. `scale` means `10^decimals` and nothing else.

---

## The 0.4 compatibility surface

A small set of names fixes the scale at 12 digits, because that is what 0.4's
API was: it had no scale parameter, only `FP64` and `FP128`. They exist so the
paired benchmark can compile 0.4's byte-identical source against this library,
which is what makes that comparison mean anything.

They are **not** the API this library is for. Nothing else in the library uses
them, and each is marked in its header with the generic replacement:

| 0.4 name | generic replacement |
|---|---|
| `FP64`, `FP128` | `Fixed64<D>`, `Fixed128<D>` |
| `fp64_min/max`, `fp128_min/max` | `basic_fixed<Bits, D>::min()` / `::max()` |
| `fractional_digits`, `scale` | `Fixed::fractional_digits`, `Fixed::scale()` |
| `mul_wide(a, b)` | `mul_to<Dest>(a, b)` |
| `narrow(v)` | `fixed_cast<Dest>(v)` |
| `parse64`, `parse128` | `parse<T>(text, rounding)` |
| `from_double64`, `from_double128` | `from_float<Target>(value, rounding)` |

Everything else in the public API is parameterised on width and scale.
