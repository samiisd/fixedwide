#pragma once
#include <fixedwide/wide_integer.hpp>
#include <fixedwide/error.hpp>
#include <expected>

namespace fixedwide {
struct UnsignedDivision { u256 quotient; u128 remainder; };
struct SignedDivision { i256 quotient; i128 remainder; };
// Full quotient; signed remainder has the numerator's sign (truncation toward zero).
[[nodiscard]] std::expected<UnsignedDivision, ArithmeticError> divmod(u256 numerator, u128 divisor) noexcept;
[[nodiscard]] std::expected<SignedDivision, ArithmeticError> divmod(i256 numerator, i128 divisor) noexcept;
// Narrowing is checked BEFORE and AFTER rounding; never silently discards quotient bits.
[[nodiscard]] std::expected<i128, ArithmeticError> divide_to_i128(i256, i128, Rounding) noexcept;
[[nodiscard]] std::expected<i128, ArithmeticError> mul_div(i128, i128, i128, Rounding) noexcept;
} // namespace fixedwide
