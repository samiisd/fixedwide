#pragma once

/// \file
/// Checked arithmetic on one fixed-point type. Every operation returns
/// `std::expected` and reports overflow, division by zero and refused rounding
/// instead of producing a wrong answer. Nothing here allocates or throws, and
/// all of it is usable in a constant expression.

#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <fixedwide/detail/overflow.hpp>
#include <fixedwide/detail/constexpr_arith.hpp>
#include <expected>
#include <type_traits>
// <concepts> and <limits> are not included: `std::same_as` and `std::integral`
// are one line each over <type_traits>, and the two raw bounds this header
// needs are a shift. Together those two standard headers cost about 12 ms of
// parse time in every translation unit -- the same trade wide.hpp already made.

namespace fixedwide {

namespace detail {

// Forward declarations of wide compiled kernels
// Scale-specialised kernels. D is a compile-time constant at every call site,
// so these are declared here and explicitly instantiated in arithmetic.cpp: a
// consumer emits one ordinary call to a symbol whose body already knows its
// scale -- no dispatch switch, no extra call level, and no implementation in the
// consumer's translation unit.
//
// The cap is per width because 10^D must fit the storage. basic_fixed already
// refuses to exceed it and the kernels static_assert it, so a scale that would
// wrap is a compile error rather than a wrong constant.
inline constexpr unsigned max_scaled_decimals_64 = 18;
inline constexpr unsigned max_scaled_decimals_128 = 19;

template<unsigned D>
std::expected<basic_fixed<128, D>, ArithmeticError>
mul128_scaled(wide::int128 a, wide::int128 b, Rounding rounding) noexcept;

template<unsigned D>
std::expected<basic_fixed<128, D>, ArithmeticError>
div128_scaled(wide::int128 a, wide::int128 b, Rounding rounding) noexcept;

template<unsigned D>
std::expected<std::int64_t, ArithmeticError>
mul64_scaled(std::int64_t a, std::int64_t b, Rounding rounding) noexcept;

template<unsigned D>
std::expected<std::int64_t, ArithmeticError>
div64_scaled(std::int64_t a, std::int64_t b, Rounding rounding) noexcept;

std::expected<wide::int128, ArithmeticError>
mul128_kernel(wide::int128 a, wide::int128 b, unsigned decimals, Rounding rounding) noexcept;

std::expected<wide::int128, ArithmeticError>
div128_kernel(wide::int128 a, wide::int128 b, unsigned decimals, Rounding rounding) noexcept;

// The divisor arrives as its two limbs, not as a wide::int128. Three 16-byte
// operands plus the returned std::expected do not fit the argument registers,
// so the third one is passed in memory -- and Clang materialises a struct
// argument in a temporary, then copies it to the outgoing slot with a 16-byte
// move over two 8-byte stores. That copy does not forward, and the stall cost
// more than the division. Scalar limbs are stored straight into the slot.
std::expected<wide::int128, ArithmeticError>
mul_div128_kernel(wide::int128 a, wide::int128 b, std::uint64_t c_low, std::uint64_t c_high,
                Rounding rounding) noexcept;

std::expected<wide::int256, ArithmeticError>
mul256_kernel(const wide::int256& a, const wide::int256& b, unsigned decimals, Rounding rounding) noexcept;

std::expected<wide::int256, ArithmeticError>
div256_kernel(const wide::int256& a, const wide::int256& b, unsigned decimals, Rounding rounding) noexcept;

std::expected<wide::int256, ArithmeticError>
mul_div256_kernel(const wide::int256& a, const wide::int256& b, const wide::int256& c, Rounding rounding) noexcept;

std::expected<wide::int128, ArithmeticError>
quantize128_kernel(wide::int128 a, unsigned current_decimals, unsigned target_decimals, Rounding rounding) noexcept;

std::expected<wide::int256, ArithmeticError>
quantize256_kernel(const wide::int256& a, unsigned current_decimals, unsigned target_decimals, Rounding rounding) noexcept;

std::expected<wide::int128, ArithmeticError>
remainder128_kernel(wide::int128 a, wide::int128 b) noexcept;

std::expected<wide::int256, ArithmeticError>
remainder256_kernel(const wide::int256& a, const wide::int256& b) noexcept;

std::expected<std::int64_t, ArithmeticError>
mul64_kernel(std::int64_t a, std::int64_t b, std::int64_t scale, Rounding rounding) noexcept;

std::expected<std::int64_t, ArithmeticError>
div64_kernel(std::int64_t a, std::int64_t b, std::int64_t scale, Rounding rounding) noexcept;

std::expected<std::int64_t, ArithmeticError>
mul_div64_kernel(std::int64_t a, std::int64_t b, std::int64_t c, Rounding rounding) noexcept;

std::expected<std::int64_t, ArithmeticError>
quantize64_kernel(std::int64_t a, unsigned current_decimals, unsigned target_decimals, Rounding rounding) noexcept;


// Overflow outranks inexact: a result that cannot be represented is an overflow
// whichever rounding mode was requested. The 64- and 128-bit kernels range-check
// before they round and report it directly, but the narrow widths range-check
// after the kernel returns, so `Rounding::exact` would otherwise surface
// `inexact` for a value that also overflows -- the same logical operation giving
// a different error at Fixed32<4> than at Fixed64<12>.
//
// Re-running the truncating operation tells the two apart, and only ever on the
// error path.
template<typename Fixed, typename Truncating>
[[nodiscard]] constexpr std::expected<Fixed, ArithmeticError>
narrow_checked(std::expected<std::int64_t, ArithmeticError> result, Truncating truncating) noexcept {
    using Raw = typename Fixed::raw_type;
    constexpr int bits = static_cast<int>(sizeof(Raw)) * 8;
    constexpr std::int64_t high = (std::int64_t{1} << (bits - 1)) - 1;
    constexpr std::int64_t low = -high - 1;
    if (!result) {
        if (result.error() == ArithmeticError::inexact) {
            const auto truncated = truncating();
            if (truncated && (*truncated < low || *truncated > high)) {
                return std::unexpected(ArithmeticError::overflow);
            }
        }
        return std::unexpected(result.error());
    }
    if (*result < low || *result > high) return std::unexpected(ArithmeticError::overflow);
    return Fixed::from_raw(static_cast<Raw>(*result));
}

} // namespace detail

/// Build a fixed-point value from a whole number, checked.
///
/// `from_integer<Fixed64<2>>(19)` is 19.00. The integer is scaled by
/// `Target::scale()`, so a value that would not fit is reported rather than
/// wrapped. Accepts every built-in integer type and `__int128` where available.
///
/// \tparam Target the fixed-point type to build.
/// \return the value, or `ArithmeticError::overflow`.
// There is deliberately no `to_integer`: going the
// other way has to choose a rounding, which is `quantize(value, 0)` followed by
// `.raw()`, or `fixed_cast<FixedN<0>>(value, rounding)` across widths. Naming a
// third spelling for one of those would hide the rounding decision.
template<typename Target, typename Integer>
    requires (std::is_integral_v<Integer>
#if defined(__SIZEOF_INT128__)
              || std::is_same_v<Integer, __int128> || std::is_same_v<Integer, unsigned __int128>
#endif
             )
[[nodiscard]] constexpr std::expected<Target, ArithmeticError> from_integer(Integer value) noexcept {
    bool negative = false;
    wide::uint256 mag{};
    if constexpr (std::is_signed_v<Integer>
#if defined(__SIZEOF_INT128__)
                  || std::is_same_v<Integer, __int128>
#endif
                 ) {
        if (value < 0) {
            negative = true;
#if defined(__SIZEOF_INT128__)
            if constexpr (std::is_same_v<Integer, __int128>) {
                unsigned __int128 u = 0 - static_cast<unsigned __int128>(value);
                mag = wide::uint256(u);
            } else
#endif
            {
                using unsigned_t = std::make_unsigned_t<Integer>;
                unsigned_t u = static_cast<unsigned_t>(0ULL - static_cast<unsigned_t>(value));
                mag = wide::uint256(u);
            }
        } else {
#if defined(__SIZEOF_INT128__)
            if constexpr (std::is_same_v<Integer, __int128>) {
                mag = wide::uint256(static_cast<unsigned __int128>(value));
            } else
#endif
            {
                using unsigned_t = std::make_unsigned_t<Integer>;
                mag = wide::uint256(static_cast<unsigned_t>(value));
            }
        }
    } else {
#if defined(__SIZEOF_INT128__)
        if constexpr (std::is_same_v<Integer, unsigned __int128>) {
            mag = wide::uint256(value);
        } else
#endif
        {
            mag = wide::uint256(value);
        }
    }

    const auto limit = detail::limit_magnitude_u256<Target::bits>(negative);

    if constexpr (Target::fractional_digits == 0) {
        if (mag > limit) return std::unexpected(ArithmeticError::overflow);
        if constexpr (Target::bits <= 64) {
            std::uint64_t u = mag.limbs[0];
            if (negative) {
                std::int64_t s = 0ULL - u;
                return Target::from_raw(static_cast<typename Target::raw_type>(s));
            } else {
                return Target::from_raw(static_cast<typename Target::raw_type>(u));
            }
        } else if constexpr (Target::bits == 128) {
            wide::uint128 u(mag.limbs[0], mag.limbs[1]);
            if (negative) {
                wide::uint128 neg_u = ~u + wide::uint128(1ULL, 0ULL);
                return Target::from_raw(wide::int128(neg_u.low, neg_u.high));
            } else {
                return Target::from_raw(wide::int128(u.low, u.high));
            }
        } else {
            if (negative) {
                wide::uint256 neg_u = ~mag + wide::uint256(1ULL);
                return Target::from_raw(wide::int256(neg_u.limbs[0], neg_u.limbs[1], neg_u.limbs[2], neg_u.limbs[3]));
            } else {
                return Target::from_raw(wide::int256(mag.limbs[0], mag.limbs[1], mag.limbs[2], mag.limbs[3]));
            }
        }
    } else {
        // Target::fractional_digits > 0
        wide::uint256 max_allowed = detail::max_integer_allowed<Target::bits, Target::fractional_digits>(negative);
        if (mag > max_allowed) return std::unexpected(ArithmeticError::overflow);

        // Not named `scale`: that would shadow fixedwide::scale, the 0.4
        // compatibility constant, in every consumer that includes this header.
        wide::uint256 target_scale = detail::to_uint256_raw(Target::scale());
        wide::uint256 scaled = mag * target_scale;
        if (scaled > limit) return std::unexpected(ArithmeticError::overflow);

        if constexpr (Target::bits <= 64) {
            std::uint64_t u = scaled.limbs[0];
            if (negative) {
                std::int64_t s = 0ULL - u;
                return Target::from_raw(static_cast<typename Target::raw_type>(s));
            } else {
                return Target::from_raw(static_cast<typename Target::raw_type>(u));
            }
        } else if constexpr (Target::bits == 128) {
            wide::uint128 u(scaled.limbs[0], scaled.limbs[1]);
            if (negative) {
                wide::uint128 neg_u = ~u + wide::uint128(1ULL, 0ULL);
                return Target::from_raw(wide::int128(neg_u.low, neg_u.high));
            } else {
                return Target::from_raw(wide::int128(u.low, u.high));
            }
        } else {
            if (negative) {
                wide::uint256 neg_u = ~scaled + wide::uint256(1ULL);
                return Target::from_raw(wide::int256(neg_u.limbs[0], neg_u.limbs[1], neg_u.limbs[2], neg_u.limbs[3]));
            } else {
                return Target::from_raw(wide::int256(scaled.limbs[0], scaled.limbs[1], scaled.limbs[2], scaled.limbs[3]));
            }
        }
    }
}


namespace detail_arith {
template<class UQ, class UR, class UD>
[[nodiscard]] constexpr bool nearest_even_inc(UQ q, UR r, UD d) noexcept {
    const auto tie = q & ~d & 1;
    return r > d / 2 - tie;
}
// A 128-bit value is representable in signed 64 bits exactly when its high limb
// is the sign extension of the low one. Written as a sum rather than the two
// paired equality tests it replaces: those cost a branch each, on the fast path
// of every wide multiply.
[[nodiscard]] constexpr bool fits64(wide::int128 v) noexcept {
    return v.high + (v.low >> 63) == 0;
}
[[nodiscard]] inline std::int64_t nearest_adj(std::int64_t q, std::int64_t rem, std::int64_t div, bool neg) noexcept {
    const auto rbits = static_cast<std::uint64_t>(rem);
    const auto dbits = static_cast<std::uint64_t>(div);
    const auto r = rem < 0 ? 0ULL - rbits : rbits;
    const auto d = div < 0 ? 0ULL - dbits : dbits;
    const bool inc = nearest_even_inc(static_cast<std::uint64_t>(q), r, d);
    const std::int64_t dir = neg ? -1 : 1;
    return dir & -static_cast<std::int64_t>(inc);
}
#if defined(FIXEDWIDE_HAS_X86_64_ASM)
inline std::uint64_t div128by64_asm(std::uint64_t high, std::uint64_t low, std::uint64_t divisor, std::uint64_t& remainder) noexcept {
    std::uint64_t quotient;
    __asm__("divq %[divisor]" : "=a"(quotient), "=d"(remainder)
            : "a"(low), "d"(high), [divisor] "r"(divisor) : "cc");
    return quotient;
}
#endif
inline bool round_inc_u64(std::uint64_t q_lo, std::uint64_t rem, std::uint64_t divisor, Rounding rounding, bool neg = false) noexcept {
    if (rem == 0) return false;
    if (rounding == Rounding::nearest_even) {
        return nearest_even_inc(q_lo, rem, divisor);
    } else if (rounding == Rounding::floor) {
        return neg;
    } else if (rounding == Rounding::ceil) {
        return !neg;
    } else if (rounding == Rounding::nearest_away) {
        return (rem * 2 >= divisor);
    }
    return false;
}
}

/// `a + b`, exactly. Both operands and the result share one type, so no
/// rounding is possible and the only failure is overflow.
/// \return the sum, or `ArithmeticError::overflow`.
/// \see add_to for operands of different widths or scales.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr std::expected<basic_fixed<Bits, D>, ArithmeticError>
add(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if constexpr (Bits == 8) {
        std::int16_t res = static_cast<std::int16_t>(a.raw()) + static_cast<std::int16_t>(b.raw());
        if (res < INT8_MIN || res > INT8_MAX) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(static_cast<std::int8_t>(res));
    } else if constexpr (Bits == 16) {
        std::int32_t res = static_cast<std::int32_t>(a.raw()) + static_cast<std::int32_t>(b.raw());
        if (res < INT16_MIN || res > INT16_MAX) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(static_cast<std::int16_t>(res));
    } else if constexpr (Bits == 32) {
        std::int32_t res;
        if (detail::add_overflow(a.raw(), b.raw(), &res)) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(res);
    } else if constexpr (Bits == 64) {
        std::int64_t res;
        if (detail::add_overflow(a.raw(), b.raw(), &res)) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(res);
    } else if constexpr (Bits == 128) {
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
        __int128 res;
        if (detail::add_overflow(static_cast<__int128>(a.raw()), static_cast<__int128>(b.raw()), &res)) {
            return std::unexpected(ArithmeticError::overflow);
        }
        return Fixed::from_raw(wide::int128(res));
#else
        wide::int128 res = a.raw() + b.raw();
        bool a_neg = a.raw().is_negative();
        bool b_neg = b.raw().is_negative();
        bool res_neg = res.is_negative();
        if ((a_neg == b_neg) && (a_neg != res_neg)) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(res);
#endif
    } else if constexpr (Bits == 256) {
        wide::int256 res = a.raw() + b.raw();
        bool a_neg = a.raw().is_negative();
        bool b_neg = b.raw().is_negative();
        bool res_neg = res.is_negative();
        if ((a_neg == b_neg) && (a_neg != res_neg)) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(res);
    }
}

/// `a - b`, exactly. No rounding is possible; the only failure is overflow.
/// \return the difference, or `ArithmeticError::overflow`.
/// \see sub_to for operands of different widths or scales.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr std::expected<basic_fixed<Bits, D>, ArithmeticError>
sub(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if constexpr (Bits == 8) {
        std::int16_t res = static_cast<std::int16_t>(a.raw()) - static_cast<std::int16_t>(b.raw());
        if (res < INT8_MIN || res > INT8_MAX) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(static_cast<std::int8_t>(res));
    } else if constexpr (Bits == 16) {
        std::int32_t res = static_cast<std::int32_t>(a.raw()) - static_cast<std::int32_t>(b.raw());
        if (res < INT16_MIN || res > INT16_MAX) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(static_cast<std::int16_t>(res));
    } else if constexpr (Bits == 32) {
        std::int32_t res;
        if (detail::sub_overflow(a.raw(), b.raw(), &res)) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(res);
    } else if constexpr (Bits == 64) {
        std::int64_t res;
        if (detail::sub_overflow(a.raw(), b.raw(), &res)) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(res);
    } else if constexpr (Bits == 128) {
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
        __int128 res;
        if (detail::sub_overflow(static_cast<__int128>(a.raw()), static_cast<__int128>(b.raw()), &res)) {
            return std::unexpected(ArithmeticError::overflow);
        }
        return Fixed::from_raw(wide::int128(res));
#else
        wide::int128 res = a.raw() - b.raw();
        bool a_neg = a.raw().is_negative();
        bool b_neg = b.raw().is_negative();
        bool res_neg = res.is_negative();
        if ((a_neg != b_neg) && (a_neg != res_neg)) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(res);
#endif
    } else if constexpr (Bits == 256) {
        wide::int256 res = a.raw() - b.raw();
        bool a_neg = a.raw().is_negative();
        bool b_neg = b.raw().is_negative();
        bool res_neg = res.is_negative();
        if ((a_neg != b_neg) && (a_neg != res_neg)) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(res);
    }
}

/// `-a`. Fails only for `min()`, whose negation is one past `max()`.
/// \return the negated value, or `ArithmeticError::overflow`.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr std::expected<basic_fixed<Bits, D>, ArithmeticError>
negate(basic_fixed<Bits, D> a) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if (a == Fixed::min()) return std::unexpected(ArithmeticError::overflow);
    return sub(Fixed{}, a);
}

