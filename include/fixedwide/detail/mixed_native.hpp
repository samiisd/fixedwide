#pragma once
#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <compare>
#include <expected>

// Narrow fast paths for the mixed-width, mixed-scale operations.
//
// The general kernels build an exact rational in 1024-bit limbs and divide with
// Knuth's algorithm. That is the right answer for Fixed256 operands, and absurd
// for a Fixed64<8> against a Fixed64<12>. Measured before this header existed:
//
//     same-type Fixed128<12> add     0.54 ns
//     add_to across scales         418    ns
//     mul_to across scales         339    ns
//     comparison across scales     269    ns
//
// against a spec that asks the abstraction to cost "essentially nothing beyond
// the arithmetic actually required".
//
// Every bound below is a compile-time constant built from the widths and scales
// in the types, so a given call either takes the native path unconditionally or
// never mentions it. When the bound does not hold, the general kernel runs
// unchanged: this adds a fast path, it does not narrow the contract.

#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
#define FIXEDWIDE_HAS_MIXED_NATIVE 1

namespace fixedwide::detail::mixed_native {

using i128 = __int128;

// Bits needed to hold 10^k, not a power of ten: 10^k < 16^k, so 4k always
// suffices. Deliberately loose -- it only decides whether the fast path is
// taken, and being conservative can never be wrong.
[[nodiscard]] constexpr unsigned bits_for_pow10(unsigned k) noexcept {
    return 4 * k;
}

template<unsigned K>
inline constexpr i128 pow10_v = [] {
    i128 value = 1;
    for (unsigned i = 0; i < K; ++i) value *= 10;
    return value;
}();

// Aligning two scales multiplies each side by ten to the difference from the
// smaller scale. The products must stay inside signed 128 bits.
template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db>
[[nodiscard]] constexpr bool alignment_fits() noexcept {
    constexpr unsigned common = Da < Db ? Da : Db;
    return BitsA <= 64 && BitsB <= 64 && BitsA + bits_for_pow10(Db - common) <= 126 &&
           BitsB + bits_for_pow10(Da - common) <= 126;
}

// Exact comparison with no division at all: scale both sides to a common
// exponent and compare the integers.
template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db, typename RawA, typename RawB>
[[nodiscard]] constexpr std::strong_ordering compare(RawA a, RawB b) noexcept {
    constexpr unsigned common = Da < Db ? Da : Db;
    const i128 lhs = static_cast<i128>(a) * pow10_v<Db - common>;
    const i128 rhs = static_cast<i128>(b) * pow10_v<Da - common>;
    if (lhs < rhs) return std::strong_ordering::less;
    if (lhs > rhs) return std::strong_ordering::greater;
    return std::strong_ordering::equal;
}

using u128 = unsigned __int128;

// Largest representable magnitude for a destination width and result sign.
template<std::size_t Bits>
[[nodiscard]] constexpr u128 limit_magnitude_u128(bool negative) noexcept {
    const u128 positive = Bits >= 128 ? (~u128{0} >> 1) : ((u128{1} << (Bits - 1)) - 1);
    return positive + static_cast<u128>(negative);
}

struct Divided {
    u128 quotient;
    u128 remainder;
};

// Every divisor that reaches here is a power of ten, and the ones a mixed
// operation actually uses fit 64 bits. A generic 128-by-128 divide is a
// __udivti3 call out to libgcc, and it was the whole cost of a mixed rescale:
// one or two hardware divisions do the same work. The generic form stays for a
// divisor past 2^64 and for constant evaluation, where no instruction runs.
[[nodiscard]] constexpr Divided divide_magnitude(u128 magnitude, u128 denominator) noexcept {
#if defined(FIXEDWIDE_HAS_X86_64_ASM)
    if !consteval {
        if ((denominator >> 64) == 0) {
            const auto d = static_cast<std::uint64_t>(denominator);
            const auto high = static_cast<std::uint64_t>(magnitude >> 64);
            const auto low = static_cast<std::uint64_t>(magnitude);
            if (high == 0) return {u128{low / d}, u128{low % d}};
            // Reduce the high limb first so the quotient of the divq below is
            // proven to fit 64 bits; it faults otherwise.
            const std::uint64_t quotient_high = high / d;
            const std::uint64_t partial = high % d;
            std::uint64_t quotient_low, remainder;
            __asm__("divq %[divisor]"
                    : "=a"(quotient_low), "=d"(remainder)
                    : "a"(low), "d"(partial), [divisor] "r"(d)
                    : "cc");
            return {(static_cast<u128>(quotient_high) << 64) | quotient_low, u128{remainder}};
        }
    }
#endif
    const u128 quotient = magnitude / denominator;
    return {quotient, magnitude - quotient * denominator};
}

// Divide and round to the requested mode, with the destination's range applied
// in the SAME ORDER as round_magnitude: the range test comes before the rounding
// decision, so a result that cannot be represented is an overflow whatever the
// mode -- not an `inexact` that happens to be checked first. Getting this order
// wrong is how the fast path and the general kernel disagreed.
[[nodiscard]] constexpr std::expected<i128, ArithmeticError> divide_rounded(i128 value, i128 divisor, Rounding rounding,
                                                                            u128 limit) noexcept {
    const bool negative = value < 0;
    const u128 magnitude = negative ? (u128{0} - static_cast<u128>(value)) : static_cast<u128>(value);
    const u128 denominator = static_cast<u128>(divisor);
    const auto divided = divide_magnitude(magnitude, denominator);
    u128 quotient = divided.quotient;
    const u128 remainder = divided.remainder;

    if (quotient > limit) return std::unexpected(ArithmeticError::overflow);
    if (remainder != 0) {
        if (rounding == Rounding::exact) return std::unexpected(ArithmeticError::inexact);
        bool increment = false;
        switch (rounding) {
        case Rounding::toward_zero: break;
        case Rounding::floor: increment = negative; break;
        case Rounding::ceil: increment = !negative; break;
        case Rounding::nearest_even: {
            const u128 twice = remainder * 2;
            increment = twice > denominator || (twice == denominator && (quotient & 1) != 0);
            break;
        }
        case Rounding::nearest_away: increment = remainder * 2 >= denominator; break;
        case Rounding::exact: break;
        }
        if (increment && quotient == limit) return std::unexpected(ArithmeticError::overflow);
        quotient += static_cast<u128>(increment);
    }
    return negative ? -static_cast<i128>(quotient) : static_cast<i128>(quotient);
}

// The destination's representable range, as signed 128-bit bounds.
template<std::size_t Bits>
[[nodiscard]] constexpr i128 dest_max() noexcept {
    if constexpr (Bits >= 128)
        return (static_cast<i128>(1) << 126) - 1 + (static_cast<i128>(1) << 126);
    else
        return (static_cast<i128>(1) << (Bits - 1)) - 1;
}
template<std::size_t Bits>
[[nodiscard]] constexpr i128 dest_min() noexcept {
    return -dest_max<Bits>() - 1;
}

template<std::size_t Bits>
[[nodiscard]] constexpr std::expected<i128, ArithmeticError> check_range(i128 value) noexcept {
    if (value < dest_min<Bits>() || value > dest_max<Bits>()) {
        return std::unexpected(ArithmeticError::overflow);
    }
    return value;
}

// The magnitude of a signed BitsA x BitsB product needs BitsA + BitsB - 2 bits,
// so the signed result needs one more. A 64x64 product is exactly 127 bits and
// does fit; a bound of BitsA + BitsB would wrongly reject it.
[[nodiscard]] constexpr unsigned product_bits(std::size_t a, std::size_t b) noexcept {
    return static_cast<unsigned>(a + b) - 1;
}

// --- fixed_cast ----------------------------------------------------------
// Widening the scale is a multiply; narrowing it is a divide and one rounding.
template<std::size_t BitsSrc, unsigned Ds, std::size_t BitsDest, unsigned Dd>
[[nodiscard]] constexpr bool cast_fits() noexcept {
    if constexpr (BitsSrc > 64 || BitsDest > 128)
        return false;
    else if constexpr (Dd >= Ds)
        return BitsSrc + bits_for_pow10(Dd - Ds) <= 126;
    else
        return true; // narrowing only shrinks the magnitude
}

template<std::size_t BitsSrc, unsigned Ds, std::size_t BitsDest, unsigned Dd, typename RawSrc>
[[nodiscard]] constexpr std::expected<i128, ArithmeticError> cast(RawSrc a, Rounding rounding) noexcept {
    if constexpr (Dd >= Ds) {
        return check_range<BitsDest>(static_cast<i128>(a) * pow10_v<Dd - Ds>);
    } else {
        return divide_rounded(static_cast<i128>(a), pow10_v<Ds - Dd>, rounding,
                              limit_magnitude_u128<BitsDest>(a < RawSrc{0}));
    }
}

// --- add_to / sub_to -----------------------------------------------------
// The exact sum lives at the larger of the two input scales; converting it to
// the destination scale is the only place a rounding can occur.
template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db, std::size_t BitsDest, unsigned Dd>
[[nodiscard]] constexpr bool add_fits() noexcept {
    constexpr unsigned common = Da < Db ? Da : Db;
    constexpr unsigned sum_scale = Da < Db ? Db : Da;
    if constexpr (!alignment_fits<BitsA, Da, BitsB, Db>() || BitsDest > 128) return false;
    // One extra bit for the sum, then the widening to the destination scale.
    else if constexpr (Dd >= sum_scale) {
        constexpr unsigned aligned = (BitsA > BitsB ? BitsA : BitsB) + bits_for_pow10(sum_scale - common) + 1;
        return aligned + bits_for_pow10(Dd - sum_scale) <= 126;
    } else
        return true;
}

template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db, std::size_t BitsDest, unsigned Dd,
         typename RawA, typename RawB>
[[nodiscard]] constexpr std::expected<i128, ArithmeticError> add_sub(RawA a, RawB b, bool subtract,
                                                                     Rounding rounding) noexcept {
    constexpr unsigned sum_scale = Da < Db ? Db : Da;
    const i128 lhs = static_cast<i128>(a) * pow10_v<sum_scale - Da>;
    const i128 rhs = static_cast<i128>(b) * pow10_v<sum_scale - Db>;
    const i128 sum = subtract ? (lhs - rhs) : (lhs + rhs);
    if constexpr (Dd >= sum_scale) {
        return check_range<BitsDest>(sum * pow10_v<Dd - sum_scale>);
    } else {
        return divide_rounded(sum, pow10_v<sum_scale - Dd>, rounding, limit_magnitude_u128<BitsDest>(sum < 0));
    }
}

// --- mul_to --------------------------------------------------------------
// A signed 64x64 product always fits signed 128 bits, and it carries scale
// Da + Db. Only the conversion to the destination scale rounds.
template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db, std::size_t BitsDest, unsigned Dd>
[[nodiscard]] constexpr bool mul_fits() noexcept {
    if constexpr (BitsA > 64 || BitsB > 64 || BitsDest > 128)
        return false;
    else if constexpr (Dd >= Da + Db)
        return product_bits(BitsA, BitsB) + bits_for_pow10(Dd - Da - Db) <= 127;
    else
        return true;
}

template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db, std::size_t BitsDest, unsigned Dd,
         typename RawA, typename RawB>
[[nodiscard]] constexpr std::expected<i128, ArithmeticError> mul(RawA a, RawB b, Rounding rounding) noexcept {
    const i128 product = static_cast<i128>(a) * static_cast<i128>(b);
    if constexpr (Dd >= Da + Db) {
        return check_range<BitsDest>(product * pow10_v<Dd - Da - Db>);
    } else {
        return divide_rounded(product, pow10_v<Da + Db - Dd>, rounding, limit_magnitude_u128<BitsDest>(product < 0));
    }
}

// --- div_to --------------------------------------------------------------
// a/b has scale Da - Db, so reaching the destination scale means scaling the
// numerator by 10^(Dd + Db - Da), or the denominator by the inverse when that
// exponent is negative. One rounding, at the end.
template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db, std::size_t BitsDest, unsigned Dd>
[[nodiscard]] constexpr bool div_fits() noexcept {
    if constexpr (BitsA > 64 || BitsB > 64 || BitsDest > 128)
        return false;
    else if constexpr (Dd + Db >= Da)
        return BitsA + bits_for_pow10(Dd + Db - Da) <= 126;
    else
        return BitsB + bits_for_pow10(Da - Db - Dd) <= 126;
}

template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db, std::size_t BitsDest, unsigned Dd,
         typename RawA, typename RawB>
[[nodiscard]] constexpr std::expected<i128, ArithmeticError> div(RawA a, RawB b, Rounding rounding) noexcept {
    if (b == RawB{0}) return std::unexpected(ArithmeticError::division_by_zero);
    i128 numerator = static_cast<i128>(a);
    i128 denominator = static_cast<i128>(b);
    if constexpr (Dd + Db >= Da)
        numerator *= pow10_v<Dd + Db - Da>;
    else
        denominator *= pow10_v<Da - Db - Dd>;
    // divide_rounded needs a positive divisor; move the sign to the numerator.
    if (denominator < 0) {
        denominator = -denominator;
        numerator = -numerator;
    }
    return divide_rounded(numerator, denominator, rounding, limit_magnitude_u128<BitsDest>(numerator < 0));
}

// --- mul_div_to ----------------------------------------------------------
// a*b/c at the destination scale is ra*rb*10^(Dc+Dd) / (rc*10^(Da+Db)),
// with the surplus exponent applied to whichever side keeps both inside 128
// bits. Still one rounding.
template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db, std::size_t BitsC, unsigned Dc,
         std::size_t BitsDest, unsigned Dd>
[[nodiscard]] constexpr bool mul_div_fits() noexcept {
    if constexpr (BitsA > 64 || BitsB > 64 || BitsC > 64 || BitsDest > 128)
        return false;
    else if constexpr (Dc + Dd >= Da + Db) {
        return product_bits(BitsA, BitsB) + bits_for_pow10(Dc + Dd - Da - Db) <= 127;
    } else {
        return product_bits(BitsA, BitsB) <= 127 && BitsC + bits_for_pow10(Da + Db - Dc - Dd) <= 126;
    }
}

template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db, std::size_t BitsC, unsigned Dc,
         std::size_t BitsDest, unsigned Dd, typename RawA, typename RawB, typename RawC>
[[nodiscard]] constexpr std::expected<i128, ArithmeticError> mul_div(RawA a, RawB b, RawC c,
                                                                     Rounding rounding) noexcept {
    if (c == RawC{0}) return std::unexpected(ArithmeticError::division_by_zero);
    i128 numerator = static_cast<i128>(a) * static_cast<i128>(b);
    i128 denominator = static_cast<i128>(c);
    if constexpr (Dc + Dd >= Da + Db)
        numerator *= pow10_v<Dc + Dd - Da - Db>;
    else
        denominator *= pow10_v<Da + Db - Dc - Dd>;
    if (denominator < 0) {
        denominator = -denominator;
        numerator = -numerator;
    }
    return divide_rounded(numerator, denominator, rounding, limit_magnitude_u128<BitsDest>(numerator < 0));
}

} // namespace fixedwide::detail::mixed_native

#endif // __SIZEOF_INT128__
