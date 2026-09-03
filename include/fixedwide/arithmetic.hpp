#pragma once
#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <fixedwide/detail/overflow.hpp>
#include <expected>
#include <concepts>
#include <type_traits>

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
std::expected<wide::int128, ArithmeticError>
mul128_scaled(wide::int128 a, wide::int128 b, Rounding rounding) noexcept;

template<unsigned D>
std::expected<wide::int128, ArithmeticError>
div128_scaled(wide::int128 a, wide::int128 b, Rounding rounding) noexcept;

template<unsigned D>
std::expected<std::int64_t, ArithmeticError>
mul64_scaled(std::int64_t a, std::int64_t b, Rounding rounding) noexcept;

template<unsigned D>
std::expected<std::int64_t, ArithmeticError>
div64_scaled(std::int64_t a, std::int64_t b, Rounding rounding) noexcept;

std::expected<wide::int128, ArithmeticError>
mul128_impl(wide::int128 a, wide::int128 b, unsigned decimals, Rounding rounding) noexcept;

std::expected<wide::int128, ArithmeticError>
div128_impl(wide::int128 a, wide::int128 b, unsigned decimals, Rounding rounding) noexcept;

std::expected<wide::int128, ArithmeticError>
mul_div128_impl(wide::int128 a, wide::int128 b, wide::int128 c, Rounding rounding) noexcept;

std::expected<wide::int256, ArithmeticError>
mul256_impl(wide::int256 a, wide::int256 b, unsigned decimals, Rounding rounding) noexcept;

std::expected<wide::int256, ArithmeticError>
div256_impl(wide::int256 a, wide::int256 b, unsigned decimals, Rounding rounding) noexcept;

std::expected<wide::int256, ArithmeticError>
mul_div256_impl(wide::int256 a, wide::int256 b, wide::int256 c, Rounding rounding) noexcept;

std::expected<wide::int128, ArithmeticError>
quantize128_impl(wide::int128 a, unsigned current_dec, unsigned target_dec, Rounding rounding) noexcept;

std::expected<wide::int256, ArithmeticError>
quantize256_impl(wide::int256 a, unsigned current_dec, unsigned target_dec, Rounding rounding) noexcept;

std::expected<wide::int128, ArithmeticError>
remainder128_impl(wide::int128 a, wide::int128 b) noexcept;

std::expected<wide::int256, ArithmeticError>
remainder256_impl(wide::int256 a, wide::int256 b) noexcept;

std::expected<std::int64_t, ArithmeticError>
mul64_impl(std::int64_t a, std::int64_t b, std::int64_t scale, Rounding rounding) noexcept;

std::expected<std::int64_t, ArithmeticError>
div64_impl(std::int64_t a, std::int64_t b, std::int64_t scale, Rounding rounding) noexcept;

std::expected<std::int64_t, ArithmeticError>
mul_div64_impl(std::int64_t a, std::int64_t b, std::int64_t c, Rounding rounding) noexcept;

std::expected<std::int64_t, ArithmeticError>
quantize64_impl(std::int64_t a, unsigned cur_dec, unsigned target_dec, Rounding rounding) noexcept;

} // namespace detail

