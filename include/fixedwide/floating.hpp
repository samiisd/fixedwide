#pragma once
#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <concepts>
#include <expected>
#include <cmath>

namespace fixedwide {

namespace detail {

std::expected<wide::int256, ArithmeticError>
from_float_kernel(float value, unsigned decimals, Rounding rounding, std::size_t bits) noexcept;

std::expected<wide::int256, ArithmeticError>
from_float_kernel(double value, unsigned decimals, Rounding rounding, std::size_t bits) noexcept;

std::expected<wide::int256, ArithmeticError>
from_float_kernel(long double value, unsigned decimals, Rounding rounding, std::size_t bits) noexcept;

float to_float_kernel(wide::int256 raw, unsigned decimals, float) noexcept;
double to_float_kernel(wide::int256 raw, unsigned decimals, double) noexcept;
long double to_float_kernel(wide::int256 raw, unsigned decimals, long double) noexcept;

} // namespace detail

template<typename Target, std::floating_point Float>
[[nodiscard]] inline std::expected<Target, ArithmeticError>
from_float(Float value, Rounding rounding = Rounding::nearest_even) noexcept {
    if (std::isnan(value) || std::isinf(value)) return std::unexpected(ArithmeticError::invalid_value);
    auto res = detail::from_float_kernel(value, Target::fractional_digits, rounding, Target::bits);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<Target>(*res);
}

template<std::floating_point Float, std::size_t Bits, unsigned D>
[[nodiscard]] inline Float
to_float(basic_fixed<Bits, D> value) noexcept {
    return detail::to_float_kernel(detail::to_int256_raw(value.raw()), D, Float{});
}

// 0.4 compatibility surface: fixed at 12 digits. Generic replacement:
// from_float<Target>(value, rounding). See fixed.hpp.
[[nodiscard]] inline std::expected<FP64, ArithmeticError> from_double64(double value, Rounding rounding = Rounding::nearest_even) noexcept {
    return from_float<FP64>(value, rounding);
}

[[nodiscard]] inline std::expected<FP128, ArithmeticError> from_double128(double value, Rounding rounding = Rounding::nearest_even) noexcept {
    return from_float<FP128>(value, rounding);
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] inline double to_double(basic_fixed<Bits, D> value) noexcept {
    return to_float<double>(value);
}

} // namespace fixedwide
