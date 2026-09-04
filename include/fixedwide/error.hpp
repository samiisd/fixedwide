#pragma once
#include <cstdint>

namespace fixedwide {

// Every fallible operation in this library returns std::expected<T, E> with one
// of the enums below. Nothing throws, nothing sets errno, and no operation
// returns a wrong answer in place of an error.

/// Why a checked arithmetic operation could not produce a value.
enum class ArithmeticError : std::uint8_t {
    /// The exact result does not fit the destination type.
    overflow,
    /// The divisor was zero.
    division_by_zero,
    /// `Rounding::exact` was asked for and the result had a remainder.
    inexact,
    /// More decimals were requested than the type carries, as in
    /// `quantize(value, decimals)` with `decimals > D`.
    invalid_precision,
    /// The input was not a number this type can represent: NaN or infinity in
    /// `from_float`, for instance.
    invalid_value,
};

// Precedence, when more than one of these could apply.
//
// A result that does not fit the destination is `overflow`, even under
// `Rounding::exact` and even when it is also inexact. Reporting `inexact` there
// would invite the caller to retry with a rounding mode, and that retry cannot
// succeed -- no rounding makes an out-of-range value representable. The order
// is:
//
//   1. division_by_zero   the divisor was zero; nothing else is evaluated
//   2. invalid_value      the input was not a number (NaN, infinity)
//   3. invalid_precision  the caller asked for more decimals than the type has
//   4. overflow           the exact result is outside the destination's range
//   5. inexact            the result fits, but `Rounding::exact` was asked for
//                         and there was a remainder
//   6. overflow           rounding pushed an in-range value out of range
//
// tests/audit_mixed.cpp asserts this exactly -- no substitution allowed --
// against Boost.Multiprecision over both the native and the portable backend.

// `inexact` and `too_precise` are not two names for one thing, and neither is
// `too_precise` a spelling of `invalid_precision` below:
//
//   invalid_precision  the CALLER asked for more decimals than the type has
//   too_precise        the DATA carries more decimals than the type can hold
//   inexact            an exact operation had a remainder
//
// ParseError has no `inexact`: text that does not land on the type's decimal
// grid is `too_precise`, and there is no other way for a parse to be inexact.
/// Why text could not be turned into a value.
enum class ParseError : std::uint8_t {
    /// The input was empty, or the pointer range was empty or null.
    empty,
    /// The input is not a decimal number this parser accepts.
    invalid,
    /// The text carries more decimals than the type can hold, and
    /// `Rounding::exact` was in force.
    too_precise,
    /// The value is outside the type's range.
    overflow,
};

/// Why a value could not be written as text.
enum class FormatError : std::uint8_t {
    /// The output buffer cannot hold the result. `text_capacity` is always
    /// enough for any `basic_fixed`.
    buffer_too_small,
    /// `FormatOptions::digits` asked for more decimals than the type carries.
    invalid_precision,
    /// `FormatOptions::rounding` was `Rounding::exact` and printing the
    /// requested number of decimals would have dropped a nonzero remainder.
    inexact,
};

/// Why a byte sequence could not be decoded.
enum class BinaryError : std::uint8_t {
    /// The span is not exactly the type's byte width.
    wrong_size,
    /// The bytes do not encode a value of this type.
    invalid_encoding,
};

} // namespace fixedwide
