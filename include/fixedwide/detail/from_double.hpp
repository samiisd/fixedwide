#pragma once

#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <bit>
#include <expected>
#include <limits>
#include <type_traits>

namespace fixedwide::detail {

// An internal dispatch hook: tests assert both the accelerated types and the
// fallback boundary. Float remains a template parameter so the bit_cast is not
// instantiated on platforms where double is not IEEE binary64.
template<typename Target, typename Float>
inline constexpr bool use_fast_from_float_v =
    std::is_same_v<Float, double> && sizeof(Float) == sizeof(std::uint64_t) &&
    std::numeric_limits<Float>::is_iec559 && std::numeric_limits<Float>::radix == 2 &&
    std::numeric_limits<Float>::digits == 53 && std::numeric_limits<Float>::max_exponent == 1024 &&
    Target::bits <= 128 && Target::fractional_digits <= 32;

#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
using float_magnitude128 = unsigned __int128;
#else
using float_magnitude128 = wide::uint128;
#endif

// 10^D = 5^D * 2^D: moving the powers of two into the exponent keeps the
// significand product smaller. (2^53 - 1) * 5^32 < 2^128, but D=33 can overflow.
// This also keeps the multiplier in 64 bits through D=27.
template<unsigned D>
inline constexpr float_magnitude128 float_pow5_v = [] {
    static_assert(D <= 32);
    float_magnitude128 value{1};
    for (unsigned i = 0; i < D; ++i) value = value * float_magnitude128{5};
    return value;
}();

template<typename Target, typename Float>
[[nodiscard]] inline std::expected<Target, ArithmeticError> from_double_fast(Float value, Rounding rounding) noexcept {
    static_assert(use_fast_from_float_v<Target, Float>);
    using UInt = float_magnitude128;
    const auto bits = std::bit_cast<std::uint64_t>(value);
    const bool negative = (bits >> 63) != 0;
    const auto exponent = static_cast<unsigned>((bits >> 52) & 0x7ff);
    std::uint64_t significand = bits & 0x000f'ffff'ffff'ffffULL;
    if (exponent == 0x7ff) return std::unexpected(ArithmeticError::invalid_value);
    if (exponent == 0) {
        if (significand == 0) return Target{}; // Both signs of zero.
    } else {
        significand |= std::uint64_t{1} << 52;
    }

    const int power = static_cast<int>(exponent == 0 ? 1 : exponent) - 1075 +
                      static_cast<int>(Target::fractional_digits);
    const UInt product = UInt{significand} * float_pow5_v<Target::fractional_digits>;
    // Magnitudes are asymmetric: the negative limit includes the signed minimum.
    const UInt limit = (UInt{1} << (Target::bits - 1)) - UInt{!negative};
    UInt quotient{};
    bool inexact = false;
    bool above_half = false;
    bool at_half = false;
    if (power >= 0) {
        const auto shift = static_cast<unsigned>(power);
        // Check before shifting; never form an overflowing numerator or shift
        // a native integer by its width. Exceeding the target is already final.
        if (shift >= 128 || product > (limit >> shift)) return std::unexpected(ArithmeticError::overflow);
        quotient = product << shift;
    } else {
        const auto shift = static_cast<unsigned>(-power);
        if (shift < 128) {
            quotient = product >> shift;
            const UInt remainder = product - (quotient << shift);
            const UInt half = UInt{1} << (shift - 1);
            inexact = remainder != UInt{};
            above_half = remainder > half;
            at_half = remainder == half;
        } else {
            // A nonzero input must retain its remainder for exact/floor/ceil.
            // At shift==128, D=32 can exceed half; only larger shifts are
            // unconditionally below half. Never construct the divisor 2^128.
            inexact = true;
            if (shift == 128) {
                const UInt half = UInt{1} << 127;
                above_half = product > half;
                at_half = product == half;
            }
        }
    }

    // Match the generic kernel's error precedence: overflow before inexact.
    if (quotient > limit) return std::unexpected(ArithmeticError::overflow);
    if (inexact) {
        bool increment = false;
        switch (rounding) {
        case Rounding::toward_zero: break;
        case Rounding::floor: increment = negative; break;
        case Rounding::ceil: increment = !negative; break;
        case Rounding::nearest_even:
            increment = above_half | (at_half & ((static_cast<std::uint64_t>(quotient) & 1) != 0));
            break;
        case Rounding::nearest_away: increment = above_half | at_half; break;
        case Rounding::exact: return std::unexpected(ArithmeticError::inexact);
        }
        if (increment && quotient == limit) return std::unexpected(ArithmeticError::overflow);
        quotient = quotient + UInt{increment};
    }

    // Apply the sign as unsigned modular arithmetic, never by negating a
    // signed minimum. The storage conversion preserves that two's-complement
    // bit pattern without a second, out-of-line width/range check.
    if constexpr (Target::bits <= 64) {
        using Raw = typename Target::raw_type;
        using UnsignedRaw = std::make_unsigned_t<Raw>;
        const auto magnitude = static_cast<std::uint64_t>(quotient);
        const auto raw = static_cast<UnsignedRaw>(negative ? std::uint64_t{0} - magnitude : magnitude);
        return Target::from_raw(std::bit_cast<Raw>(raw));
    } else {
        const UInt raw = negative ? UInt{} - quotient : quotient;
        return Target::from_raw(wide::int128(static_cast<std::uint64_t>(raw), static_cast<std::uint64_t>(raw >> 64)));
    }
}

} // namespace fixedwide::detail
