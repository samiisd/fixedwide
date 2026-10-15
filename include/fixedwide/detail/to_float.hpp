#pragma once

#include <fixedwide/detail/from_float.hpp>

namespace fixedwide::detail {

// The recognized output layouts are the same ones used by the input decoder.
// Fixed256 and unknown FP layouts retain the compiled compatibility kernel.
template<typename Float, std::size_t Bits>
inline constexpr bool use_fast_to_float_v = Bits <= 128 && binary_float_format_v<Float> != binary_float_format::unknown;

// Encode an already rounded, positive significand at an unbiased exponent.
// A subnormal has exponent=min_exponent-1 and no leading integer bit.
template<typename Float, typename UInt>
[[nodiscard]] inline Float encode_float(UInt significand, int exponent, bool negative) noexcept {
    constexpr auto format = binary_float_format_v<Float>;
    constexpr unsigned fraction = std::numeric_limits<Float>::digits - 1;
    constexpr int bias = std::numeric_limits<Float>::max_exponent - 1;
    const bool normal = significand >= (UInt{1} << fraction);
    const auto field = normal ? static_cast<unsigned>(exponent + bias) : 0u;
    const auto words = static_cast<wide::uint128>(significand);
    const auto low = words.low;
    if constexpr (format == binary_float_format::binary32) {
        const auto bits =
            (std::uint32_t{negative} << 31) | (field << 23) | (static_cast<std::uint32_t>(low) & 0x007f'ffffu);
        return std::bit_cast<Float>(bits);
    } else if constexpr (format == binary_float_format::binary64) {
        const auto bits =
            (std::uint64_t{negative} << 63) | (std::uint64_t{field} << 52) | (low & 0x000f'ffff'ffff'ffffULL);
        return std::bit_cast<Float>(bits);
    } else if constexpr (format == binary_float_format::x87) {
        // Initialize padding, but do not depend on its value surviving the ABI.
        Float value;
        std::array<unsigned char, sizeof(Float)> bytes{};
        const auto sign_exponent = static_cast<std::uint16_t>((negative ? 0x8000u : 0u) | field);
        std::memcpy(bytes.data(), &low, 8);
        std::memcpy(bytes.data() + 8, &sign_exponent, 2);
        std::memcpy(&value, bytes.data(), sizeof(value));
        return value;
    } else {
        const auto high =
            (std::uint64_t{negative} << 63) | (std::uint64_t{field} << 48) | (words.high & 0x0000'ffff'ffff'ffffULL);
        constexpr bool little = std::endian::native == std::endian::little;
        return std::bit_cast<Float>(std::array<std::uint64_t, 2>{little ? low : high, little ? high : low});
    }
}

// N has at most K bits. With M=floor(2^K / 5^D), floor(N*M / 2^K)
// underestimates floor(N / 5^D) by at most one, since N/2^K < 1.
// One exact remainder check corrects it. This is exact division by a constant,
// NOT an approximate floating reciprocal. See reports/TO_FLOAT_0_6_7.md.
template<unsigned K, unsigned D>
inline constexpr wide::uint128 to_float_reciprocal_v = [] {
    constexpr auto divisor = float_pow5_v<wide::uint256, D>;
    constexpr auto power = wide::uint256{1} << K;
    constexpr auto reciprocal = power / divisor;
    static_assert(reciprocal * divisor <= power && (reciprocal + wide::uint256{1}) * divisor > power);
    return static_cast<wide::uint128>(reciprocal);
}();

template<typename UInt>
struct to_float_division {
    UInt quotient;
    bool inexact;
};

template<unsigned K, unsigned D, unsigned Precision, typename UInt>
[[nodiscard]] inline to_float_division<UInt> to_float_quotient(UInt numerator) noexcept {
    if constexpr (D == 0) {
        return {numerator, false};
    } else if constexpr (K <= 64) {
        // The compiler's ordinary constant-division lowering is sufficient.
        constexpr auto divisor = float_pow5_v<UInt, D>;
        return {numerator / divisor, numerator % divisor != 0};
    }
#if defined(FIXEDWIDE_HAS_X86_64_ASM)
    else if constexpr (K <= 128 && Precision + 2 <= 64 && D <= 27) {
        // N/5^D has at most Precision+2 bits (<64 here), proving high<divisor.
        // The divisor is nonzero. Keep volatile so no compiler hoists a trapping
        // divide out of a path where these preconditions have been established.
        std::uint64_t q, r;
        __asm__ volatile(
            "divq %[divisor]"
            : "=a"(q), "=d"(r)
            : "a"(static_cast<std::uint64_t>(numerator)),
              "d"(static_cast<std::uint64_t>(numerator >> 64)), [divisor] "r"(float_pow5_v<std::uint64_t, D>)
            : "cc");
        return {UInt{q}, r != 0};
    }
#endif
    else {
        constexpr auto reciprocal = to_float_reciprocal_v<K, D>;
        UInt estimate;
        if constexpr (K <= 128 && Precision + 2 <= 64) {
            // The low 64 product bits cannot contribute after shifting by K.
            // These two 64x64 products also use the portable backend when forced.
            using U = float_magnitude128;
            constexpr auto m = reciprocal.low;
            const U low_product = U{static_cast<std::uint64_t>(numerator)} * U{m};
            const U high_product = U{static_cast<std::uint64_t>(numerator >> 64)} * U{m} + (low_product >> 64);
            estimate = static_cast<UInt>(high_product >> (K - 64));
        } else if constexpr (Precision + 2 <= 64) {
            // Here 128<K<=143: the numerator has only three nonzero limbs,
            // and the reciprocal fits 64 bits. Propagate the two low carries.
            using U = float_magnitude128;
            constexpr auto m = reciprocal.low;
            const U p0 = U{numerator.limbs[0]} * U{m};
            const U p1 = U{numerator.limbs[1]} * U{m} + (p0 >> 64);
            const U p2 = U{numerator.limbs[2]} * U{m} + (p1 >> 64);
            estimate = UInt{static_cast<std::uint64_t>(p2 >> (K - 128))};
        } else {
            const auto n = static_cast<wide::uint256>(numerator);
            constexpr auto m = wide::uint256{reciprocal};
            if constexpr (K + Precision + 2 <= 256) {
                estimate = static_cast<UInt>(static_cast<wide::uint128>((n * m) >> K));
            } else {
                // Binary128 can require a 318-bit product. Split at 128 bits
                // and propagate the carry; each individual product fits u256.
                const auto low_product = wide::uint256{static_cast<wide::uint128>(n)} * m;
                const auto high_product = (n >> 128) * m + (low_product >> 128);
                estimate = static_cast<UInt>(static_cast<wide::uint128>(high_product >> (K - 128)));
            }
        }
        // The true remainder before correction is <2*5^D <2^90 for D<=38.
        // Low-128-bit modular subtraction therefore recovers it exactly, even
        // when N and estimate*5^D need more than 128 bits individually.
        using R = float_magnitude128;
        constexpr auto divisor = float_pow5_v<R, D>;
        const R remainder = static_cast<R>(static_cast<wide::uint128>(numerator)) -
                            static_cast<R>(static_cast<wide::uint128>(estimate)) * divisor;
        return {estimate + UInt{remainder >= divisor}, remainder != R{} && remainder != divisor};
    }
}

template<typename Float, std::size_t Bits, unsigned D>
[[nodiscard]] inline Float to_float_fast(basic_fixed<Bits, D> value) noexcept {
    static_assert(use_fast_to_float_v<Float, Bits>);
    constexpr unsigned precision = std::numeric_limits<Float>::digits;
    constexpr auto five = float_pow5_v<wide::uint128, D>;
    constexpr int divisor_bits =
        five.high == 0 ? 64 - wide::countl_zero64(five.low) : 128 - wide::countl_zero64(five.high);
    // Normalize to precision+1+divisor_bits input bits. Division then gives
    // precision+1 OR precision+2 result bits: at least one guard bit, plus an
    // exact sticky bit from both the normalization and the division remainder.
    constexpr unsigned k = precision + 1 + static_cast<unsigned>(divisor_bits);
    using Magnitude = std::conditional_t<Bits <= 64, std::uint64_t, float_magnitude128>;
    Magnitude raw;
    bool negative;
    if constexpr (Bits <= 64) {
        raw = static_cast<std::uint64_t>(static_cast<std::int64_t>(value.raw()));
        negative = value.raw() < 0;
    } else {
        raw = static_cast<Magnitude>(wide::uint128{value.raw().low, value.raw().high});
        negative = value.raw().is_negative();
    }
    const Magnitude mask = Magnitude{} - Magnitude{negative};
    const Magnitude magnitude = (raw ^ mask) + Magnitude{negative};
    if (magnitude == Magnitude{}) return Float{0};
    int bits;
    if constexpr (Bits <= 64) {
        bits = 64 - wide::countl_zero64(magnitude);
    } else {
        const auto high = static_cast<std::uint64_t>(magnitude >> 64);
        bits = high == 0 ? 64 - wide::countl_zero64(static_cast<std::uint64_t>(magnitude))
                         : 128 - wide::countl_zero64(high);
    }
    using UInt =
        std::conditional_t<k <= 64, std::uint64_t, std::conditional_t<k <= 128, float_magnitude128, wide::uint256>>;
    using ShiftUInt = std::conditional_t<(k <= 64 && Bits <= 64), std::uint64_t,
                                         std::conditional_t<k <= 128, float_magnitude128, wide::uint256>>;
    const ShiftUInt n = static_cast<ShiftUInt>(magnitude);
    const int shift = static_cast<int>(k) - bits;
    UInt numerator;
    bool sticky = false;
    if (shift >= 0) {
        numerator = static_cast<UInt>(n << static_cast<unsigned>(shift));
    } else {
        const auto right = static_cast<unsigned>(-shift);
        numerator = static_cast<UInt>(n >> right);
        sticky = (static_cast<ShiftUInt>(numerator) << right) != n;
    }
    const auto division = to_float_quotient<k, D, precision>(numerator);
    const UInt quotient = division.quotient;
    sticky |= division.inexact;
    using RoundUInt = std::conditional_t<(precision + 2 <= 64), std::uint64_t, float_magnitude128>;
    const RoundUInt q = static_cast<RoundUInt>(static_cast<wide::uint128>(quotient));
    const bool extra = q >= (RoundUInt{1} << (precision + 1));
    int exponent = bits - divisor_bits - static_cast<int>(D) - 1 + static_cast<int>(extra);
    unsigned discard = 1u + static_cast<unsigned>(extra);
    constexpr int min_exponent = std::numeric_limits<Float>::min_exponent - 1;
    if constexpr (std::numeric_limits<Float>::min_exponent > -128) {
        // Only binary32 can become subnormal for a <=128-bit fixed type.
        // Round once at subnormal spacing, not once to normal and again to tiny.
        if (exponent < min_exponent) {
            discard += static_cast<unsigned>(min_exponent - exponent);
            exponent = min_exponent;
        }
    }
    RoundUInt rounded = q >> discard;
    const RoundUInt tail = q - (rounded << discard);
    const RoundUInt half = RoundUInt{1} << (discard - 1);
    const bool increment = (tail > half) | ((tail == half) & (sticky | ((rounded & RoundUInt{1}) != RoundUInt{})));
    rounded = rounded + RoundUInt{increment};
    if (rounded == (RoundUInt{1} << precision)) {
        rounded = rounded >> 1;
        ++exponent;
    }
    // No supported <=128-bit source can overflow these Float formats:
    // |value| <= 2^127, and the least nonzero magnitude is 10^-38.
    return encode_float<Float>(rounded, exponent, negative);
}

} // namespace fixedwide::detail
