#include <fixedwide/arithmetic.hpp>
#include "detail.hpp"
#include "division.hpp"
#include "limbs.hpp"
#include "native.hpp"

namespace fixedwide::detail {

namespace {

// Fits in signed 64 bits helper
[[nodiscard]] constexpr bool fits64(std::int64_t) noexcept { return true; }

[[nodiscard]] constexpr bool fits64(wide::int128 val) noexcept {
    auto h = static_cast<std::int64_t>(val.high);
    return (h == 0 && (val.low >> 63) == 0) || (h == -1 && (val.low >> 63) == 1);
}

[[nodiscard]] constexpr bool common_rounding(Rounding r) noexcept {
    return r == Rounding::toward_zero || r == Rounding::nearest_even;
}

[[nodiscard]] constexpr bool quotient_fits_signed64(std::int64_t high, std::uint64_t low, std::int64_t divisor) noexcept {
    (void)low;
    const auto bits = static_cast<std::uint64_t>(divisor);
    const auto mag = divisor < 0 ? std::uint64_t{0} - bits : bits;
    const auto half = mag / 2;
    const auto uhi = static_cast<std::uint64_t>(high);
    return uhi + half < 2 * half && !(divisor < 0 && uhi + half == 0);
}

[[nodiscard]] std::int64_t nearest_adjustment(SignedQuotient64 val, std::int64_t divisor, bool negative) noexcept {
    const auto rbits = static_cast<std::uint64_t>(val.remainder);
    const auto dbits = static_cast<std::uint64_t>(divisor);
    const auto r = val.remainder < 0 ? std::uint64_t{0} - rbits : rbits;
    const auto d = divisor < 0 ? std::uint64_t{0} - dbits : dbits;
    const bool increment = nearest_even_increment(static_cast<std::uint64_t>(val.quotient), r, d);
    const std::int64_t direction = negative ? -1 : 1;
    return direction & -static_cast<std::int64_t>(increment);
}

std::expected<std::int64_t, ArithmeticError> quotient64_general(
    std::int64_t high, std::uint64_t low, std::int64_t divisor, Rounding rounding) noexcept {
    bool neg_num = high < 0;
    bool neg_den = divisor < 0;
    bool negative = neg_num != neg_den;
    std::uint64_t uhi = static_cast<std::uint64_t>(high);
    std::uint64_t ulo = low;
    if (neg_num) {
        ulo = ~ulo + 1;
        uhi = ~uhi + (ulo == 0 ? 1 : 0);
    }
    std::uint64_t udiv = neg_den ? 0ULL - static_cast<std::uint64_t>(divisor) : static_cast<std::uint64_t>(divisor);
    if (uhi >= udiv) return std::unexpected(ArithmeticError::overflow);
    std::uint64_t rem;
    std::uint64_t q = div128by64(uhi, ulo, udiv, rem);
    std::uint64_t limit = static_cast<std::uint64_t>(INT64_MAX) + (negative ? 1 : 0);
    auto rounded = round_magnitude(q, rem, udiv, negative, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());
    if (negative) {
        if (*rounded == limit && negative) return INT64_MIN;
        return -static_cast<std::int64_t>(*rounded);
    }
    return static_cast<std::int64_t>(*rounded);
}

std::expected<std::int64_t, ArithmeticError> quotient64_signed(
    std::int64_t high, std::uint64_t low, std::int64_t divisor, Rounding rounding) noexcept {
    if (divisor == 0) return std::unexpected(ArithmeticError::division_by_zero);
    if (common_rounding(rounding) && quotient_fits_signed64(high, low, divisor)) {
        auto val = div_signed64(high, low, divisor);
        if (rounding == Rounding::nearest_even && val.remainder != 0) {
            const auto adj = nearest_adjustment(val, divisor, (high < 0) != (divisor < 0));
            if (__builtin_add_overflow(val.quotient, adj, &val.quotient)) {
                return quotient64_general(high, low, divisor, rounding);
            }
        }
        return val.quotient;
    }
    return quotient64_general(high, low, divisor, rounding);
}

#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
inline wide::int128 divide_native_general(wide::int128 numerator, wide::int128 divisor, Rounding rounding) noexcept {
    __int128 n = static_cast<__int128>(numerator);
    __int128 d = static_cast<__int128>(divisor);
    bool neg = (n < 0) != (d < 0);
    unsigned __int128 un = n < 0 ? (static_cast<unsigned __int128>(0) - static_cast<unsigned __int128>(n))
                                 : static_cast<unsigned __int128>(n);
    unsigned __int128 ud = d < 0 ? (static_cast<unsigned __int128>(0) - static_cast<unsigned __int128>(d))
                                 : static_cast<unsigned __int128>(d);
    unsigned __int128 q = un / ud;
    unsigned __int128 rem = un % ud;
    if (rem != 0) {
        bool inc = false;
        if (rounding == Rounding::nearest_even) {
            inc = nearest_even_increment(q, rem, ud);
        } else if (rounding == Rounding::floor) {
            inc = neg;
        } else if (rounding == Rounding::ceil) {
            inc = !neg;
        } else if (rounding == Rounding::nearest_away) {
            inc = (rem * 2 >= ud);
        }
        if (inc) q += 1;
    }
    if (neg) {
        unsigned __int128 neg_q = static_cast<unsigned __int128>(0) - q;
        return wide::int128(static_cast<std::uint64_t>(neg_q), static_cast<std::uint64_t>(neg_q >> 64));
    }
    return wide::int128(static_cast<std::uint64_t>(q), static_cast<std::uint64_t>(q >> 64));
}

inline wide::int128 divide_native(wide::int128 numerator, wide::int128 divisor, Rounding rounding) noexcept {
    if (fits64(divisor)) {
        std::int64_t d = static_cast<std::int64_t>(divisor.low);
        std::int64_t h = static_cast<std::int64_t>(numerator.high);
        std::uint64_t l = numerator.low;
        if (quotient_fits_signed64(h, l, d)) {
            auto val = div_signed64(h, l, d);
            if (rounding == Rounding::toward_zero || val.remainder == 0) return wide::int128(val.quotient);
            return wide::int128(val.quotient) + wide::int128(nearest_adjustment(val, d, (h < 0) != (d < 0)));
        }
    }
    return divide_native_general(numerator, divisor, rounding);
}

inline wide::int128 divide_scale_general(wide::int128 numerator, wide::uint128 scale, Rounding rounding) noexcept {
    bool neg = numerator.is_negative();
    auto mag = magnitude(numerator);
    auto divres = divide128(mag, scale, rounding != Rounding::toward_zero);
    wide::uint128 limit = wide::uint128::max() >> 1;
    if (neg) limit = limit + wide::uint128(1ULL);
    auto rounded = round_magnitude(divres.quotient, divres.remainder, scale, neg, rounding, limit);
    if (!rounded) return wide::int128{};
    if (neg) {
        wide::uint128 r = *rounded;
        wide::int128 signed_r(r.low, r.high);
        return -signed_r;
    }
    return wide::int128(rounded->low, rounded->high);
}

inline wide::int128 divide_product_by_scale(wide::int128 product, wide::uint128 scale, Rounding rounding) noexcept {
    if (scale.high == 0 && quotient_fits_signed64(static_cast<std::int64_t>(product.high), product.low, static_cast<std::int64_t>(scale.low))) {
        return divide_native(product, wide::int128(scale.low), rounding);
    }
    return divide_scale_general(product, scale, rounding);
}
#endif

} // namespace

std::expected<std::int64_t, ArithmeticError>
mul64_impl(std::int64_t a, std::int64_t b, std::int64_t scale, Rounding rounding) noexcept {
    std::int64_t hi;
    std::uint64_t lo;
    imul64x64(a, b, hi, lo);
    return quotient64_signed(hi, lo, scale, rounding);
}

std::expected<std::int64_t, ArithmeticError>
div64_impl(std::int64_t a, std::int64_t b, std::int64_t scale, Rounding rounding) noexcept {
    if (b == 0) return std::unexpected(ArithmeticError::division_by_zero);
    std::int64_t hi;
    std::uint64_t lo;
    imul64x64(a, scale, hi, lo);
    return quotient64_signed(hi, lo, b, rounding);
}

std::expected<std::int64_t, ArithmeticError>
mul_div64_impl(std::int64_t a, std::int64_t b, std::int64_t c, Rounding rounding) noexcept {
    if (c == 0) return std::unexpected(ArithmeticError::division_by_zero);
    std::int64_t hi;
    std::uint64_t lo;
    imul64x64(a, b, hi, lo);
    return quotient64_signed(hi, lo, c, rounding);
}

std::expected<std::int64_t, ArithmeticError>
quantize64_impl(std::int64_t a, unsigned cur_dec, unsigned target_dec, Rounding rounding) noexcept {
    if (target_dec > cur_dec) return std::unexpected(ArithmeticError::invalid_precision);
    if (target_dec == cur_dec) return a;
    std::int64_t divisor = pow10_wide<std::int64_t>(cur_dec - target_dec);
    bool neg = a < 0;
    std::uint64_t mag = neg ? 0ULL - static_cast<std::uint64_t>(a) : static_cast<std::uint64_t>(a);
    std::uint64_t udiv = static_cast<std::uint64_t>(divisor);
    std::uint64_t q = mag / udiv;
    std::uint64_t r = mag % udiv;
    std::uint64_t limit = static_cast<std::uint64_t>(INT64_MAX) + (neg ? 1 : 0);
    auto rounded = round_magnitude(q, r, udiv, neg, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());
    // Rescaling can overflow. Detect it with the multiply we have to do anyway
    // rather than a second runtime division by the same divisor.
    std::uint64_t res_mag;
    if (__builtin_mul_overflow(*rounded, udiv, &res_mag) || res_mag > limit) {
        return std::unexpected(ArithmeticError::overflow);
    }
    if (neg) {
        if (res_mag == limit && neg) return INT64_MIN;
        return -static_cast<std::int64_t>(res_mag);
    }
    return static_cast<std::int64_t>(res_mag);
}

// ---------------------------------------------------------------------------
// 128-bit fixed-point kernels.
//
// These compute in nat:: — the compiler's own __int128, held in register pairs.
// That is what 0.4 did, and its wide-path performance depended on it: alpha.3
// computed in the limb structs instead, so every shift, compare and add went
// through a member function on a 32-byte object passed in memory, and the wide
// FP128 rows ran up to 2.1x slower than 0.4.
//
// wide::int128 stays the public storage; load/store at the boundary are bit
// moves. Without __int128 none of this is compiled and the portable limb
// backend below takes over.
// ---------------------------------------------------------------------------
#if defined(FIXEDWIDE_HAS_NATIVE_128)

namespace {

using nat::i128;
using nat::u128;

[[nodiscard]] constexpr bool fits64_n(i128 value) noexcept {
    return value == static_cast<std::int64_t>(value);
}

// Write N = signed_high * 2^64 + low and h = floor(|d|/2). Then -h <= signed_high
// < h proves -h*2^64 <= N < h*2^64, hence a signed-64 quotient for positive d.
// The unsigned addition encodes that signed interval without signed overflow.
// For negative d the whole lowest slab is excluded, so +2^63 is impossible too.
[[nodiscard]] inline bool quotient_fits_signed64_n(i128 numerator, std::int64_t divisor) noexcept {
    const auto bits = static_cast<std::uint64_t>(divisor);
    const auto mag = divisor < 0 ? std::uint64_t{0} - bits : bits;
    const auto half = mag / 2;
    const auto high = static_cast<std::uint64_t>(static_cast<u128>(numerator) >> 64);
    return high + half < 2 * half && !(divisor < 0 && high + half == 0);
}

// All six rounding modes on unsigned magnitudes, including exact and overflow.
[[nodiscard, gnu::noinline]] std::expected<i128, ArithmeticError>
divide_signed_general_n(i128 numerator, i128 divisor, Rounding rounding) noexcept {
    const u128 denominator = nat::magnitude(divisor);
    return nat::finish(nat::divide128(nat::magnitude(numerator), denominator,
                                      rounding != Rounding::toward_zero),
                       denominator, (numerator < 0) != (divisor < 0), rounding);
}

// The wide fallback stays out of line. Inlining it drags a __divti3 call and a
// full rounding tail into every caller's hot path, which costs register pressure
// and code layout even on the operands that never reach it.
// q*d lies between zero and the numerator, so neither the product nor the
// subtraction can overflow. Reuse q instead of a second runtime divide.
[[nodiscard, gnu::noinline]] i128
divide_native_general_n(i128 numerator, i128 divisor, Rounding rounding) noexcept {
    const i128 quotient = numerator / divisor;
    if (rounding == Rounding::toward_zero) return quotient;
    const auto remainder = nat::magnitude(numerator - quotient * divisor);
    if (remainder == 0) return quotient;
    const bool increment = nearest_even_increment(static_cast<u128>(quotient), remainder,
                                                  nat::magnitude(divisor));
    return quotient + ((numerator < 0) != (divisor < 0) ? -i128{increment} : i128{increment});
}

// Callers prove divisor != 0 and that signed_min / -1 cannot occur.
[[nodiscard, gnu::noinline]] i128
divide_native_n(i128 numerator, i128 divisor, Rounding rounding) noexcept {
    if (fits64_n(divisor) && quotient_fits_signed64_n(numerator, static_cast<std::int64_t>(divisor))) {
        const auto d = static_cast<std::int64_t>(divisor);
        const auto value = div_signed64(static_cast<std::int64_t>(static_cast<u128>(numerator) >> 64),
                                        static_cast<std::uint64_t>(numerator), d);
        // Widen BEFORE rounding: +2^63 is a valid Fixed128 raw result.
        if (rounding == Rounding::toward_zero || value.remainder == 0) return value.quotient;
        return i128{value.quotient} + nearest_adjustment(value, d, (numerator < 0) != (divisor < 0));
    }
    return divide_native_general_n(numerator, divisor, rounding);
}

// For a full signed-128 product divided by the positive scale the rounded result
// always fits 128 bits, so no generic signed-128 divide or overflow check is needed.
[[gnu::noinline]] i128 divide_scale_general_n(i128 numerator, u128 scale, Rounding rounding) noexcept {
    auto value = (scale >> 64) == 0
        ? nat::divide128by64(nat::magnitude(numerator), static_cast<std::uint64_t>(scale))
        : nat::divide128(nat::magnitude(numerator), scale);
    if (rounding == Rounding::nearest_even) {
        value.quotient += static_cast<unsigned>(
            nearest_even_increment(value.quotient, value.remainder, scale));
    }
    return nat::apply_sign(value.quotient, numerator < 0);
}

// Only valid for toward_zero / nearest_even, where the result cannot overflow.
// noinline, like the general path: with a runtime scale this body no longer
// folds away the way 0.4's compile-time one did, and inlining it into the entry
// point put its branches and register pressure on the wide path too.
[[gnu::noinline]] i128
divide_product_by_scale_n(i128 product, u128 scale, Rounding rounding) noexcept {
    if ((scale >> 64) == 0 && quotient_fits_signed64_n(product, static_cast<std::int64_t>(scale)))
        return divide_native_n(product, static_cast<i128>(scale), rounding);
    return divide_scale_general_n(product, scale, rounding);
}

// Shared tail for the wide paths: a 256-bit magnitude over a 128-bit divisor.
// always_inline is load-bearing, not decoration: a 32-byte struct is passed in
// memory by the SysV ABI, so leaving this out of line makes every wide operation
// spill and reload the 256-bit product. That single boundary cost alpha.3 about
// half of its wide FP128 throughput.
[[nodiscard, gnu::always_inline]] inline std::expected<wide::int128, ArithmeticError>
finish_wide(nat::u256 numerator, u128 divisor, bool negative, Rounding rounding) noexcept {
    if (numerator.hi == 0) {
        auto rounded = nat::finish(nat::divide128(numerator.lo, divisor,
                                                  rounding != Rounding::toward_zero),
                                   divisor, negative, rounding);
        if (!rounded) return std::unexpected(rounded.error());
        return nat::store(*rounded);
    }
    auto quotient = nat::divide_narrow(numerator, divisor);
    if (!quotient) return std::unexpected(quotient.error());
    auto rounded = nat::finish(*quotient, divisor, negative, rounding);
    if (!rounded) return std::unexpected(rounded.error());
    return nat::store(*rounded);
}

} // namespace

// The compiled kernels are the WIDE path only. The narrow fast paths live in
// arithmetic.hpp, where D is a real compile-time constant and the code inlines
// straight into the caller. Duplicating them here cost twice over: an extra call
// level per operation (each one copying a 24-byte std::expected through memory),
// and, when they were dispatched per decimal count, twenty inlined copies that
// turned a 71-instruction entry point into a 992-instruction one.
// The compiled kernels are the WIDE path only. arithmetic.hpp keeps the narrow
// fast paths inline in the caller, where the scale is a compile-time constant.
// Duplicating them here paid twice: an extra call level per operation, each one
// returning a 24-byte std::expected through memory, and the fast path's register
// pressure spread across operands that never take it.
std::expected<wide::int128, ArithmeticError>
mul128_impl(wide::int128 a, wide::int128 b, unsigned decimals, Rounding rounding) noexcept {
    const i128 ra = nat::load(a);
    const i128 rb = nat::load(b);
    return finish_wide(nat::multiply128(nat::magnitude(ra), nat::magnitude(rb)),
                       pow10_wide<u128>(decimals), (ra < 0) != (rb < 0), rounding);
}

std::expected<wide::int128, ArithmeticError>
div128_impl(wide::int128 a, wide::int128 b, unsigned decimals, Rounding rounding) noexcept {
    const i128 ra = nat::load(a);
    const i128 rb = nat::load(b);
    if (rb == 0) return std::unexpected(ArithmeticError::division_by_zero);
    return finish_wide(nat::multiply128(nat::magnitude(ra), pow10_wide<u128>(decimals)),
                       nat::magnitude(rb), (ra < 0) != (rb < 0), rounding);
}

std::expected<wide::int128, ArithmeticError>
mul_div128_impl(wide::int128 a, wide::int128 b, wide::int128 c, Rounding rounding) noexcept {
    const i128 ra = nat::load(a);
    const i128 rb = nat::load(b);
    const i128 rc = nat::load(c);
    if (rc == 0) return std::unexpected(ArithmeticError::division_by_zero);
    return finish_wide(nat::multiply128(nat::magnitude(ra), nat::magnitude(rb)), nat::magnitude(rc),
                       (ra < 0) != ((rb < 0) != (rc < 0)), rounding);
}

#else // portable limb backend

std::expected<wide::int128, ArithmeticError>
mul128_impl(wide::int128 a, wide::int128 b, unsigned decimals, Rounding rounding) noexcept {
    auto scale = pow10_wide<wide::int128>(decimals);
    auto uscale = wide::uint128(scale.low, scale.high);

#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    if (common_rounding(rounding) && fits64(a) && fits64(b)) {
        std::int64_t hi;
        std::uint64_t lo;
        imul64x64(static_cast<std::int64_t>(a.low), static_cast<std::int64_t>(b.low), hi, lo);
        wide::int128 prod(lo, static_cast<std::uint64_t>(hi));
        return divide_product_by_scale(prod, uscale, rounding);
    }
#endif

