#pragma once

/// \file
/// Wide-integer division on raw `wide::` values, with no scale attached. What
/// the fixed-point kernels use, exposed because it is occasionally what a
/// caller wants directly.

#include <fixedwide/wide.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <expected>

namespace fixedwide {
/// Quotient and remainder of an unsigned wide division, both exact.
struct UnsignedDivision { u256 quotient; u128 remainder; };
/// Quotient and remainder of a signed wide division. The remainder takes the
/// sign of the numerator, as `%` does.
struct SignedDivision { i256 quotient; i128 remainder; };

/// Exact 256-by-128 division: quotient and remainder in one pass, no rounding.
/// \return both halves, or `ArithmeticError::division_by_zero`.
[[nodiscard]] std::expected<UnsignedDivision, ArithmeticError> divmod(u256 numerator, u128 divisor) noexcept;
/// Exact 256-by-128 signed division: quotient and remainder in one pass, no
/// rounding. The remainder takes the sign of the numerator.
/// \return both halves, or `ArithmeticError::division_by_zero`, or `overflow`
///         for `i256_min / -1`, which has no representable quotient.
[[nodiscard]] std::expected<SignedDivision, ArithmeticError> divmod(i256 numerator, i128 divisor) noexcept;
/// Divide a 256-bit numerator by a 128-bit divisor and round the quotient into
/// 128 bits.
/// \return the quotient, or `ArithmeticError::division_by_zero` / `overflow`
///         when it does not fit 128 bits / `inexact`.
[[nodiscard]] std::expected<i128, ArithmeticError> divide_to_i128(i256 numerator, i128 divisor, Rounding rounding) noexcept;
/// `a * b / divisor` on raw wide integers with one rounding: the product is
/// formed at 256 bits and divided once. The unscaled counterpart of
/// `fixedwide::mul_div`.
/// \return the result, or `ArithmeticError::division_by_zero` / `overflow`
///         / `inexact`.
[[nodiscard]] std::expected<i128, ArithmeticError> mul_div(i128 a, i128 b, i128 divisor, Rounding rounding) noexcept;
} // namespace fixedwide
