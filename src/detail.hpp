#pragma once
#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <fixedwide/wide.hpp>
#include <array>
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
    // Add branchlessly. Whether a rounding mode increments is a coin flip on
    // real data, so branching on it costs a mispredict on roughly every
    // operation; the overflow test below is almost never true and predicts
    // perfectly. Branching on `increment` here was worth 1.4 mispredicts per
    // operation and 78% of the wide nearest-even multiply.
    if (increment && quotient == limit) return std::unexpected(ArithmeticError::overflow);
    return quotient + static_cast<UInt>(increment);
}

inline constexpr std::uint64_t pow10_u64[19] = {
    1ULL,
    10ULL,
    100ULL,
    1'000ULL,
    10'000ULL,
    100'000ULL,
    1'000'000ULL,
    10'000'000ULL,
    100'000'000ULL,
    1'000'000'000ULL,
    10'000'000'000ULL,
    100'000'000'000ULL,
    1'000'000'000'000ULL,
    10'000'000'000'000ULL,
    100'000'000'000'000ULL,
    1'000'000'000'000'000ULL,
    10'000'000'000'000'000ULL,
    100'000'000'000'000'000ULL,
    1'000'000'000'000'000'000ULL
};

[[nodiscard]] constexpr std::uint64_t pow10(unsigned exp) noexcept {
    return exp < 19 ? pow10_u64[exp] : 0ULL;
}

// compute_pow10 is a shift-add loop. Calling it with a runtime exponent — which
// is what generalising the scale into a function argument caused — runs that
// loop on every arithmetic operation. These tables are built once at compile
// time so a runtime exponent costs one indexed load instead.
template<typename T, unsigned N>
inline constexpr std::array<T, N> pow10_table = [] {
    std::array<T, N> table{};
    for (unsigned i = 0; i < N; ++i) table[i] = compute_pow10<T>(i);
    // Every entry must exceed its predecessor. A power of ten that wrapped its
    // storage type is then a compile error rather than a silently wrong
    // constant, which is the failure mode this table is easiest to get wrong in.
    for (unsigned i = 1; i < N; ++i) {
        if (!(table[i] > table[i - 1])) throw "pow10 table entry overflowed its type";
    }
    return table;
}();

// How many powers of ten the table holds.
//
// sizeof(T) is the wrong question. 10^38 fits an unsigned 128-bit integer, but
// 10^77 does NOT fit a SIGNED 256-bit one: a table sized from sizeof held a
// wrapped value in its last slot. Size it instead from the same per-width cap
// that basic_fixed enforces, which is by construction the largest exponent any
// caller can legitimately ask for.
template<typename T>
inline constexpr unsigned pow10_limit = max_decimals_for_bits<sizeof(T) * 8>() + 1;

// Table lookup for any width, falling back to the loop only for exponents that
// cannot be represented anyway (where the caller already reports overflow).
template<typename T>
[[nodiscard]] constexpr T pow10_wide(unsigned exp) noexcept {
    if (exp < pow10_limit<T>) return pow10_table<T, pow10_limit<T>>[exp];
    return compute_pow10<T>(exp);
}

// ---------------------------------------------------------------------------
// Compile-time decimal scale
//
// 0.4 was a single-scale library, so its kernels saw `scale` as a constant and
// the optimiser folded every bound, branch and 64-bit division that used it.
// basic_fixed<Bits, D> turned that constant into a function argument, and the
// cost is not subtle: `i128_max / scale` alone became a __udivti3 call on every
// division, and `magnitude / scale` lost its reciprocal multiply.
//
// with_decimals gives the constant back. One switch at the top of a compiled
// kernel hands the body an integral_constant, so `scale_of<D>()` is constexpr
// again. Every call site passes the same D, so the switch target is perfectly
// predicted. Scales beyond the dispatched range stay correct on the runtime
// path — they simply do not get the folding.
// ---------------------------------------------------------------------------
// i128_max / 10^k, so the division fast path can range-check without issuing a
// runtime 128-bit divide (which is a __udivti3 call) on every operation.
template<typename T>
inline constexpr std::array<T, pow10_limit<T>> pow10_bound = [] {
    std::array<T, pow10_limit<T>> table{};
    const T limit = static_cast<T>((~static_cast<T>(0)) >> 1);
    for (unsigned i = 0; i < pow10_limit<T>; ++i) table[i] = limit / compute_pow10<T>(i);
    return table;
}();
// The scale as a compile-time constant, for the scale-specialised kernels.
template<unsigned D, typename T>
inline constexpr T scale_v = compute_pow10<T>(D);

inline constexpr unsigned dynamic_decimals = ~0u;

template<unsigned D, typename T>
[[nodiscard]] constexpr T scale_of(unsigned decimals) noexcept {
    if constexpr (D == dynamic_decimals || D >= pow10_limit<T>) return pow10_wide<T>(decimals);
    else return pow10_table<T, pow10_limit<T>>[D];
}

template<typename F>
[[nodiscard]] constexpr decltype(auto) with_decimals(unsigned decimals, F&& f) {
    switch (decimals) {
    case  0: return f(std::integral_constant<unsigned,  0>{});
    case  1: return f(std::integral_constant<unsigned,  1>{});
    case  2: return f(std::integral_constant<unsigned,  2>{});
    case  3: return f(std::integral_constant<unsigned,  3>{});
    case  4: return f(std::integral_constant<unsigned,  4>{});
    case  5: return f(std::integral_constant<unsigned,  5>{});
    case  6: return f(std::integral_constant<unsigned,  6>{});
    case  7: return f(std::integral_constant<unsigned,  7>{});
    case  8: return f(std::integral_constant<unsigned,  8>{});
    case  9: return f(std::integral_constant<unsigned,  9>{});
    case 10: return f(std::integral_constant<unsigned, 10>{});
    case 11: return f(std::integral_constant<unsigned, 11>{});
    case 12: return f(std::integral_constant<unsigned, 12>{});
    case 13: return f(std::integral_constant<unsigned, 13>{});
    case 14: return f(std::integral_constant<unsigned, 14>{});
    case 15: return f(std::integral_constant<unsigned, 15>{});
    case 16: return f(std::integral_constant<unsigned, 16>{});
    case 17: return f(std::integral_constant<unsigned, 17>{});
    case 18: return f(std::integral_constant<unsigned, 18>{});
    default: return f(std::integral_constant<unsigned, dynamic_decimals>{});
    }
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
    std::int64_t q = static_cast<std::int64_t>(neg_num != neg_den ? (0ULL - uq) : uq);
    std::int64_t r = static_cast<std::int64_t>(neg_num ? (0ULL - urem) : urem);
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