    bool neg = a.is_negative() != b.is_negative();
    auto ma = magnitude(a);
    auto mb = magnitude(b);
    auto prod256 = multiply128(ma, mb);

    if (prod256.limbs[2] == 0 && prod256.limbs[3] == 0) {
        wide::uint128 p128(prod256.limbs[0], prod256.limbs[1]);
        auto divres = divide128(p128, uscale, rounding != Rounding::toward_zero);
        wide::uint128 limit = wide::uint128::max() >> 1;
        if (neg) limit = limit + wide::uint128(1ULL);
        auto rounded = round_magnitude(divres.quotient, divres.remainder, uscale, neg, rounding, limit);
        if (!rounded) return std::unexpected(rounded.error());
        if (neg) {
            wide::int128 s(rounded->low, rounded->high);
            return -s;
        }
        return wide::int128(rounded->low, rounded->high);
    }

    auto divres = divide_narrow(prod256, uscale);
    if (!divres) return std::unexpected(divres.error());
    wide::uint128 limit = wide::uint128::max() >> 1;
    if (neg) limit = limit + wide::uint128(1ULL);
    auto rounded = round_magnitude(divres->quotient, divres->remainder, uscale, neg, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());
    if (neg) {
        wide::int128 s(rounded->low, rounded->high);
        return -s;
    }
    return wide::int128(rounded->low, rounded->high);
}

