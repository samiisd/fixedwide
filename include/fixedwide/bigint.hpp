#pragma once
#include <fixedwide/wide.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <expected>

namespace fixedwide {
struct UnsignedDivision { u256 quotient; u128 remainder; };
struct SignedDivision { i256 quotient; i128 remainder; };

[[nodiscard]] std::expected<UnsignedDivision, ArithmeticError> divmod(u256 numerator, u128 divisor) noexcept;
[[nodiscard]] std::expected<SignedDivision, ArithmeticError> divmod(i256 numerator, i128 divisor) noexcept;
[[nodiscard]] std::expected<i128, ArithmeticError> divide_to_i128(i256 numerator, i128 divisor, Rounding rounding) noexcept;
[[nodiscard]] std::expected<i128, ArithmeticError> mul_div(i128 a, i128 b, i128 divisor, Rounding rounding) noexcept;
} // namespace fixedwide
