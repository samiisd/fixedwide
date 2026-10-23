#pragma once

/// \file
/// Exact decimal exponents, not approximate fractional logarithms.

#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <array>
#include <bit>
#include <expected>

namespace fixedwide::detail {

template<class UInt, std::size_t Count>
inline constexpr auto log10_powers = [] {
    std::array<UInt, Count> powers{};
    powers[0] = UInt(1);
    for (std::size_t i = 1; i < Count; ++i) {
        powers[i] = (powers[i - 1] << 3) + (powers[i - 1] << 1);
    }
    return powers;
}();

// For a positive signed raw integer, 1 <= bit_width <= 255. Throughout
// that finite domain, t = (bit_width * 1233) >> 12 is either the decimal
// exponent or one above it. An exact comparison with 10^t resolves the
// ambiguity. The tests verify both ends of every binary bin using an
// independent repeated-division oracle; no floating-point estimate is used.
[[nodiscard]] constexpr unsigned log10_estimate(unsigned bit_width) noexcept {
    return (bit_width * 1233U) >> 12;
}

// Preconditions for these internal overloads: raw > 0, and raw fits the
// corresponding signed storage type. No absolute value of a signed minimum.
[[nodiscard]] constexpr int log10_floor_raw(std::uint64_t raw) noexcept {
    const auto t = log10_estimate(static_cast<unsigned>(std::bit_width(raw)));
    return static_cast<int>(t) - static_cast<int>(raw < log10_powers<std::uint64_t, 19>[t]);
}

[[nodiscard]] constexpr int log10_floor_raw(wide::int128 raw) noexcept {
    const auto bits = raw.high != 0 ? 64U + static_cast<unsigned>(std::bit_width(raw.high))
                                    : static_cast<unsigned>(std::bit_width(raw.low));
    const auto t = log10_estimate(bits);
    const wide::uint128 magnitude(raw.low, raw.high);
    return static_cast<int>(t) - static_cast<int>(magnitude < log10_powers<wide::uint128, 39>[t]);
}

[[nodiscard]] constexpr int log10_floor_raw(wide::int256 raw) noexcept {
    unsigned bits = 0;
    for (std::size_t i = 4; i != 0; --i) {
        if (raw.limbs[i - 1] != 0) {
            bits = static_cast<unsigned>((i - 1) * 64) + static_cast<unsigned>(std::bit_width(raw.limbs[i - 1]));
            break;
        }
    }
    const auto t = log10_estimate(bits);
    const wide::uint256 magnitude(raw.limbs[0], raw.limbs[1], raw.limbs[2], raw.limbs[3]);
    return static_cast<int>(t) - static_cast<int>(magnitude < log10_powers<wide::uint256, 77>[t]);
}

} // namespace fixedwide::detail

namespace fixedwide {

/// Exact floor(log10(value)) for a positive fixed-point value.
///
/// Returns the unique integer e such that 10^e <= value < 10^(e+1).
/// Values below one have negative exponents: 0.01 -> -2, 0.99 -> -1.
/// Zero and negative values report ArithmeticError::invalid_value.
///
/// This is a decimal-magnitude query, NOT a fractional logarithm. It neither
/// converts to floating point nor approximates the result, including directly
/// below a power of ten. constexpr, allocation-free, and independent of the
/// floating-point environment. The scale is subtracted exactly as an integer.
///
///     using F = fixedwide::Fixed64<2>;
///     static_assert(*fixedwide::log10_floor(F::from_raw(999)) == 0);
///     static_assert(*fixedwide::log10_floor(F::from_raw(1000)) == 1);
///
/// \return the signed exponent, or ArithmeticError::invalid_value.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr std::expected<int, ArithmeticError> log10_floor(basic_fixed<Bits, D> value) noexcept {
    if (value <= basic_fixed<Bits, D>{}) return std::unexpected(ArithmeticError::invalid_value);
    if constexpr (Bits <= 64) {
        return detail::log10_floor_raw(static_cast<std::uint64_t>(value.raw())) - static_cast<int>(D);
    } else {
        return detail::log10_floor_raw(value.raw()) - static_cast<int>(D);
    }
}

} // namespace fixedwide
