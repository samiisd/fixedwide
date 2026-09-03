# API Reference

## Primary Types
```cpp
#include <fixedwide/all.hpp>

using fixedwide::basic_fixed;
using fixedwide::Fixed8;
using fixedwide::Fixed16;
using fixedwide::Fixed32;
using fixedwide::Fixed64;
using fixedwide::Fixed128;
using fixedwide::Fixed256;
using fixedwide::Rounding;
using fixedwide::ArithmeticError;
```

## Rounding Modes
- `Rounding::toward_zero`: Truncates toward zero.
- `Rounding::floor`: Rounds toward negative infinity.
- `Rounding::ceil`: Rounds toward positive infinity.
- `Rounding::nearest_even`: Halfway ties rounded to nearest even digit (banker's rounding).
- `Rounding::nearest_away`: Halfway ties rounded away from zero.
- `Rounding::exact`: Fails with `ArithmeticError::inexact` if any non-zero remainder exists.

## Arithmetic Functions
- `add(a, b)` -> `std::expected<T, ArithmeticError>`
- `sub(a, b)` -> `std::expected<T, ArithmeticError>`
- `mul(a, b, rounding)` -> `std::expected<T, ArithmeticError>`
- `div(a, b, rounding)` -> `std::expected<T, ArithmeticError>`
- `remainder(a, b)` -> `std::expected<T, ArithmeticError>`
- `quantize(a, target_decimals, rounding)` -> `std::expected<T, ArithmeticError>`

## Mixed Precision
- `mul_to<Dest>(a, b, rounding)` -> `std::expected<Dest, ArithmeticError>`
- `div_to<Dest>(a, b, rounding)` -> `std::expected<Dest, ArithmeticError>`
- `add_to<Dest>(a, b)` -> `std::expected<Dest, ArithmeticError>`
- `sub_to<Dest>(a, b)` -> `std::expected<Dest, ArithmeticError>`

## Text I/O & Formatting
- `to_chars(buf, cap, val, options)` -> `std::expected<size_t, FormatError>`
- `from_chars(first, last, val, rounding)` -> `std::from_chars_result`
- `to_string(val, options)` -> `std::expected<std::string, FormatError>`
- `std::format("{}", val)`
- `operator<<(os, val)` / `operator>>(is, val)`
