#pragma once
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <fixedwide/wide.hpp>
#include <cstdint>
#include <expected>
#include <type_traits>

namespace fixedwide::detail {

// Halfway tie detection for nearest-even.
template<class UInt>
[[nodiscard]] constexpr bool nearest_even_increment(UInt q, UInt r, UInt d) noexcept {
    const UInt tie = q & ~d & UInt(1);
    return r > (d >> 1) - tie;
}

template<class UInt>
[[nodiscard]] constexpr std::expected<UInt, ArithmeticError> round_magnitude(
    UInt quotient, UInt remainder, UInt divisor, bool negative, Rounding rounding, UInt limit) noexcept {
    if (quotient > limit) return std::unexpected(ArithmeticError::overflow);
    if (remainder == 0) return quotient;
    bool increment = false;
    switch (rounding) {
    case Rounding::toward_zero: break;
    case Rounding::floor: increment = negative; break;
    case Rounding::ceil: increment = !negative; break;
    case Rounding::nearest_even:
        increment = nearest_even_increment(quotient, remainder, divisor);
        break;
    case Rounding::nearest_away:
        increment = (remainder >= divisor - remainder);
        break;
    case Rounding::exact:
        return std::unexpected(ArithmeticError::inexact);
    }
    if (increment) {
        if (quotient == limit) return std::unexpected(ArithmeticError::overflow);
        quotient += UInt(1);
    }
    return quotient;
}

// Low-level primitives for x86-64 / native vs portable
#if (defined(__x86_64__) || defined(_M_X64)) && (defined(__GNUC__) || defined(__clang__)) && !defined(FIXEDWIDE_FORCE_PORTABLE)

[[nodiscard]] inline std::uint64_t div128by64(std::uint64_t high, std::uint64_t low,
                                              std::uint64_t divisor, std::uint64_t& remainder) noexcept {
    std::uint64_t quotient;
    __asm__("divq %[divisor]"
            : "=a"(quotient), "=d"(remainder)
            : "a"(low), "d"(high), [divisor] "r"(divisor)
            : "cc");
    return quotient;
}

struct SignedQuotient64 { std::int64_t quotient; std::int64_t remainder; };
[[nodiscard]] inline SignedQuotient64 div_signed64(std::int64_t high, std::uint64_t low, std::int64_t divisor) noexcept {
    std::int64_t quotient, remainder;
    __asm__("idivq %[divisor]"
            : "=a"(quotient), "=d"(remainder)
            : "a"(low), "d"(high), [divisor] "r"(divisor)
            : "cc");
    return {quotient, remainder};
}

inline void mul64x64(std::uint64_t u, std::uint64_t v, std::uint64_t& hi, std::uint64_t& lo) noexcept {
    __asm__("mulq %[v]"
            : "=a"(lo), "=d"(hi)
            : "a"(u), [v] "r"(v)
            : "cc");
}

inline void imul64x64(std::int64_t u, std::int64_t v, std::int64_t& hi, std::uint64_t& lo) noexcept {
    std::uint64_t h;
    __asm__("imulq %[v]"
            : "=a"(lo), "=d"(h)
            : "a"(u), [v] "r"(v)
            : "cc");
    hi = static_cast<std::int64_t>(h);
}

#else

// Pure portable C++23 fallback implementations
inline void mul64x64(std::uint64_t u, std::uint64_t v, std::uint64_t& hi, std::uint64_t& lo) noexcept {
    std::uint64_t u0 = u & 0xFFFF'FFFFULL;
    std::uint64_t u1 = u >> 32;
    std::uint64_t v0 = v & 0xFFFF'FFFFULL;
    std::uint64_t v1 = v >> 32;

    std::uint64_t w0 = u0 * v0;
    std::uint64_t t = u1 * v0 + (w0 >> 32);
    std::uint64_t w1 = t & 0xFFFF'FFFFULL;
    std::uint64_t w2 = t >> 32;

    w1 += u0 * v1;
    w2 += (w1 >> 32);
    w1 &= 0xFFFF'FFFFULL;

    hi = u1 * v1 + w2;
    lo = (w1 << 32) | (w0 & 0xFFFF'FFFFULL);
}

[[nodiscard]] inline std::uint64_t div128by64(std::uint64_t high, std::uint64_t low,
                                              std::uint64_t divisor, std::uint64_t& remainder) noexcept {
    // High must be strictly less than divisor.
    // Binary long division: 64 steps
    std::uint64_t q = 0;
    std::uint64_t rem = high;
    for (int i = 63; i >= 0; --i) {
        std::uint64_t carry = (rem >> 63) & 1;
        rem = (rem << 1) | ((low >> i) & 1);
        if (carry || rem >= divisor) {
            rem -= divisor;
            q |= (1ULL << i);
        }
    }
    remainder = rem;
    return q;
}

struct SignedQuotient64 { std::int64_t quotient; std::int64_t remainder; };
[[nodiscard]] inline SignedQuotient64 div_signed64(std::int64_t high, std::uint64_t low, std::int64_t divisor) noexcept {
    bool neg_num = high < 0;
    bool neg_den = divisor < 0;
    std::uint64_t uhi = static_cast<std::uint64_t>(high);
    std::uint64_t ulo = low;
    if (neg_num) {
        ulo = ~ulo + 1;
        uhi = ~uhi + (ulo == 0 ? 1 : 0);
    }
    std::uint64_t udiv = neg_den ? (0ULL - static_cast<std::uint64_t>(divisor)) : static_cast<std::uint64_t>(divisor);
    std::uint64_t urem;
    std::uint64_t uq = div128by64(uhi, ulo, udiv, urem);
    std::int64_t q = static_cast<std::int64_t>(uq);
    std::int64_t r = static_cast<std::int64_t>(urem);
    if (neg_num != neg_den) q = -q;
    if (neg_num) r = -r;
    return {q, r};
}

inline void imul64x64(std::int64_t u, std::int64_t v, std::int64_t& hi, std::uint64_t& lo) noexcept {
    bool neg = (u < 0) != (v < 0);
    std::uint64_t uu = u < 0 ? (0ULL - static_cast<std::uint64_t>(u)) : static_cast<std::uint64_t>(u);
    std::uint64_t vv = v < 0 ? (0ULL - static_cast<std::uint64_t>(v)) : static_cast<std::uint64_t>(v);
    std::uint64_t h, l;
    mul64x64(uu, vv, h, l);
    if (neg) {
        l = ~l + 1;
        h = ~h + (l == 0 ? 1 : 0);
    }
    hi = static_cast<std::int64_t>(h);
    lo = l;
}

#endif

struct Quotient128 { wide::uint128 quotient; wide::uint128 remainder; };

[[nodiscard]] inline Quotient128 divide128by64(wide::uint128 numerator, std::uint64_t divisor) noexcept {
    const auto high = numerator.high;
    const auto low = numerator.low;
    if (high == 0) return {wide::uint128(low / divisor), wide::uint128(low % divisor)};
    if (high < divisor) {
        std::uint64_t remainder;
        const auto quotient = div128by64(high, low, divisor, remainder);
        return {wide::uint128(quotient), wide::uint128(remainder)};
    }
    const std::uint64_t qhigh = high / divisor;
    std::uint64_t remainder = high % divisor;
    const std::uint64_t qlow = div128by64(remainder, low, divisor, remainder);
    return {wide::uint128(qlow, qhigh), wide::uint128(remainder)};
}

[[nodiscard]] inline Quotient128 divide128(wide::uint128 numerator, wide::uint128 divisor, bool need_remainder = true) noexcept {
    if (divisor.high == 0) return divide128by64(numerator, divisor.low);
    if (numerator < divisor) return {wide::uint128(0), numerator};

#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    unsigned __int128 num = static_cast<unsigned __int128>(numerator);
    unsigned __int128 den = static_cast<unsigned __int128>(divisor);
    unsigned __int128 q = num / den;
    unsigned __int128 r = need_remainder ? (num - q * den) : 0;
    return {wide::uint128(q), wide::uint128(r)};
#else
    // Divisor >= 2^64 proves that quotient fits in 64 bits.
    // Binary search / Knuth D for 2 limbs by 2 limbs:
    int shift = std::countl_zero(divisor.high);
    wide::uint128 norm_d = divisor << shift;
    wide::uint128 norm_n = numerator << shift;
    std::uint64_t v1 = norm_d.high;
    std::uint64_t v0 = norm_d.low;
    std::uint64_t u2 = shift == 0 ? 0 : (numerator.high >> (64 - shift));
    std::uint64_t u1 = norm_n.high;
    std::uint64_t u0 = norm_n.low;

    std::uint64_t rhat = 0;
    std::uint64_t qhat;
    if (u2 == v1) {
        qhat = ~0ULL;
        rhat = u1 + v1;
    } else {
        qhat = div128by64(u2, u1, v1, rhat);
    }
    std::uint64_t ph, pl;
    mul64x64(qhat, v0, ph, pl);
    while (qhat != ~0ULL && (ph > rhat || (ph == rhat && pl > u0))) {
        --qhat;
        if (rhat + v1 < rhat) break; // carry out
        rhat += v1;
    }
    wide::uint128 rem = (numerator - divisor * wide::uint128(qhat));
    return {wide::uint128(qhat), rem};
#endif
}

} // namespace fixedwide::detail
