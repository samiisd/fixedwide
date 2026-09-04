#pragma once

/// \file
/// Explicit conversions between fixed point and binary floating point. Every
/// one is spelled out, because this is the boundary where decimal exactness is
/// lost and it should be visible at the call site.

#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <concepts>
#include <expected>
#include <cmath>

namespace fixedwide {

namespace detail {

std::expected<wide::int256, ArithmeticError> from_float_kernel(float value, unsigned decimals, Rounding rounding,
                                                               std::size_t bits) noexcept;

std::expected<wide::int256, ArithmeticError> from_float_kernel(double value, unsigned decimals, Rounding rounding,
                                                               std::size_t bits) noexcept;

std::expected<wide::int256, ArithmeticError> from_float_kernel(long double value, unsigned decimals, Rounding rounding,
                                                               std::size_t bits) noexcept;

float to_float_kernel(wide::int256 raw, unsigned decimals, float) noexcept;
double to_float_kernel(wide::int256 raw, unsigned decimals, double) noexcept;
long double to_float_kernel(wide::int256 raw, unsigned decimals, long double) noexcept;

} // namespace detail

/// Convert a binary floating-point value to fixed point, checked.
///
/// The conversion is explicit because it is where decimal exactness is lost:
/// `0.1` as a `double` is not 0.1, and this reports what that actually rounds
/// to on the destination's grid rather than pretending.
///
/// \tparam Target the fixed-point type to produce.
/// \return the value, or `ArithmeticError::invalid_value` for NaN or infinity,
///         or `overflow` / `inexact`.
template<typename Target, std::floating_point Float>
[[nodiscard]] inline std::expected<Target, ArithmeticError>
from_float(Float value, Rounding rounding = Rounding::nearest_even) noexcept {
    if (std::isnan(value) || std::isinf(value)) return std::unexpected(ArithmeticError::invalid_value);
    auto res = detail::from_float_kernel(value, Target::fractional_digits, rounding, Target::bits);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<Target>(*res);
}

/// Convert fixed point to binary floating point.
///
/// Cannot fail, and is not exact: the result is the nearest `Float` to the
/// value. Explicit for the same reason as `from_float`.
/// \tparam Float `float`, `double` or `long double`.
template<std::floating_point Float, std::size_t Bits, unsigned D>
[[nodiscard]] inline Float to_float(basic_fixed<Bits, D> value) noexcept {
    return detail::to_float_kernel(detail::to_int256_raw(value.raw()), D, Float{});
}

// 0.4 compatibility surface: fixed at 12 digits. Generic replacement:
// from_float<Target>(value, rounding). See fixed.hpp.
[[nodiscard]] inline std::expected<FP64, ArithmeticError>
from_double64(double value, Rounding rounding = Rounding::nearest_even) noexcept {
    return from_float<FP64>(value, rounding);
}

[[nodiscard]] inline std::expected<FP128, ArithmeticError>
from_double128(double value, Rounding rounding = Rounding::nearest_even) noexcept {
    return from_float<FP128>(value, rounding);
}

// The pair of to_double below. Both are conveniences for the Float-generic
// from_float / to_float above; `double` is spelled out because it is the one
// callers ask for by name.
/// `from_float<Target>` with the source type fixed to `double`. Reports NaN
/// and infinity as `ArithmeticError::invalid_value`; rounds on the
/// destination's decimal grid otherwise.
template<typename Target>
[[nodiscard]] inline std::expected<Target, ArithmeticError>
from_double(double value, Rounding rounding = Rounding::nearest_even) noexcept {
    return from_float<Target>(value, rounding);
}

/// `to_float<double>`. Cannot fail, and is not exact: the result is the
/// nearest `double` to the value.
template<std::size_t Bits, unsigned D>
[[nodiscard]] inline double to_double(basic_fixed<Bits, D> value) noexcept {
    return to_float<double>(value);
}

} // namespace fixedwide
