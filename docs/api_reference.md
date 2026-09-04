# API reference

Everything is in namespace `fixedwide`. Every public declaration also carries a
`///` doc comment, so an editor with `clangd` shows this on hover; a top-level
build writes `build/compile_commands.json`, which is where `clangd` looks.

| Header | What it gives you |
|---|---|
| `<fixedwide/fixed.hpp>` | `basic_fixed<Bits, Decimals>` and the `FixedN<D>` aliases |
| `<fixedwide/arithmetic.hpp>` | Checked arithmetic on two operands of the same type |
| `<fixedwide/mixed.hpp>` | Cross-width, cross-scale operations and comparison |
| `<fixedwide/chars.hpp>` | `parse`, `to_chars`, `from_chars`, `FormatOptions` |
| `<fixedwide/string.hpp>` | `to_string` — the one function here that allocates |
| `<fixedwide/binary.hpp>` | `to_bytes` / `from_bytes` with an explicit byte order |
| `<fixedwide/floating.hpp>` | Explicit conversion to and from `float` / `double` |
| `<fixedwide/format.hpp>` | `std::formatter`, so `std::format("{}", v)` works |
| `<fixedwide/iostream.hpp>` | `operator<<` / `operator>>` |
| `<fixedwide/hash.hpp>` | `std::hash`, for the unordered containers |
| `<fixedwide/wide.hpp>` | `wide::int128`, `uint128`, `int256`, `uint256` |
| `<fixedwide/all.hpp>` | All of the above |

Include only what you use: `<fixedwide/iostream.hpp>` pulls in `<iostream>`,
which is one of the heaviest headers in the standard library.

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

Two types with different `Bits` or different `Decimals` are different types. A
price and a quantity cannot be added by accident; the compiler says so.

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

## Rounding modes

`Rounding` is taken by every operation that can lose information. There is no
hidden default inside the library: arithmetic defaults to `nearest_even`,
parsing and `fixed_cast` default to `exact`.

| Mode | 0.5 → | −0.5 → | Use |
|---|---|---|---|
| `toward_zero` | 0 | 0 | Truncation; the machine's own division |
| `floor` | 0 | −1 | Directed, toward −∞ |
| `ceil` | 1 | 0 | Directed, toward +∞ |
| `nearest_even` | 0 | 0 | Banker's rounding. Does not drift over a sum, so it is the arithmetic default |
| `nearest_away` | 1 | −1 | Commercial rounding |
| `exact` | error | error | Refuses to round: `ArithmeticError::inexact`, or `ParseError::too_precise` |

> See [`examples/02_rounding_modes.cpp`](../examples/02_rounding_modes.cpp).

---

## Arithmetic functions

All in `<fixedwide/arithmetic.hpp>`, all `constexpr`, all taking two operands of
the **same** type and returning `std::expected<T, ArithmeticError>`.

| Function | Notes |
|---|---|
| `add(a, b)` / `sub(a, b)` | Exact; the only failure is overflow |
| `negate(a)` / `abs(a)` | Fail only for `min()` |
| `mul(a, b, rounding = nearest_even)` | Product formed at twice the width, then rescaled once |
| `div(a, b, rounding = nearest_even)` | Quotient carries every digit the type can hold |
| `mul_div(a, b, c, rounding = nearest_even)` | `a*b/c` with **one** rounding, not two |
| `remainder(a, b)` | Exact, takes the sign of `a`, so no rounding mode |
| `quantize(a, decimals, rounding)` | Same type, coarser grid: `quantize(1.2345, 2)` is `1.2300` |
| `from_integer<Target>(n)` | An ordinary integer to a fixed-point value |

`add`, `sub`, `mul`, `div` and `mul_div` are **deleted** for operands of
different types. Calling one is a compile error naming the deleted overload and
pointing at `mul_to<Dest>` and friends — never a silent conversion.

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

Nothing throws, nothing sets `errno`, and no operation returns a wrong answer in
place of an error. Every failure is a `std::expected` you have to look at.

`ArithmeticError`: `overflow`, `division_by_zero`, `inexact`,
`invalid_precision`, `invalid_value`.

`ParseError`: `empty`, `invalid`, `too_precise`, `overflow`.

`FormatError`: `buffer_too_small`, `invalid_precision`, `inexact`.

`BinaryError`: `wrong_size`, `invalid_encoding`.

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

| Function | |
|---|---|
| `parse<T>(text, rounding = exact)` | `std::string_view` in, `std::expected<T, ParseError>` out |
| `from_chars<T>(first, last, rounding)` | Pointer-range form |
| `to_chars(buffer, capacity, v, options = {})` | Writes into your buffer; never allocates |
| `to_string(v, options = {})` | Convenient, and the one thing here that allocates |
| `text_capacity` | `char[text_capacity]` is always enough for any `basic_fixed` |

`FormatOptions`: `digits` (decimals to print; `0` means *all of them* unless
`explicit_digits` is set), `trim_trailing_zeros`, `rounding`, `explicit_digits`.

`std::format("{}", v)` works after including `<fixedwide/format.hpp>`, and
inherits every width, fill and alignment spec from
`std::formatter<std::string_view>`.

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
cannot fail.

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
   name for text-to-value is enough, and it already takes a `string_view`. And
   `from_integer` has no inverse, because going back has to choose a rounding:
   that is `quantize(v, 0)` or `fixed_cast<FixedN<0>>`.
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
