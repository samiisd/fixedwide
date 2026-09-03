# Architecture & Numerical Design

`fixedwide` is an exact, high-performance fixed-point arithmetic library for C++23.

## Type Hierarchy

The core template is:
```cpp
template<std::size_t Bits, unsigned Decimals>
class basic_fixed;
```
Supported bit widths:
- 8-bit: `Fixed8<D>`
- 16-bit: `Fixed16<D>`
- 32-bit: `Fixed32<D>`
- 64-bit: `Fixed64<D>`
- 128-bit: `Fixed128<D>`
- 256-bit: `Fixed256<D>`

## Storage Layout
- Widths <= 64 bits use native standard integer types (`int8_t`, `int16_t`, `int32_t`, `int64_t`).
- Width 128 uses `wide::int128` (two `uint64_t` limbs, little-endian, matching `__int128`).
- Width 256 uses `wide::int256` (four `uint64_t` limbs, little-endian).

## Arithmetic Pipeline
1. **Same-domain Arithmetic**:
   - `add`, `sub`, `mul`, `div`, `remainder` operate on identical types.
   - For width 64: `mul` and `div` use single-cycle x86-64 `imulq` and `idivq` instructions with exact remainder analysis.
   - For width 128: When scale fits in 64 bits (<= 19 decimal places), specialized assembly routines bypass multi-precision division, issuing dual `divq` instructions.
   - For generic widths: Knuth Algorithm D (`divmod_knuth`) provides arbitrary multi-limb division.
2. **Cross-scale Mixed Arithmetic**:
   - Explicit destination type required via `mul_to<Dest>(a, b, rounding)`, `div_to<Dest>(a, b, rounding)`, `add_to<Dest>(a, b)`, `sub_to<Dest>(a, b)`.
   - Ambiguous non-destination mixed operators are deleted at compile time.
