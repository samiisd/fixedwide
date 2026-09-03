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
from_float_kernel(double value, unsigned decimals, Rounding rounding, std::size_t bits) noexcept;

double to_float_kernel(wide::int256 raw, unsigned decimals) noexcept;

} // namespace detail

template<typename Target, std::floating_point Float>
[[nodiscard]] inline std::expected<Target, ArithmeticError>
from_float(Float value, Rounding rounding = Rounding::nearest_even) noexcept {
    if (std::isnan(value) || std::isinf(value)) return std::unexpected(ArithmeticError::invalid_value);
    auto res = detail::from_float_kernel(static_cast<double>(value), Target::fractional_digits, rounding, Target::bits);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<Target>(*res);
}

template<std::floating_point Float, std::size_t Bits, unsigned D>
[[nodiscard]] inline Float
to_float(basic_fixed<Bits, D> value) noexcept {
    double d = detail::to_float_kernel(detail::to_int256_raw(value.raw()), D);
    return static_cast<Float>(d);
}

} // namespace fixedwide