/// `|a|`. Fails only for `min()`, whose magnitude is one past `max()`.
/// \return the magnitude, or `ArithmeticError::overflow`.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr std::expected<basic_fixed<Bits, D>, ArithmeticError>
abs(basic_fixed<Bits, D> a) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if constexpr (Bits <= 64) {
        if (a.raw() < 0) return negate(a);
        return a;
    } else {
        if (a.raw().is_negative()) return negate(a);
        return a;
    }
}

/// `a * b`, rounded once to the shared scale.
///
/// The product is formed at twice the width before it is rescaled, so no
/// intermediate precision is lost: `mul` on a `Fixed64<12>` multiplies into 128
/// bits and divides by 10^12 exactly once.
///
/// \param rounding how to resolve the single rounding. `Rounding::exact`
///                 returns `ArithmeticError::inexact` rather than round.
/// \return the product, or `ArithmeticError::overflow` / `inexact`.
/// \see mul_to for operands of different widths or scales, and mul_div to
///      multiply and divide with only one rounding in between.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr std::expected<basic_fixed<Bits, D>, ArithmeticError>
mul(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b, Rounding rounding = Rounding::nearest_even) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    // The runtime kernels are compiled out of line and use inline assembly, so
    // they cannot run in a constant expression. detail::ce is a separate,
    // deliberately simple implementation of the same contract that can, and the
    // two are differential-tested against each other.
    if consteval {
        auto raw = detail::ce::mul<Bits, D>(a.raw(), b.raw(), rounding);
        if (!raw) return std::unexpected(raw.error());
        return Fixed::from_raw(*raw);
    }
    if constexpr (Bits == 8) {
        std::int32_t prod = static_cast<std::int32_t>(a.raw()) * static_cast<std::int32_t>(b.raw());
        return detail::narrow_checked<Fixed>(
            detail::mul64_kernel(prod, 1, Fixed::scale(), rounding),
            [&] { return detail::mul64_kernel(prod, 1, Fixed::scale(), Rounding::toward_zero); });
    } else if constexpr (Bits == 16) {
        std::int64_t prod = static_cast<std::int64_t>(a.raw()) * static_cast<std::int64_t>(b.raw());
        return detail::narrow_checked<Fixed>(
            detail::mul64_kernel(prod, 1, Fixed::scale(), rounding),
            [&] { return detail::mul64_kernel(prod, 1, Fixed::scale(), Rounding::toward_zero); });
    } else if constexpr (Bits == 32) {
        std::int64_t prod = static_cast<std::int64_t>(a.raw()) * static_cast<std::int64_t>(b.raw());
        return detail::narrow_checked<Fixed>(
            detail::mul64_kernel(prod, 1, Fixed::scale(), rounding),
            [&] { return detail::mul64_kernel(prod, 1, Fixed::scale(), Rounding::toward_zero); });
    } else if constexpr (Bits == 64) {
#if defined(FIXEDWIDE_HAS_X86_64_ASM)
        if (rounding == Rounding::toward_zero || rounding == Rounding::nearest_even) {
            std::int64_t scale_val = Fixed::scale();
            std::uint64_t lo;
            std::int64_t hi;
            __asm__("imulq %[rhs]" : "=a"(lo), "=d"(hi) : "a"(a.raw()), [rhs] "r"(b.raw()) : "cc");
            std::uint64_t half = static_cast<std::uint64_t>(scale_val) / 2;
            std::uint64_t uhi = static_cast<std::uint64_t>(hi);
            if (uhi + half < 2 * half) {
                std::int64_t q, r;
                __asm__("idivq %[div]" : "=a"(q), "=d"(r) : "a"(lo), "d"(hi), [div] "r"(scale_val) : "cc");
                if (rounding == Rounding::nearest_even && r != 0) {
                    q += detail_arith::nearest_adj(q, r, scale_val, hi < 0);
                }
                return Fixed::from_raw(q);
            }
        }
#endif
        auto res = detail::mul64_scaled<D>(a.raw(), b.raw(), rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    } else if constexpr (Bits == 128) {
#if defined(FIXEDWIDE_HAS_X86_64_ASM)
        if (rounding == Rounding::toward_zero || rounding == Rounding::nearest_even) {
            auto alow = static_cast<std::int64_t>(a.raw().low);
            auto blow = static_cast<std::int64_t>(b.raw().low);
            if (detail_arith::fits64(a.raw()) && detail_arith::fits64(b.raw()) &&
                Fixed::scale().high == 0) {
                std::int64_t scale_val = static_cast<std::int64_t>(Fixed::scale().low);
                std::uint64_t lo;
                std::int64_t hi;
                __asm__("imulq %[rhs]" : "=a"(lo), "=d"(hi) : "a"(alow), [rhs] "r"(blow) : "cc");
                std::uint64_t half = static_cast<std::uint64_t>(scale_val) / 2;
                std::uint64_t uhi = static_cast<std::uint64_t>(hi);
                if (uhi + half < 2 * half) {
                    std::int64_t q, r;
                    __asm__("idivq %[div]" : "=a"(q), "=d"(r) : "a"(lo), "d"(hi), [div] "r"(scale_val) : "cc");
                    // Branchless, like the 64-bit and mul_div paths: the
                    // increment is a coin flip on real data, and a mispredict on
                    // a dependent chain costs more than the whole division.
                    if (rounding == Rounding::nearest_even && r != 0) {
                        q += detail_arith::nearest_adj(q, r, scale_val, hi < 0);
                    }
                    return Fixed::from_raw(wide::int128(q));
                } else {
                    bool neg = hi < 0;
                    std::uint64_t ulo = lo;
                    std::uint64_t u_hi = static_cast<std::uint64_t>(hi);
                    if (neg) {
                        ulo = ~ulo + 1;
                        u_hi = ~u_hi + (ulo == 0 ? 1 : 0);
                    }
                    std::uint64_t rem;
                    std::uint64_t qhi = u_hi / static_cast<std::uint64_t>(scale_val);
                    std::uint64_t rem_hi = u_hi % static_cast<std::uint64_t>(scale_val);
                    std::uint64_t qlo = detail_arith::div128by64_asm(rem_hi, ulo, static_cast<std::uint64_t>(scale_val), rem);
                    wide::uint128 quotient(qlo, qhi);
                    if (detail_arith::round_inc_u64(qlo, rem, static_cast<std::uint64_t>(scale_val), rounding, neg)) {
                        quotient = quotient + wide::uint128(1ULL);
                    }
                    if (neg) {
                        wide::uint128 neg_q = ~quotient + wide::uint128(1ULL);
                        return Fixed::from_raw(wide::int128(neg_q.low, neg_q.high));
                    }
                    return Fixed::from_raw(wide::int128(quotient.low, quotient.high));
                }
            }
        }
#endif
        if constexpr (D <= detail::max_scaled_decimals_128) {
            // Returned straight through: an identical return type lets the
            // kernel write the caller's sret buffer directly. Re-wrapping it
            // costs a 16-byte reload of two 8-byte stores, which does not
            // forward.
            return detail::mul128_scaled<D>(a.raw(), b.raw(), rounding);
        } else {
            auto res = detail::mul128_kernel(a.raw(), b.raw(), D, rounding);
            if (!res) return std::unexpected(res.error());
            return Fixed::from_raw(*res);
        }
    } else if constexpr (Bits == 256) {
        auto res = detail::mul256_kernel(a.raw(), b.raw(), D, rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    }
}

/// `a / b`, rounded once to the shared scale.
///
/// The numerator is scaled up at twice the width before the division, so the
/// quotient carries every digit the type can hold.
///
/// \param rounding how to resolve the single rounding. `Rounding::exact`
///                 returns `ArithmeticError::inexact` rather than round.
/// \return the quotient, or `ArithmeticError::division_by_zero` / `overflow`
///         / `inexact`.
/// \see div_to for operands of different widths or scales.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr std::expected<basic_fixed<Bits, D>, ArithmeticError>
div(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b, Rounding rounding = Rounding::nearest_even) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if consteval {
        auto raw = detail::ce::div<Bits, D>(a.raw(), b.raw(), rounding);
        if (!raw) return std::unexpected(raw.error());
        return Fixed::from_raw(*raw);
    }
    if constexpr (Bits <= 64) {
        if (b.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
        if constexpr (Bits == 8) {
            std::int32_t num = static_cast<std::int32_t>(a.raw()) * Fixed::scale();
            return detail::narrow_checked<Fixed>(
                detail::mul64_kernel(num, 1, b.raw(), rounding),
                [&] { return detail::mul64_kernel(num, 1, b.raw(), Rounding::toward_zero); });
        } else if constexpr (Bits == 16) {
            std::int64_t num = static_cast<std::int64_t>(a.raw()) * Fixed::scale();
            return detail::narrow_checked<Fixed>(
                detail::mul64_kernel(num, 1, b.raw(), rounding),
                [&] { return detail::mul64_kernel(num, 1, b.raw(), Rounding::toward_zero); });
        } else if constexpr (Bits == 32) {
            return detail::narrow_checked<Fixed>(
                detail::div64_scaled<D>(a.raw(), b.raw(), rounding),
                [&] { return detail::div64_scaled<D>(a.raw(), b.raw(), Rounding::toward_zero); });
        } else { // Bits == 64
#if defined(FIXEDWIDE_HAS_X86_64_ASM)
            if (rounding == Rounding::toward_zero || rounding == Rounding::nearest_even) {
                std::int64_t scale_val = Fixed::scale();
                std::uint64_t lo;
                std::int64_t hi;
                __asm__("imulq %[rhs]" : "=a"(lo), "=d"(hi) : "a"(a.raw()), [rhs] "r"(scale_val) : "cc");
                std::uint64_t mag = b.raw() < 0 ? 0ULL - static_cast<std::uint64_t>(b.raw()) : static_cast<std::uint64_t>(b.raw());
                std::uint64_t half = mag / 2;
                std::uint64_t uhi = static_cast<std::uint64_t>(hi);
                if (uhi + half < 2 * half && !(b.raw() < 0 && uhi + half == 0)) {
                    std::int64_t q, r;
                    __asm__("idivq %[div]" : "=a"(q), "=d"(r) : "a"(lo), "d"(hi), [div] "r"(b.raw()) : "cc");
                    if (rounding == Rounding::nearest_even && r != 0) {
                        q += detail_arith::nearest_adj(q, r, b.raw(), (hi < 0) != (b.raw() < 0));
                    }
                    return Fixed::from_raw(q);
                }
            }
#endif
            auto res = detail::div64_scaled<D>(a.raw(), b.raw(), rounding);
            if (!res) return std::unexpected(res.error());
            return Fixed::from_raw(*res);
        }
    } else if constexpr (Bits == 128) {
        if (b.raw().is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
        // No inline narrow path here, unlike mul and mul_div. Measured: keeping
        // one cost native_by128.FP128.div 85% against 0.4 versus 37% without it,
        // because div's inline test rejects most operands and its code sits on
        // the path of the ones it rejects. Which operations earn an inline copy
        // is decided per operation by measurement, not by symmetry.
        if constexpr (D <= detail::max_scaled_decimals_128) {
            return detail::div128_scaled<D>(a.raw(), b.raw(), rounding);
        } else {
            auto res = detail::div128_kernel(a.raw(), b.raw(), D, rounding);
            if (!res) return std::unexpected(res.error());
            return Fixed::from_raw(*res);
        }
    } else if constexpr (Bits == 256) {
        if (b.raw().is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
        auto res = detail::div256_kernel(a.raw(), b.raw(), D, rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    }
}

/// `a * b / c` with **one** rounding, not two.
///
/// `mul(a, b)` followed by `div(..., c)` rounds twice and can be off by a unit
/// in the last place; this forms the full-width product and divides it once.
/// The natural spelling for a rate applied to an amount.
///
/// \param rounding how to resolve the single rounding.
/// \return the result, or `ArithmeticError::division_by_zero` / `overflow`
///         / `inexact`.
/// \see mul_div_to when the three operands are not the same type.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr std::expected<basic_fixed<Bits, D>, ArithmeticError>
mul_div(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b, basic_fixed<Bits, D> c, Rounding rounding = Rounding::nearest_even) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if consteval {
        auto raw = detail::ce::mul_div<Bits, D>(a.raw(), b.raw(), c.raw(), rounding);
        if (!raw) return std::unexpected(raw.error());
        return Fixed::from_raw(*raw);
    }
    if constexpr (Bits <= 64) {
        if (c.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
        if constexpr (Bits == 8) {
            std::int32_t num = static_cast<std::int32_t>(a.raw()) * static_cast<std::int32_t>(b.raw());
            return detail::narrow_checked<Fixed>(
                detail::mul64_kernel(num, 1, c.raw(), rounding),
                [&] { return detail::mul64_kernel(num, 1, c.raw(), Rounding::toward_zero); });
        } else if constexpr (Bits == 16) {
            std::int64_t num = static_cast<std::int64_t>(a.raw()) * static_cast<std::int64_t>(b.raw());
            return detail::narrow_checked<Fixed>(
                detail::mul64_kernel(num, 1, c.raw(), rounding),
                [&] { return detail::mul64_kernel(num, 1, c.raw(), Rounding::toward_zero); });
        } else if constexpr (Bits == 32) {
            return detail::narrow_checked<Fixed>(
                detail::mul_div64_kernel(a.raw(), b.raw(), c.raw(), rounding),
                [&] { return detail::mul_div64_kernel(a.raw(), b.raw(), c.raw(), Rounding::toward_zero); });
        } else { // Bits == 64
#if defined(FIXEDWIDE_HAS_X86_64_ASM)
            if (rounding == Rounding::toward_zero || rounding == Rounding::nearest_even) {
                std::uint64_t lo;
                std::int64_t hi;
                __asm__("imulq %[rhs]" : "=a"(lo), "=d"(hi) : "a"(a.raw()), [rhs] "r"(b.raw()) : "cc");
                std::uint64_t mag = c.raw() < 0 ? 0ULL - static_cast<std::uint64_t>(c.raw()) : static_cast<std::uint64_t>(c.raw());
                std::uint64_t half = mag / 2;
                std::uint64_t uhi = static_cast<std::uint64_t>(hi);
                if (uhi + half < 2 * half && !(c.raw() < 0 && uhi + half == 0)) {
                    std::int64_t q, r;
                    __asm__("idivq %[div]" : "=a"(q), "=d"(r) : "a"(lo), "d"(hi), [div] "r"(c.raw()) : "cc");
                    if (rounding == Rounding::nearest_even && r != 0) {
                        q += detail_arith::nearest_adj(q, r, c.raw(), (hi < 0) != (c.raw() < 0));
                    }
                    return Fixed::from_raw(q);
                }
            }
#endif
            auto res = detail::mul_div64_kernel(a.raw(), b.raw(), c.raw(), rounding);
            if (!res) return std::unexpected(res.error());
            return Fixed::from_raw(*res);
        }
    } else if constexpr (Bits == 128) {
        if (c.raw().is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
#if defined(FIXEDWIDE_HAS_X86_64_ASM)
        if (rounding == Rounding::toward_zero || rounding == Rounding::nearest_even) {
            auto alow = static_cast<std::int64_t>(a.raw().low);
            auto blow = static_cast<std::int64_t>(b.raw().low);
            auto clow = static_cast<std::int64_t>(c.raw().low);
            if (detail_arith::fits64(a.raw()) && detail_arith::fits64(b.raw()) &&
                detail_arith::fits64(c.raw())) {
                std::uint64_t lo;
                std::int64_t hi;
                __asm__("imulq %[rhs]" : "=a"(lo), "=d"(hi) : "a"(alow), [rhs] "r"(blow) : "cc");
                std::uint64_t mag = clow < 0 ? 0ULL - static_cast<std::uint64_t>(clow) : static_cast<std::uint64_t>(clow);
                std::uint64_t half = mag / 2;
                std::uint64_t uhi = static_cast<std::uint64_t>(hi);
                if (uhi + half < 2 * half && !(clow < 0 && uhi + half == 0)) {
                    std::int64_t q, r;
                    __asm__("idivq %[div]" : "=a"(q), "=d"(r) : "a"(lo), "d"(hi), [div] "r"(clow) : "cc");
                    if (rounding == Rounding::nearest_even && r != 0) {
                        q += detail_arith::nearest_adj(q, r, clow, (hi < 0) != (clow < 0));
                    }
                    return Fixed::from_raw(wide::int128(q));
                }
            }
        }
#endif
        auto res = detail::mul_div128_kernel(a.raw(), b.raw(), c.raw().low, c.raw().high, rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    } else if constexpr (Bits == 256) {
        if (c.raw().is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
        auto res = detail::mul_div256_kernel(a.raw(), b.raw(), c.raw(), rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    }
}

/// `a % b`: what is left of `a` after removing whole multiples of `b`, with the
/// sign of `a`. Exact, so no rounding mode is taken.
/// \return the remainder, or `ArithmeticError::division_by_zero`.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr std::expected<basic_fixed<Bits, D>, ArithmeticError>
remainder(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if consteval {
        auto raw = detail::ce::remainder<Bits>(a.raw(), b.raw());
        if (!raw) return std::unexpected(raw.error());
        return Fixed::from_raw(*raw);
    }
    if constexpr (Bits <= 64) {
        if (b.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
        if (b.raw() == -1) return Fixed{}; // mathematical remainder is 0, avoid UB
        return Fixed::from_raw(a.raw() % b.raw());
    } else if constexpr (Bits == 128) {
        if (b.raw().is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
        auto res = detail::remainder128_kernel(a.raw(), b.raw());
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    } else if constexpr (Bits == 256) {
        if (b.raw().is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
        auto res = detail::remainder256_kernel(a.raw(), b.raw());
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    }
}

/// Round `a` to `decimals` places, keeping the same type.
///
/// The result still has `D` decimals -- it is the same type -- but its value
/// lands on a coarser grid: `quantize(Fixed64<4>(1.2345), 2)` is 1.2300.
///
/// \param decimals how many decimals to keep, at most `D`.
/// \param rounding how to resolve the rounding.
/// \return the rounded value, or `ArithmeticError::invalid_precision` when
///         `decimals > D`, or `overflow` / `inexact`.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr std::expected<basic_fixed<Bits, D>, ArithmeticError>
quantize(basic_fixed<Bits, D> a, unsigned decimals, Rounding rounding = Rounding::nearest_even) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if consteval {
        auto raw = detail::ce::quantize<Bits, D>(a.raw(), decimals, rounding);
        if (!raw) return std::unexpected(raw.error());
        return Fixed::from_raw(*raw);
    }
    if (decimals > D) return std::unexpected(ArithmeticError::invalid_precision);
    if (decimals == D) return a;
    if constexpr (Bits <= 64) {
        auto res = detail::quantize64_kernel(a.raw(), D, decimals, rounding);
        if constexpr (Bits == 64) {
            if (!res) return std::unexpected(res.error());
            return Fixed::from_raw(*res);
        } else {
            return detail::narrow_checked<Fixed>(
                std::move(res),
                [&] { return detail::quantize64_kernel(a.raw(), D, decimals, Rounding::toward_zero); });
        }
    } else if constexpr (Bits == 128) {
        auto res = detail::quantize128_kernel(a.raw(), D, decimals, rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    } else if constexpr (Bits == 256) {
        auto res = detail::quantize256_kernel(a.raw(), D, decimals, rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    }
}

// Disallow mixed same-name arithmetic without explicit result type.
/// Deleted on purpose: `add`, `sub`, `mul`, `div` and `mul_div` take two
/// operands of the SAME type. Mixing scales or widths has no single obvious
/// result type, so it must be spelled with a destination -- `add_to<Dest>`,
/// `mul_to<Dest>`, … from `<fixedwide/mixed.hpp>`. Calling one of these is a
/// compile error naming this overload, not a silent conversion.
template<typename T, typename U>
    requires (!std::is_same_v<T, U>)
void add(T, U) = delete;

template<typename T, typename U>
    requires (!std::is_same_v<T, U>)
void sub(T, U) = delete;

template<typename T, typename U>
    requires (!std::is_same_v<T, U>)
void mul(T, U) = delete;

template<typename T, typename U>
    requires (!std::is_same_v<T, U>)
void div(T, U) = delete;

template<typename T, typename U, typename V>
    requires (!std::is_same_v<T, U> || !std::is_same_v<T, V>)
void mul_div(T, U, V) = delete;


// 0.4 compatibility surface: fixed at 12 digits. Generic replacement:
// mul_to<Dest>(a, b) in <fixedwide/mixed.hpp>. See fixed.hpp.
[[nodiscard]] inline std::expected<basic_fixed<128, 12>, ArithmeticError>
mul_wide(basic_fixed<64, 12> a, basic_fixed<64, 12> b, Rounding rounding = Rounding::nearest_even) noexcept {
#if defined(FIXEDWIDE_HAS_X86_64_ASM)
    constexpr std::int64_t scale_val = 1'000'000'000'000LL;
    std::uint64_t lo;
    std::int64_t hi;
    __asm__("imulq %[rhs]" : "=a"(lo), "=d"(hi) : "a"(a.raw()), [rhs] "r"(b.raw()) : "cc");
    std::uint64_t uhi = static_cast<std::uint64_t>(hi);
    std::uint64_t half = static_cast<std::uint64_t>(scale_val) / 2;
    if (uhi + half < 2 * half) {
        std::int64_t q, r;
        __asm__("idivq %[div]" : "=a"(q), "=d"(r) : "a"(lo), "d"(hi), [div] "r"(scale_val) : "cc");
        if (r != 0) {
            if (rounding == Rounding::exact) return std::unexpected(ArithmeticError::inexact);
            bool neg = hi < 0;
            if (rounding == Rounding::nearest_even) {
                std::uint64_t ur = neg ? (0ULL - static_cast<std::uint64_t>(r)) : static_cast<std::uint64_t>(r);
                bool is_odd = (q & 1) != 0;
                if ((ur * 2 > static_cast<std::uint64_t>(scale_val)) ||
                    (ur * 2 == static_cast<std::uint64_t>(scale_val) && is_odd)) {
                    q += (neg ? -1 : 1);
                }
            } else if (rounding == Rounding::floor) {
                if (neg) q -= 1;
            } else if (rounding == Rounding::ceil) {
                if (!neg) q += 1;
            } else if (rounding == Rounding::nearest_away) {
                std::uint64_t ur = neg ? (0ULL - static_cast<std::uint64_t>(r)) : static_cast<std::uint64_t>(r);
                if (ur * 2 >= static_cast<std::uint64_t>(scale_val)) q += (neg ? -1 : 1);
            }
        }
        return basic_fixed<128, 12>::from_raw(wide::int128(q));
    } else {
        bool neg = hi < 0;
        std::uint64_t ulo = lo;
        std::uint64_t u_hi = static_cast<std::uint64_t>(hi);
        if (neg) {
            ulo = ~ulo + 1;
            u_hi = ~u_hi + (ulo == 0 ? 1 : 0);
        }
        std::uint64_t rem;
        std::uint64_t qhi = u_hi / static_cast<std::uint64_t>(scale_val);
        std::uint64_t rem_hi = u_hi % static_cast<std::uint64_t>(scale_val);
        std::uint64_t qlo = detail_arith::div128by64_asm(rem_hi, ulo, static_cast<std::uint64_t>(scale_val), rem);
        wide::uint128 quotient(qlo, qhi);
        if (rem != 0) {
            if (rounding == Rounding::exact) return std::unexpected(ArithmeticError::inexact);
            if (detail_arith::round_inc_u64(qlo, rem, static_cast<std::uint64_t>(scale_val), rounding, neg)) {
                quotient = quotient + wide::uint128(1ULL);
            }
        }
        if (neg) {
            wide::uint128 neg_q = ~quotient + wide::uint128(1ULL);
            return basic_fixed<128, 12>::from_raw(wide::int128(neg_q.low, neg_q.high));
        }
        return basic_fixed<128, 12>::from_raw(wide::int128(quotient.low, quotient.high));
    }
#else
    wide::int128 w_a(a.raw());
    wide::int128 w_b(b.raw());
    auto res = detail::mul128_kernel(w_a, w_b, 12, rounding);
    if (!res) return std::unexpected(res.error());
    return basic_fixed<128, 12>::from_raw(*res);
#endif
}

// 0.4 compatibility surface: fixed at 12 digits. Generic replacement:
// fixed_cast<Dest>(value) in <fixedwide/mixed.hpp>. See fixed.hpp.
[[nodiscard]] constexpr std::expected<FP64, ArithmeticError> narrow(FP128 value) noexcept {
    auto r = value.raw();
    if (r.high == 0 && (r.low >> 63) == 0) {
        return FP64::from_raw(static_cast<std::int64_t>(r.low));
    }
    if (r.high == ~0ULL && (r.low >> 63) == 1) {
        return FP64::from_raw(static_cast<std::int64_t>(r.low));
    }
    return std::unexpected(ArithmeticError::overflow);
}

} // namespace fixedwide
