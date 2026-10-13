#pragma once

#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <array>
#include <bit>
#include <cstring>
#include <expected>
#include <limits>
#include <type_traits>

namespace fixedwide::detail {

enum class binary_float_format { unknown, binary32, binary64, x87, binary128 };

// Precision AND layout, not sizeof(long double). The constant bit patterns
// also exclude mixed-word-endian IEEE layouts. Unknown formats keep the kernel.
template<typename Float>
inline constexpr binary_float_format binary_float_format_v = [] {
    using Limits = std::numeric_limits<Float>;
    if constexpr (!Limits::is_iec559 || Limits::radix != 2) {
        return binary_float_format::unknown;
    } else if constexpr (sizeof(Float) == 4 && Limits::digits == 24 && Limits::max_exponent == 128) {
        if constexpr (std::bit_cast<std::uint32_t>(Float{1}) == 0x3f80'0000u) return binary_float_format::binary32;
    } else if constexpr (sizeof(Float) == 8 && Limits::digits == 53 && Limits::max_exponent == 1024) {
        if constexpr (std::bit_cast<std::uint64_t>(Float{1}) == 0x3ff0'0000'0000'0000ULL)
            return binary_float_format::binary64;
    } else if constexpr (sizeof(Float) == 16 && Limits::digits == 113 && Limits::max_exponent == 16384) {
        constexpr auto one = std::bit_cast<std::array<std::uint64_t, 2>>(Float{1});
        if constexpr ((std::endian::native == std::endian::little && one[0] == 0 &&
                       one[1] == 0x3fff'0000'0000'0000ULL) ||
                      (std::endian::native == std::endian::big && one[1] == 0 && one[0] == 0x3fff'0000'0000'0000ULL))
            return binary_float_format::binary128;
    }
#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
    // x87 stores an explicit integer bit and has ABI padding. Only the known
    // little-endian x86 layout is decoded; never inspect its padding bytes.
    if constexpr (Limits::radix == 2 && Limits::digits == 64 && Limits::max_exponent == 16384 &&
                  (sizeof(Float) == 10 || sizeof(Float) == 12 || sizeof(Float) == 16) &&
                  std::endian::native == std::endian::little)
        return binary_float_format::x87;
#endif
    return binary_float_format::unknown;
}();

template<typename Target, typename Float>
inline constexpr bool use_fast_from_float_v = binary_float_format_v<Float> != binary_float_format::unknown &&
                                              Target::bits <= 128 && Target::fractional_digits <= 38;

template<typename Float>
using float_significand_t =
    std::conditional_t<(std::numeric_limits<Float>::digits <= 64), std::uint64_t, wide::uint128>;

template<typename Float>
struct decoded_float {
    float_significand_t<Float> significand{};
    int power{};
    bool negative{};
    bool invalid{};
};

template<typename Float>
[[nodiscard]] inline decoded_float<Float> decode_float(const Float& value) noexcept {
    constexpr auto format = binary_float_format_v<Float>;
    static_assert(format != binary_float_format::unknown);
    if constexpr (format == binary_float_format::binary32 || format == binary_float_format::binary64) {
        using Bits = std::conditional_t<format == binary_float_format::binary32, std::uint32_t, std::uint64_t>;
        constexpr unsigned fraction = std::numeric_limits<Float>::digits - 1;
        constexpr unsigned max_exponent = 2 * std::numeric_limits<Float>::max_exponent - 1;
        constexpr int bias = std::numeric_limits<Float>::max_exponent - 1;
        const auto bits = std::bit_cast<Bits>(value);
        const auto exponent = static_cast<unsigned>((bits >> fraction) & max_exponent);
        const auto significand = (bits & ((Bits{1} << fraction) - 1)) | (exponent == 0 ? Bits{} : Bits{1} << fraction);
        return {significand, static_cast<int>(exponent == 0 ? 1 : exponent) - bias - static_cast<int>(fraction),
                (bits >> (sizeof(Bits) * 8 - 1)) != 0, exponent == max_exponent};
    } else if constexpr (format == binary_float_format::x87) {
        std::uint64_t significand;
        std::uint16_t sign_exponent;
        std::memcpy(&significand, &value, sizeof(significand));
        std::memcpy(&sign_exponent, reinterpret_cast<const unsigned char*>(&value) + 8, sizeof(sign_exponent));
        const unsigned exponent = sign_exponent & 0x7fff;
        // Pseudo-denormals (e=0, integer bit=1) equal the corresponding e=1
        // value. Unnormals (e!=0, integer bit=0) are invalid x87 operands.
        return {significand, static_cast<int>(exponent == 0 ? 1 : exponent) - 16383 - 63, (sign_exponent & 0x8000) != 0,
                exponent == 0x7fff || (exponent != 0 && (significand >> 63) == 0)};
    } else {
        const auto words = std::bit_cast<std::array<std::uint64_t, 2>>(value);
        constexpr bool little = std::endian::native == std::endian::little;
        const auto low = words[little ? 0 : 1];
        const auto high = words[little ? 1 : 0];
        const auto exponent = static_cast<unsigned>((high >> 48) & 0x7fff);
        const auto significand_high = (high & 0x0000'ffff'ffff'ffffULL) | (exponent == 0 ? 0ULL : 1ULL << 48);
        return {wide::uint128(low, significand_high), static_cast<int>(exponent == 0 ? 1 : exponent) - 16383 - 112,
                (high >> 63) != 0, exponent == 0x7fff};
    }
}

#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
using float_magnitude128 = unsigned __int128;
#else
using float_magnitude128 = wide::uint128;
#endif

// 10^D = 5^D * 2^D. All scales of a <=128-bit target have D<=38.
// Pick the smallest intermediate that holds BOTH the product and destination;
// this makes binary32 D<=17 a 64-bit path, without runtime range speculation.
template<typename UInt, unsigned D>
inline constexpr UInt float_pow5_v = [] {
    static_assert(D <= 38);
    UInt value{1};
    for (unsigned i = 0; i < D; ++i) value = value * UInt{5};
    return value;
}();

template<typename Target, typename Float>
inline constexpr unsigned float_intermediate_bits_v = [] {
    constexpr auto five = float_pow5_v<wide::uint128, Target::fractional_digits>;
    constexpr unsigned scale_bits = Target::fractional_digits == 0
                                        ? 0
                                        : static_cast<unsigned>(five.high == 0 ? 64 - wide::countl_zero64(five.low)
                                                                               : 128 - wide::countl_zero64(five.high));
    constexpr unsigned product_bits = std::numeric_limits<Float>::digits + scale_bits;
    static_assert(product_bits <= 256);
    return Target::bits <= 64 && product_bits <= 64 ? 64u : product_bits <= 128 ? 128u : 256u;
}();

template<typename Target, typename Float>
[[nodiscard]] inline std::expected<Target, ArithmeticError> from_float_fast(const Float& value,
                                                                            Rounding rounding) noexcept {
    static_assert(use_fast_from_float_v<Target, Float>);
    constexpr unsigned width = float_intermediate_bits_v<Target, Float>;
    using UInt = std::conditional_t<width == 64, std::uint64_t,
                                    std::conditional_t<width == 128, float_magnitude128, wide::uint256>>;
    const auto decoded = decode_float(value);
    if (decoded.invalid) return std::unexpected(ArithmeticError::invalid_value);
    if (decoded.significand == float_significand_t<Float>{}) return Target{};
    const bool negative = decoded.negative;
    const int power = decoded.power + static_cast<int>(Target::fractional_digits);
    const UInt product = static_cast<UInt>(decoded.significand) * float_pow5_v<UInt, Target::fractional_digits>;
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
        if (shift >= width || product > (limit >> shift)) return std::unexpected(ArithmeticError::overflow);
        quotient = product << shift;
    } else {
        const auto shift = static_cast<unsigned>(-power);
        if (shift < width) {
            quotient = product >> shift;
            const UInt remainder = product - (quotient << shift);
            const UInt half = UInt{1} << (shift - 1);
            inexact = remainder != UInt{};
            above_half = remainder > half;
            at_half = remainder == half;
        } else {
            // A nonzero input must retain its remainder for exact/floor/ceil.
            // At shift==width the product may exceed half. Only larger shifts
            // are unconditionally below half; never construct 2^width.
            inexact = true;
            if (shift == width) {
                const UInt half = UInt{1} << (width - 1);
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
        case Rounding::nearest_even: increment = above_half | (at_half & ((quotient & UInt{1}) != UInt{})); break;
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
        const auto magnitude = static_cast<wide::uint128>(quotient).low;
        const auto raw = static_cast<UnsignedRaw>(negative ? std::uint64_t{0} - magnitude : magnitude);
        return Target::from_raw(std::bit_cast<Raw>(raw));
    } else {
        const auto raw = static_cast<wide::uint128>(negative ? UInt{} - quotient : quotient);
        return Target::from_raw(wide::int128(raw.low, raw.high));
    }
}

} // namespace fixedwide::detail