// Construction from integer
template<typename Target, typename Integer>
    requires (std::integral<Integer>
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

        wide::uint256 scale = detail::to_uint256_raw(Target::scale());
        wide::uint256 scaled = mag * scale;
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

// 1. ADD

namespace detail_arith {
template<class UQ, class UR, class UD>
[[nodiscard]] constexpr bool nearest_even_inc(UQ q, UR r, UD d) noexcept {
    const auto tie = q & ~d & 1;
    return r > d / 2 - tie;
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

// 2. SUB
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

// 3. NEGATE
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr std::expected<basic_fixed<Bits, D>, ArithmeticError>
negate(basic_fixed<Bits, D> a) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if (a == Fixed::min()) return std::unexpected(ArithmeticError::overflow);
    return sub(Fixed{}, a);
}

// 4. ABS
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

// 5. MUL
template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::expected<basic_fixed<Bits, D>, ArithmeticError>
mul(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b, Rounding rounding = Rounding::nearest_even) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if constexpr (Bits == 8) {
        std::int32_t prod = static_cast<std::int32_t>(a.raw()) * static_cast<std::int32_t>(b.raw());
        auto res = detail::mul64_impl(prod, 1, Fixed::scale(), rounding);
        if (!res) return std::unexpected(res.error());
        if (*res < INT8_MIN || *res > INT8_MAX) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(static_cast<std::int8_t>(*res));
    } else if constexpr (Bits == 16) {
        std::int64_t prod = static_cast<std::int64_t>(a.raw()) * static_cast<std::int64_t>(b.raw());
        auto res = detail::mul64_impl(prod, 1, Fixed::scale(), rounding);
        if (!res) return std::unexpected(res.error());
        if (*res < INT16_MIN || *res > INT16_MAX) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(static_cast<std::int16_t>(*res));
    } else if constexpr (Bits == 32) {
        std::int64_t prod = static_cast<std::int64_t>(a.raw()) * static_cast<std::int64_t>(b.raw());
        auto res = detail::mul64_impl(prod, 1, Fixed::scale(), rounding);
        if (!res) return std::unexpected(res.error());
        if (*res < INT32_MIN || *res > INT32_MAX) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(static_cast<std::int32_t>(*res));
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
            bool a_fits = (a.raw().high == 0 && (a.raw().low >> 63) == 0) ||
                          (a.raw().high == ~0ULL && (a.raw().low >> 63) == 1);
            bool b_fits = (b.raw().high == 0 && (b.raw().low >> 63) == 0) ||
                          (b.raw().high == ~0ULL && (b.raw().low >> 63) == 1);
            if (a_fits && b_fits && Fixed::scale().high == 0) {
                std::int64_t scale_val = static_cast<std::int64_t>(Fixed::scale().low);
                std::uint64_t lo;
                std::int64_t hi;
                __asm__("imulq %[rhs]" : "=a"(lo), "=d"(hi) : "a"(alow), [rhs] "r"(blow) : "cc");
                std::uint64_t half = static_cast<std::uint64_t>(scale_val) / 2;
                std::uint64_t uhi = static_cast<std::uint64_t>(hi);
                if (uhi + half < 2 * half) {
                    std::int64_t q, r;
                    __asm__("idivq %[div]" : "=a"(q), "=d"(r) : "a"(lo), "d"(hi), [div] "r"(scale_val) : "cc");
                    if (rounding == Rounding::nearest_even && r != 0) {
                        std::uint64_t ur = r < 0 ? 0ULL - static_cast<std::uint64_t>(r) : static_cast<std::uint64_t>(r);
                        std::uint64_t ud = static_cast<std::uint64_t>(scale_val);
                        bool is_odd = (q & 1) != 0;
                        if ((ur * 2 > ud) || (ur * 2 == ud && is_odd)) {
                            q += ((hi < 0) ? -1 : 1);
                        }
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
        auto res = [&] {
            if constexpr (D <= detail::max_scaled_decimals_128)
                return detail::mul128_scaled<D>(a.raw(), b.raw(), rounding);
            else
                return detail::mul128_impl(a.raw(), b.raw(), D, rounding);
        }();
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    } else if constexpr (Bits == 256) {
        auto res = detail::mul256_impl(a.raw(), b.raw(), D, rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    }
}

// 6. DIV
template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::expected<basic_fixed<Bits, D>, ArithmeticError>
div(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b, Rounding rounding = Rounding::nearest_even) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if constexpr (Bits <= 64) {
        if (b.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
        if constexpr (Bits == 8) {
            std::int32_t num = static_cast<std::int32_t>(a.raw()) * Fixed::scale();
            auto res = detail::mul64_impl(num, 1, b.raw(), rounding);
            if (!res) return std::unexpected(res.error());
            if (*res < INT8_MIN || *res > INT8_MAX) return std::unexpected(ArithmeticError::overflow);
            return Fixed::from_raw(static_cast<std::int8_t>(*res));
        } else if constexpr (Bits == 16) {
            std::int64_t num = static_cast<std::int64_t>(a.raw()) * Fixed::scale();
            auto res = detail::mul64_impl(num, 1, b.raw(), rounding);
            if (!res) return std::unexpected(res.error());
            if (*res < INT16_MIN || *res > INT16_MAX) return std::unexpected(ArithmeticError::overflow);
            return Fixed::from_raw(static_cast<std::int16_t>(*res));
        } else if constexpr (Bits == 32) {
            auto res = detail::div64_scaled<D>(a.raw(), b.raw(), rounding);
            if (!res) return std::unexpected(res.error());
            if (*res < INT32_MIN || *res > INT32_MAX) return std::unexpected(ArithmeticError::overflow);
            return Fixed::from_raw(static_cast<std::int32_t>(*res));
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
        auto res = [&] {
            if constexpr (D <= detail::max_scaled_decimals_128)
                return detail::div128_scaled<D>(a.raw(), b.raw(), rounding);
            else
                return detail::div128_impl(a.raw(), b.raw(), D, rounding);
        }();
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    } else if constexpr (Bits == 256) {
        if (b.raw().is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
        auto res = detail::div256_impl(a.raw(), b.raw(), D, rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    }
}

// 7. MUL_DIV
template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::expected<basic_fixed<Bits, D>, ArithmeticError>
mul_div(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b, basic_fixed<Bits, D> c, Rounding rounding = Rounding::nearest_even) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if constexpr (Bits <= 64) {
        if (c.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
        if constexpr (Bits == 8) {
            std::int32_t num = static_cast<std::int32_t>(a.raw()) * static_cast<std::int32_t>(b.raw());
            auto res = detail::mul64_impl(num, 1, c.raw(), rounding);
            if (!res) return std::unexpected(res.error());
            if (*res < INT8_MIN || *res > INT8_MAX) return std::unexpected(ArithmeticError::overflow);
            return Fixed::from_raw(static_cast<std::int8_t>(*res));
        } else if constexpr (Bits == 16) {
            std::int64_t num = static_cast<std::int64_t>(a.raw()) * static_cast<std::int64_t>(b.raw());
            auto res = detail::mul64_impl(num, 1, c.raw(), rounding);
            if (!res) return std::unexpected(res.error());
            if (*res < INT16_MIN || *res > INT16_MAX) return std::unexpected(ArithmeticError::overflow);
            return Fixed::from_raw(static_cast<std::int16_t>(*res));
        } else if constexpr (Bits == 32) {
            auto res = detail::mul_div64_impl(a.raw(), b.raw(), c.raw(), rounding);
            if (!res) return std::unexpected(res.error());
            if (*res < INT32_MIN || *res > INT32_MAX) return std::unexpected(ArithmeticError::overflow);
            return Fixed::from_raw(static_cast<std::int32_t>(*res));
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
            auto res = detail::mul_div64_impl(a.raw(), b.raw(), c.raw(), rounding);
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
            bool a_fits = (a.raw().high == 0 && (a.raw().low >> 63) == 0) ||
                          (a.raw().high == ~0ULL && (a.raw().low >> 63) == 1);
            bool b_fits = (b.raw().high == 0 && (b.raw().low >> 63) == 0) ||
                          (b.raw().high == ~0ULL && (b.raw().low >> 63) == 1);
            bool c_fits = (c.raw().high == 0 && (c.raw().low >> 63) == 0) ||
                          (c.raw().high == ~0ULL && (c.raw().low >> 63) == 1);
            if (a_fits && b_fits && c_fits) {
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
        auto res = detail::mul_div128_impl(a.raw(), b.raw(), c.raw(), rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    } else if constexpr (Bits == 256) {
        if (c.raw().is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
        auto res = detail::mul_div256_impl(a.raw(), b.raw(), c.raw(), rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    }
}

// 8. REMAINDER
template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::expected<basic_fixed<Bits, D>, ArithmeticError>
remainder(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if constexpr (Bits <= 64) {
        if (b.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
        if (b.raw() == -1) return Fixed{}; // mathematical remainder is 0, avoid UB
        return Fixed::from_raw(a.raw() % b.raw());
    } else if constexpr (Bits == 128) {
        if (b.raw().is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
        auto res = detail::remainder128_impl(a.raw(), b.raw());
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    } else if constexpr (Bits == 256) {
        if (b.raw().is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
        auto res = detail::remainder256_impl(a.raw(), b.raw());
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    }
}

// 9. QUANTIZE
template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::expected<basic_fixed<Bits, D>, ArithmeticError>
quantize(basic_fixed<Bits, D> a, unsigned digits, Rounding rounding = Rounding::nearest_even) noexcept {
    using Fixed = basic_fixed<Bits, D>;
    if (digits > D) return std::unexpected(ArithmeticError::invalid_precision);
    if (digits == D) return a;
    if constexpr (Bits <= 64) {
        auto res = detail::quantize64_impl(a.raw(), D, digits, rounding);
        if (!res) return std::unexpected(res.error());
        if constexpr (Bits == 8) {
            if (*res < INT8_MIN || *res > INT8_MAX) return std::unexpected(ArithmeticError::overflow);
            return Fixed::from_raw(static_cast<std::int8_t>(*res));
        } else if constexpr (Bits == 16) {
            if (*res < INT16_MIN || *res > INT16_MAX) return std::unexpected(ArithmeticError::overflow);
            return Fixed::from_raw(static_cast<std::int16_t>(*res));
        } else if constexpr (Bits == 32) {
            if (*res < INT32_MIN || *res > INT32_MAX) return std::unexpected(ArithmeticError::overflow);
            return Fixed::from_raw(static_cast<std::int32_t>(*res));
        } else {
            return Fixed::from_raw(*res);
        }
    } else if constexpr (Bits == 128) {
        auto res = detail::quantize128_impl(a.raw(), D, digits, rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    } else if constexpr (Bits == 256) {
        auto res = detail::quantize256_impl(a.raw(), D, digits, rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    }
}

// Disallow mixed same-name arithmetic without explicit result type
template<typename T, typename U>
    requires (!std::same_as<T, U>)
void add(T, U) = delete;

template<typename T, typename U>
    requires (!std::same_as<T, U>)
void sub(T, U) = delete;

template<typename T, typename U>
    requires (!std::same_as<T, U>)
void mul(T, U) = delete;

template<typename T, typename U>
    requires (!std::same_as<T, U>)
void div(T, U) = delete;

template<typename T, typename U, typename V>
    requires (!std::same_as<T, U> || !std::same_as<T, V>)
void mul_div(T, U, V) = delete;


// Backward compatibility mul_wide
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
    auto res = detail::mul128_impl(w_a, w_b, 12, rounding);
    if (!res) return std::unexpected(res.error());
    return basic_fixed<128, 12>::from_raw(*res);
#endif
}

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