std::expected<wide::int128, ArithmeticError>
div128_impl(wide::int128 a, wide::int128 b, unsigned decimals, Rounding rounding) noexcept {
    if (b.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    wide::int128 scale = (decimals < 19) ? wide::int128(pow10(decimals), 0ULL)
                                         : pow10_wide<wide::int128>(decimals);
    auto uscale = wide::uint128(scale.low, scale.high);

#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    wide::uint128 bound = (decimals < 19) ? wide::uint128((((~static_cast<unsigned __int128>(0)) >> 1) / pow10(decimals)))
                                         : divide128(wide::uint128::max() >> 1, uscale).quotient;
    auto ua = magnitude(a);
    if (common_rounding(rounding) && ua <= bound) {
        __int128 scaled_a = static_cast<__int128>(a) * static_cast<__int128>(scale);
        if (rounding == Rounding::toward_zero) {
            return wide::int128(scaled_a / static_cast<__int128>(b));
        }
        return divide_native(wide::int128(scaled_a), b, rounding);
    }
#endif

    bool neg = a.is_negative() != b.is_negative();
    auto ma = magnitude(a);
    auto mb = magnitude(b);

    // Numerator = ma * uscale (up to 256 bits)
#if defined(__SIZEOF_INT128__) && defined(__clang__)
    using u256_internal = unsigned _BitInt(256);
    unsigned __int128 ma_128 = (static_cast<unsigned __int128>(ma.high) << 64) | ma.low;
    u256_internal num_256 = (uscale.high == 0)
        ? (static_cast<u256_internal>(ma_128) * uscale.low)
        : (static_cast<u256_internal>(ma_128) * ((static_cast<unsigned __int128>(uscale.high) << 64) | uscale.low));
    wide::uint256 num(static_cast<std::uint64_t>(num_256),
                      static_cast<std::uint64_t>(num_256 >> 64),
                      static_cast<std::uint64_t>(num_256 >> 128),
                      static_cast<std::uint64_t>(num_256 >> 192));
#else
    wide::uint256 num = multiply128(ma, uscale);
#endif
    if (num.limbs[2] == 0 && num.limbs[3] == 0) {
        wide::uint128 n128(num.limbs[0], num.limbs[1]);
        auto divres = divide128(n128, mb, rounding != Rounding::toward_zero);
        wide::uint128 limit = wide::uint128::max() >> 1;
        if (neg) limit = limit + wide::uint128(1ULL);
        auto rounded = round_magnitude(divres.quotient, divres.remainder, mb, neg, rounding, limit);
        if (!rounded) return std::unexpected(rounded.error());
        if (neg) {
            wide::uint128 neg_r = ~(*rounded) + wide::uint128(1ULL);
            return wide::int128(neg_r.low, neg_r.high);
        }
        return wide::int128(rounded->low, rounded->high);
    }

    auto divres = divide_narrow(num, mb);
    if (!divres) return std::unexpected(divres.error());
    wide::uint128 limit = wide::uint128::max() >> 1;
    if (neg) limit = limit + wide::uint128(1ULL);
    auto rounded = round_magnitude(divres->quotient, divres->remainder, mb, neg, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());
    if (neg) {
        wide::uint128 neg_r = ~(*rounded) + wide::uint128(1ULL);
        return wide::int128(neg_r.low, neg_r.high);
    }
    return wide::int128(rounded->low, rounded->high);
}

std::expected<wide::int128, ArithmeticError>
mul_div128_impl(wide::int128 a, wide::int128 b, wide::int128 c, Rounding rounding) noexcept {
    if (c.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);

#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    if (common_rounding(rounding) && fits64(a) && fits64(b)) {
        std::int64_t hi;
        std::uint64_t lo;
        imul64x64(static_cast<std::int64_t>(a.low), static_cast<std::int64_t>(b.low), hi, lo);
        wide::int128 prod(lo, static_cast<std::uint64_t>(hi));
        return divide_native(prod, c, rounding);
    }
#endif

    bool neg = a.is_negative() != (b.is_negative() != c.is_negative());
    auto ma = magnitude(a);
    auto mb = magnitude(b);
    auto mc = magnitude(c);
#if defined(__SIZEOF_INT128__) && defined(__clang__)
    using u256_internal = unsigned _BitInt(256);
    unsigned __int128 ma_128 = (static_cast<unsigned __int128>(ma.high) << 64) | ma.low;
    unsigned __int128 mb_128 = (static_cast<unsigned __int128>(mb.high) << 64) | mb.low;
    u256_internal p256 = static_cast<u256_internal>(ma_128) * static_cast<u256_internal>(mb_128);
    wide::uint256 prod256(static_cast<std::uint64_t>(p256),
                          static_cast<std::uint64_t>(p256 >> 64),
                          static_cast<std::uint64_t>(p256 >> 128),
                          static_cast<std::uint64_t>(p256 >> 192));
#else
    auto prod256 = multiply128(ma, mb);
#endif

    if (prod256.limbs[2] == 0 && prod256.limbs[3] == 0) {
        wide::uint128 p128(prod256.limbs[0], prod256.limbs[1]);
        auto divres = divide128(p128, mc, rounding != Rounding::toward_zero);
        wide::uint128 limit = wide::uint128::max() >> 1;
        if (neg) limit = limit + wide::uint128(1ULL);
        auto rounded = round_magnitude(divres.quotient, divres.remainder, mc, neg, rounding, limit);
        if (!rounded) return std::unexpected(rounded.error());
        if (neg) {
            wide::int128 s(rounded->low, rounded->high);
            return -s;
        }
        return wide::int128(rounded->low, rounded->high);
    }

    auto divres = divide_narrow(prod256, mc);
    if (!divres) return std::unexpected(divres.error());
    wide::uint128 limit = wide::uint128::max() >> 1;
    if (neg) limit = limit + wide::uint128(1ULL);
    auto rounded = round_magnitude(divres->quotient, divres->remainder, mc, neg, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());
    if (neg) {
        wide::int128 s(rounded->low, rounded->high);
        return -s;
    }
    return wide::int128(rounded->low, rounded->high);
}

