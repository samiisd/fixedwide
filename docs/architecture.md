# Architecture

## The type

```cpp
template<std::size_t Bits, unsigned Decimals> struct basic_fixed;
```

A `basic_fixed` is an integer scaled by `10^Decimals` and nothing else. There is
no exponent field, no tag, no indirection, and no allocation. The scale lives in
the type, so it costs nothing at runtime and two different scales are two
different types.

| Width | Storage |
|---|---|
| 8, 16, 32, 64 | `std::int8_t` … `std::int64_t` |
| 128 | `wide::int128` — two `std::uint64_t` limbs, little-endian, laid out like `__int128` |
| 256 | `wide::int256` — four `std::uint64_t` limbs, little-endian |

The public storage types never use the compiler's `_BitInt`. Two Clang-only
inline fast paths use `_BitInt(256)` as a *local* computation type, where it has
no ABI surface; that is stated rather than denied.

## Backends

Three implementations of the same arithmetic, selected at compile time. All
three are differentially tested against each other and against
Boost.Multiprecision on every CI run, so they cannot drift apart.

| Backend | When | What it uses |
|---|---|---|
| x86-64 assembly | GCC or Clang on x86-64 | `mulq` / `divq` with exact remainder analysis; for 128-bit values whose scale fits 64 bits, paired `divq` instead of multi-precision division |
| `__int128` | Any compiler with `__SIZEOF_INT128__` whose 128-bit division links | The compiler's own 128-bit type |
| Portable multi-limb | Everything else, or `FIXEDWIDE_FORCE_PORTABLE=ON` | Knuth Algorithm D over `std::uint64_t` limbs; no extensions, no assembly |

The middle row is a link test, not a compiler name. 128-bit division is not an
instruction — the compiler emits a call to `__divti3` in its runtime library —
and clang-cl defines `__SIZEOF_INT128__` while targeting a CRT that has no such
symbol. `CMakeLists.txt` compiles and links a 128-bit division at configure time
and drops to the portable backend if it does not resolve. MSVC has no
`__int128` at all and lands there directly.

## Same-type arithmetic

`add`, `sub`, `mul`, `div`, `mul_div`, `remainder` and `quantize` take two (or
three) operands of the same type.

The invariant is **one rounding per operation**. `mul` forms the product at
twice the operand width before rescaling, so nothing is lost in between:
multiplying two `Fixed64<12>` goes through 128 bits and divides by `10^12`
exactly once. `mul_div` forms the full-width product and divides it once, rather
than rounding after the multiply and again after the divide.

Every one of them is `constexpr`, and every one returns
`std::expected<T, ArithmeticError>`. Overflow is detected before it happens, not
observed afterwards.

## Cross-scale arithmetic

Mixing widths or scales requires naming the destination — `mul_to<Dest>(a, b)` —
because two different scales have no single obvious result scale. The
same-type `add`/`sub`/`mul`/`div`/`mul_div` are *deleted* for mismatched
operands, so a mix is a compile error naming the deleted overload rather than a
silent conversion.

There are two implementations, and the difference between them matters:

- **`detail::mixed_native`** does the whole operation in one `__int128` when the
  aligned intermediate fits 126 bits.
- **The general kernel** in `src/mixed.cpp` works in 1024-bit limbs and handles
  everything else.

They differ by about 160x. Which one applies is decided at compile time by the
widths and scales; [benchmarks.md](benchmarks.md#the-mixed-path-cliff) gives the
rule and the numbers.

Comparison is the exception: `==` and `<=>` need no destination and cannot lose
anything, because both sides are lifted to a common exponent and the integers
compared — no division happens. There are two implementations here too, for a
different reason: the runtime one is compiled into the library, so a
constant-evaluation path lives in the header, and a differential sweep over
every scale pair and both signs keeps the two in agreement.

## Text

Parsing and formatting both work on the scaled integer directly and never
allocate. `to_chars` writes into a caller's buffer; `text_capacity` is a bound
that always fits, so it cannot return `buffer_too_small` when given
`char[text_capacity]`. The formatting kernel narrows to 64-bit arithmetic
whenever the quotient, remainder and divisor all fit, which is most of the time.

## What the library never does

No heap allocation, on any path. No virtual dispatch. No exceptions — it builds
and works under `-fno-exceptions` and `-fno-rtti`. No `errno`. No global state.
No silent wrong answer: every operation that can fail says so in its return type.
