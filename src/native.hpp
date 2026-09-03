#pragma once
#include "detail.hpp"
#include <bit>

// Internal computation types.
//
// The PUBLIC storage stays wide::intN — stable, trivially copyable limbs, no
// compiler ABI in the object representation. This header is what the compiled
// kernels *compute* in, and on any compiler with __int128 it is the compiler's
// own 128-bit integer held in register pairs.
//
// 0.5.0-alpha.3 computed in the limb structs as well, which is why its wide
// FP128 rows ran up to 2.1x slower than 0.4: every shift, compare and add went
// through a member function on a 32-byte object passed in memory. The 256-bit
// intermediate here is a plain {hi, lo} pair of native halves rather than a
// _BitInt(256), so GCC and Clang both keep it in registers and no _BitInt
// appears anywhere in the library.
//
// Without __int128 this header is not used at all; the portable limb backend in
// detail.hpp/division.hpp takes over and is differential-tested against this one.

#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
#define FIXEDWIDE_HAS_NATIVE_128 1

namespace fixedwide::detail::nat {

using u128 = unsigned __int128;
using i128 = __int128;

inline constexpr u128 u128_max = ~u128{0};
inline constexpr i128 i128_max = static_cast<i128>(u128_max >> 1);

[[nodiscard]] constexpr u128 load(wide::uint128 v) noexcept {
    return (static_cast<u128>(v.high) << 64) | v.low;
}
[[nodiscard]] constexpr i128 load(wide::int128 v) noexcept {
    return static_cast<i128>((static_cast<u128>(v.high) << 64) | v.low);
}
[[nodiscard]] constexpr wide::int128 store(i128 v) noexcept {
    return wide::int128(static_cast<std::uint64_t>(v),
                        static_cast<std::uint64_t>(static_cast<u128>(v) >> 64));
}
[[nodiscard]] constexpr wide::uint128 store(u128 v) noexcept {
    return wide::uint128(static_cast<std::uint64_t>(v), static_cast<std::uint64_t>(v >> 64));
}
[[nodiscard]] constexpr u128 magnitude(i128 v) noexcept {
    return v < 0 ? u128{0} - static_cast<u128>(v) : static_cast<u128>(v);
}
// Negation in unsigned arithmetic, so magnitude 2^127 is well defined.
[[nodiscard]] constexpr i128 apply_sign(u128 value, bool negative) noexcept {
    return static_cast<i128>(negative ? u128{0} - value : value);
}

// A 256-bit magnitude as two native halves. Never spilled to a limb array.
struct u256 {
    u128 lo{};
    u128 hi{};
};

// Calculate the full product ONCE. Probing for 128-bit overflow and then
// redoing the multiplication at 256 bits duplicates the expensive work.
[[nodiscard]] inline u256 multiply128(u128 a, u128 b) noexcept {
    if (((a | b) >> 64) == 0) {
        return u256{static_cast<u128>(static_cast<std::uint64_t>(a)) * static_cast<std::uint64_t>(b), 0};
    }
    const auto a0 = static_cast<std::uint64_t>(a);
    const auto a1 = static_cast<std::uint64_t>(a >> 64);
    const auto b0 = static_cast<std::uint64_t>(b);
    const auto b1 = static_cast<std::uint64_t>(b >> 64);
    const u128 p00 = static_cast<u128>(a0) * b0;
    const u128 p01 = static_cast<u128>(a0) * b1;
    const u128 p10 = static_cast<u128>(a1) * b0;
    const u128 p11 = static_cast<u128>(a1) * b1;
    const u128 middle = (p00 >> 64) + static_cast<std::uint64_t>(p01) + static_cast<std::uint64_t>(p10);
    u256 result;
    result.lo = (middle << 64) | static_cast<std::uint64_t>(p00);
    result.hi = p11 + (p01 >> 64) + (p10 >> 64) + (middle >> 64);
    return result;
}

struct Quotient { u128 quotient; u128 remainder; };

// General 128/64: reduce the high limb first, then div128by64's precondition holds.
[[nodiscard]] inline Quotient divide128by64(u128 numerator, std::uint64_t divisor) noexcept {
    const auto high = static_cast<std::uint64_t>(numerator >> 64);
    const auto low = static_cast<std::uint64_t>(numerator);
    if (high == 0) return {low / divisor, low % divisor};
    if (high < divisor) {
        std::uint64_t remainder;
        const auto quotient = div128by64(high, low, divisor, remainder);
        return {quotient, remainder};
    }
    const std::uint64_t qhigh = high / divisor;
    std::uint64_t remainder = high % divisor;
    const std::uint64_t qlow = div128by64(remainder, low, divisor, remainder);
    return {(static_cast<u128>(qhigh) << 64) | qlow, remainder};
}

// Use the compiler's native-width division before reaching for a limb algorithm.
// Recover the remainder by multiplication rather than a second runtime divide.
[[nodiscard]] inline Quotient divide128(u128 numerator, u128 divisor, bool need_remainder = true) noexcept {
    if ((divisor >> 64) == 0) return divide128by64(numerator, static_cast<std::uint64_t>(divisor));
    if (numerator < divisor) return {0, numerator};
    // divisor >= 2^64 proves this quotient fits one 64-bit limb.
    const auto quotient = static_cast<std::uint64_t>(numerator / divisor);
    return {quotient, need_remainder ? numerator - static_cast<u128>(quotient) * divisor : u128{0}};
}

// Append one radix-2^64 digit to a remainder smaller than the normalized
// two-limb divisor. Returns one quotient digit and replaces the remainder.
[[nodiscard]] inline std::uint64_t divide_digit(u128& remainder, std::uint64_t next,
                                                std::uint64_t v0, std::uint64_t v1) noexcept {
    const auto high = static_cast<std::uint64_t>(remainder >> 64);
    const auto low = static_cast<std::uint64_t>(remainder);
    std::uint64_t quotient, rhat;
    bool carry = false;
    if (high == v1) {
        quotient = UINT64_MAX;
        rhat = low + v1;
        carry = rhat < low;
    } else {
        quotient = div128by64(high, low, v1, rhat);
    }
    while (!carry && static_cast<u128>(quotient) * v0 > ((static_cast<u128>(rhat) << 64) | next)) {
        --quotient;
        const std::uint64_t previous = rhat;
        rhat += v1;
        carry = rhat < previous;
    }
    // For a TWO-limb divisor this correction checks the entire product, so no
    // general Algorithm-D add-back stage is needed. If rhat carried, the
    // unrepresented 2^128 cancels the product borrow; unsigned subtraction still
    // produces the exact remainder, known to be < the normalized divisor.
    remainder = ((static_cast<u128>(rhat) << 64) | next) - static_cast<u128>(quotient) * v0;
    return quotient;
}

// 256/128 with a quotient proven to fit 128 bits. always_inline so callers with
// a constant divisor keep specialising it instead of copying 256-bit temporaries
// through an extra call ABI.
[[nodiscard, gnu::always_inline]] inline std::expected<Quotient, ArithmeticError>
divide_narrow(u256 numerator, u128 divisor) noexcept {
    if (divisor == 0) return std::unexpected(ArithmeticError::division_by_zero);
    // floor(N/d) < 2^128 iff floor(N/2^128) < d: exact, not a heuristic.
    if (numerator.hi >= divisor) return std::unexpected(ArithmeticError::overflow);
    if (numerator.hi == 0) return divide128(numerator.lo, divisor);
    if ((divisor >> 64) == 0) {
        const auto d = static_cast<std::uint64_t>(divisor);
        // hi < d <= 2^64-1: exactly two divq instructions suffice.
        auto remainder = static_cast<std::uint64_t>(numerator.hi);
        const auto qhigh = div128by64(remainder, static_cast<std::uint64_t>(numerator.lo >> 64), d, remainder);
        const auto qlow = div128by64(remainder, static_cast<std::uint64_t>(numerator.lo), d, remainder);
        return Quotient{(static_cast<u128>(qhigh) << 64) | qlow, remainder};
    }
    const auto shift = static_cast<unsigned>(
        std::countl_zero(static_cast<std::uint64_t>(divisor >> 64)));
    const u128 normalized_divisor = divisor << shift;
    const auto v0 = static_cast<std::uint64_t>(normalized_divisor);
    const auto v1 = static_cast<std::uint64_t>(normalized_divisor >> 64);
    u128 remainder = shift == 0 ? numerator.hi : (numerator.hi << shift) | (numerator.lo >> (128 - shift));
    const u128 normalized_low = numerator.lo << shift;
    const auto middle = static_cast<std::uint64_t>(normalized_low >> 64);
    std::uint64_t qhigh = 0;
    // Compare the top 192 bits of the numerator against the divisor: if smaller,
    // the quotient fits 64 bits and its leading digit is known to be zero.
    const bool quotient_fits_64 = (numerator.hi >> 64) == 0 &&
        (((numerator.hi << 64) | (numerator.lo >> 64)) < divisor);
    if (quotient_fits_64) {
        remainder = (remainder << 64) | middle;
    } else {
        qhigh = divide_digit(remainder, middle, v0, v1);
    }
    const auto qlow = divide_digit(remainder, static_cast<std::uint64_t>(normalized_low), v0, v1);
    return Quotient{(static_cast<u128>(qhigh) << 64) | qlow, remainder >> shift};
}

[[nodiscard]] inline std::expected<i128, ArithmeticError> finish(
    Quotient value, u128 divisor, bool negative, Rounding rounding,
    u128 positive_limit = static_cast<u128>(i128_max)) noexcept {
    const auto rounded = round_magnitude(value.quotient, value.remainder, divisor, negative,
                                         rounding, positive_limit + static_cast<unsigned>(negative));
    if (!rounded) return std::unexpected(rounded.error());
    return apply_sign(*rounded, negative);
}

} // namespace fixedwide::detail::nat

#endif // __SIZEOF_INT128__