#endif // FIXEDWIDE_HAS_NATIVE_128

std::expected<wide::int128, ArithmeticError>
quantize128_impl(wide::int128 a, unsigned current_dec, unsigned target_dec, Rounding rounding) noexcept {
    if (target_dec > current_dec) return std::unexpected(ArithmeticError::invalid_precision);
    if (target_dec == current_dec) return a;
    unsigned diff = current_dec - target_dec;
    bool neg = a.is_negative();
    auto mag = magnitude(a);
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    if (diff < 19) {
        std::uint64_t udiv = pow10(diff);
        unsigned __int128 ma = (static_cast<unsigned __int128>(mag.high) << 64) | mag.low;
        unsigned __int128 q = ma / udiv;
        unsigned __int128 r = ma % udiv;
        constexpr unsigned __int128 limit = (~static_cast<unsigned __int128>(0)) >> 1;
        unsigned __int128 lim = limit + (neg ? 1 : 0);
        if (r != 0) {
            if (rounding == Rounding::exact) return std::unexpected(ArithmeticError::inexact);
            bool inc = false;
            if (rounding == Rounding::nearest_even) {
                inc = detail::nearest_even_increment(q, r, static_cast<unsigned __int128>(udiv));
            } else if (rounding == Rounding::floor) {
                inc = neg;
            } else if (rounding == Rounding::ceil) {
                inc = !neg;
            } else if (rounding == Rounding::nearest_away) {
                inc = (r * 2 >= udiv);
            }
            if (inc) {
                if (q == lim) return std::unexpected(ArithmeticError::overflow);
                q += 1;
            }
        }
        unsigned __int128 res_u;
        if (__builtin_mul_overflow(q, static_cast<unsigned __int128>(udiv), &res_u) || res_u > lim) {
            return std::unexpected(ArithmeticError::overflow);
        }
        if (neg) {
            res_u = static_cast<unsigned __int128>(0) - res_u;
        }
        return wide::int128(static_cast<std::uint64_t>(res_u), static_cast<std::uint64_t>(res_u >> 64));
    }
#endif
    auto divisor = pow10_wide<wide::int128>(diff);
    auto udiv = wide::uint128(divisor.low, divisor.high);
    auto divres = divide128(mag, udiv, rounding != Rounding::toward_zero);
    wide::uint128 limit = wide::uint128::max() >> 1;
    if (neg) limit = limit + wide::uint128(1ULL);
    auto rounded = round_magnitude(divres.quotient, divres.remainder, udiv, neg, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());
    // Detect rescaling overflow from the product we already need, instead of a
    // second runtime division by the same divisor.
    auto res_mag = multiply128(*rounded, udiv);
    if (res_mag.limbs[2] != 0 || res_mag.limbs[3] != 0) return std::unexpected(ArithmeticError::overflow);
    wide::uint128 r128(res_mag.limbs[0], res_mag.limbs[1]);
    if (r128 > limit) return std::unexpected(ArithmeticError::overflow);
    if (neg) {
        wide::uint128 neg_r = ~r128 + wide::uint128(1ULL);
        return wide::int128(neg_r.low, neg_r.high);
    }
    return wide::int128(r128.low, r128.high);
}

std::expected<wide::int128, ArithmeticError>
remainder128_impl(wide::int128 a, wide::int128 b) noexcept {
    if (b.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    if (b == wide::int128(-1)) return wide::int128(0);
    bool neg_num = a.is_negative();
    auto ma = magnitude(a);
    auto mb = magnitude(b);
    auto divres = divide128(ma, mb, true);
    if (neg_num) {
        wide::int128 r(divres.remainder.low, divres.remainder.high);
        return -r;
    }
    return wide::int128(divres.remainder.low, divres.remainder.high);
}

// 256-bit operations using u512_limbs
std::expected<wide::int256, ArithmeticError>
mul256_impl(wide::int256 a, wide::int256 b, unsigned decimals, Rounding rounding) noexcept {
    auto scale = pow10_wide<wide::int256>(decimals);
    bool neg = a.is_negative() != b.is_negative();
    u256_limbs ma(magnitude(a));
    u256_limbs mb(magnitude(b));
    u512_limbs prod = mul_full(ma, mb);
    u256_limbs s(magnitude(scale));

    auto divres = divmod_knuth(prod, s);
    // Check overflow: quotient must fit 256 bits
    for (int i = 4; i < 8; ++i) {
        if (divres.quotient.limbs[i] != 0) return std::unexpected(ArithmeticError::overflow);
    }
    u256_limbs q(divres.quotient.limbs[0]);
    for (int i = 0; i < 4; ++i) q.limbs[i] = divres.quotient.limbs[i];
    u256_limbs r = divres.remainder;

    u256_limbs limit(wide::uint256::max() >> 1);
    if (neg) limit = limit + u256_limbs(1ULL);

    auto rounded = round_magnitude(q, r, s, neg, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());
    wide::uint256 uq = rounded->to_uint256();
    if (neg) {
        wide::int256 res(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
        return -res;
    }
    return wide::int256(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
}

std::expected<wide::int256, ArithmeticError>
div256_impl(wide::int256 a, wide::int256 b, unsigned decimals, Rounding rounding) noexcept {
    if (b.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    auto scale = pow10_wide<wide::int256>(decimals);
    bool neg = a.is_negative() != b.is_negative();
    u256_limbs ma(magnitude(a));
    u256_limbs mb(magnitude(b));
    u256_limbs s(magnitude(scale));
    u512_limbs num = mul_full(ma, s);

    auto divres = divmod_knuth(num, mb);
    for (int i = 4; i < 8; ++i) {
        if (divres.quotient.limbs[i] != 0) return std::unexpected(ArithmeticError::overflow);
    }
    u256_limbs q;
    for (int i = 0; i < 4; ++i) q.limbs[i] = divres.quotient.limbs[i];
    u256_limbs r = divres.remainder;

    u256_limbs limit(wide::uint256::max() >> 1);
    if (neg) limit = limit + u256_limbs(1ULL);

    auto rounded = round_magnitude(q, r, mb, neg, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());
    wide::uint256 uq = rounded->to_uint256();
    if (neg) {
        wide::int256 res(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
        return -res;
    }
    return wide::int256(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
}

std::expected<wide::int256, ArithmeticError>
mul_div256_impl(wide::int256 a, wide::int256 b, wide::int256 c, Rounding rounding) noexcept {
    if (c.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    bool neg = a.is_negative() != (b.is_negative() != c.is_negative());
    u256_limbs ma(magnitude(a));
    u256_limbs mb(magnitude(b));
    u256_limbs mc(magnitude(c));
    u512_limbs prod = mul_full(ma, mb);

    auto divres = divmod_knuth(prod, mc);
    for (int i = 4; i < 8; ++i) {
        if (divres.quotient.limbs[i] != 0) return std::unexpected(ArithmeticError::overflow);
    }
    u256_limbs q;
    for (int i = 0; i < 4; ++i) q.limbs[i] = divres.quotient.limbs[i];
    u256_limbs r = divres.remainder;

    u256_limbs limit(wide::uint256::max() >> 1);
    if (neg) limit = limit + u256_limbs(1ULL);

    auto rounded = round_magnitude(q, r, mc, neg, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());
    wide::uint256 uq = rounded->to_uint256();
    if (neg) {
        wide::int256 res(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
        return -res;
    }
    return wide::int256(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
}

std::expected<wide::int256, ArithmeticError>
quantize256_impl(wide::int256 a, unsigned current_dec, unsigned target_dec, Rounding rounding) noexcept {
    if (target_dec > current_dec) return std::unexpected(ArithmeticError::invalid_precision);
    if (target_dec == current_dec) return a;
    auto divisor = pow10_wide<wide::int256>(current_dec - target_dec);
    u256_limbs udiv(magnitude(divisor));
    bool neg = a.is_negative();
    u256_limbs mag(magnitude(a));
    auto divres = divmod_knuth(mag, udiv);
    u256_limbs limit(wide::uint256::max() >> 1);
    if (neg) limit = limit + u256_limbs(1ULL);
    auto rounded = round_magnitude(divres.quotient, divres.remainder, udiv, neg, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());
    // Multiply rounded * udiv
    auto res_full = mul_full(*rounded, udiv);
    for (int i = 4; i < 8; ++i) {
        if (res_full.limbs[i] != 0) return std::unexpected(ArithmeticError::overflow);
    }
    u256_limbs res_mag;
    for (int i = 0; i < 4; ++i) res_mag.limbs[i] = res_full.limbs[i];
    if (res_mag > limit) return std::unexpected(ArithmeticError::overflow);
    wide::uint256 uq = res_mag.to_uint256();
    if (neg) {
        wide::int256 s(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
        return -s;
    }
    return wide::int256(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
}

std::expected<wide::int256, ArithmeticError>
remainder256_impl(wide::int256 a, wide::int256 b) noexcept {
    if (b.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    if (b == wide::int256(-1)) return wide::int256(0);
    bool neg_num = a.is_negative();
    u256_limbs ma(magnitude(a));
    u256_limbs mb(magnitude(b));
    auto divres = divmod_knuth(ma, mb);
    wide::uint256 ur = divres.remainder.to_uint256();
    if (neg_num) {
        wide::int256 r(ur.limbs[0], ur.limbs[1], ur.limbs[2], ur.limbs[3]);
        return -r;
    }
    return wide::int256(ur.limbs[0], ur.limbs[1], ur.limbs[2], ur.limbs[3]);
}

} // namespace fixedwide::detail
