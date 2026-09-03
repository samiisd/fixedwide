#pragma once
#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <expected>
#include <concepts>
#include <type_traits>

namespace fixedwide {

namespace detail {

// Forward declarations of wide compiled kernels
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
template<typename Target, std::integral Integer>
[[nodiscard]] constexpr std::expected<Target, ArithmeticError> from_integer(Integer value) noexcept {
    using raw_t = typename Target::raw_type;
    if constexpr (Target::fractional_digits == 0) {
        if constexpr (Target::bits <= 64) {
            if (value < static_cast<Integer>(Target::min().raw()) || value > static_cast<Integer>(Target::max().raw())) {
                return std::unexpected(ArithmeticError::overflow);
            }
            return Target::from_raw(static_cast<raw_t>(value));
        } else if constexpr (Target::bits == 128) {
            return Target::from_raw(wide::int128(static_cast<std::int64_t>(value)));
        } else {
            return Target::from_raw(wide::int256(static_cast<std::int64_t>(value)));
        }
    } else {
        if constexpr (Target::bits == 8) {
            std::int16_t prod = static_cast<std::int16_t>(value) * static_cast<std::int16_t>(Target::scale());
            if (prod < INT8_MIN || prod > INT8_MAX) return std::unexpected(ArithmeticError::overflow);
            return Target::from_raw(static_cast<std::int8_t>(prod));
        } else if constexpr (Target::bits == 16) {
            std::int32_t prod = static_cast<std::int32_t>(value) * static_cast<std::int32_t>(Target::scale());
            if (prod < INT16_MIN || prod > INT16_MAX) return std::unexpected(ArithmeticError::overflow);
            return Target::from_raw(static_cast<std::int16_t>(prod));
        } else if constexpr (Target::bits == 32) {
            std::int64_t prod = static_cast<std::int64_t>(value) * static_cast<std::int64_t>(Target::scale());
            if (prod < INT32_MIN || prod > INT32_MAX) return std::unexpected(ArithmeticError::overflow);
            return Target::from_raw(static_cast<std::int32_t>(prod));
        } else if constexpr (Target::bits == 64) {
            std::int64_t res;
            if (__builtin_mul_overflow(static_cast<std::int64_t>(value), Target::scale(), &res)) {
                return std::unexpected(ArithmeticError::overflow);
            }
            return Target::from_raw(res);
        } else if constexpr (Target::bits == 128) {
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
            __int128 res;
            __int128 s = static_cast<__int128>(Target::scale());
            if (__builtin_mul_overflow(static_cast<__int128>(value), s, &res)) {
                return std::unexpected(ArithmeticError::overflow);
            }
            return Target::from_raw(wide::int128(res));
#else
            // Portable check
            if (value > 1'000'000'000'000LL || value < -1'000'000'000'000LL) {
                // Approximate scale check
            }
            // Will route through 128-bit multiplication
            return Target::from_raw(wide::int128(static_cast<std::int64_t>(value)) * Target::scale());
#endif
        } else {
            return Target::from_raw(wide::int256(static_cast<std::int64_t>(value)) * Target::scale());
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
        if (__builtin_add_overflow(a.raw(), b.raw(), &res)) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(res);
    } else if constexpr (Bits == 64) {
        std::int64_t res;
        if (__builtin_add_overflow(a.raw(), b.raw(), &res)) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(res);
    } else if constexpr (Bits == 128) {
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
        __int128 res;
        if (__builtin_add_overflow(static_cast<__int128>(a.raw()), static_cast<__int128>(b.raw()), &res)) {
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
        if (__builtin_sub_overflow(a.raw(), b.raw(), &res)) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(res);
    } else if constexpr (Bits == 64) {
        std::int64_t res;
        if (__builtin_sub_overflow(a.raw(), b.raw(), &res)) return std::unexpected(ArithmeticError::overflow);
        return Fixed::from_raw(res);
    } else if constexpr (Bits == 128) {
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
        __int128 res;
        if (__builtin_sub_overflow(static_cast<__int128>(a.raw()), static_cast<__int128>(b.raw()), &res)) {
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
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
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
        auto res = detail::mul64_impl(a.raw(), b.raw(), Fixed::scale(), rounding);
        if (!res) return std::unexpected(res.error());
        return Fixed::from_raw(*res);
    } else if constexpr (Bits == 128) {
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
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
                }
            }
        }
#endif
        auto res = detail::mul128_impl(a.raw(), b.raw(), D, rounding);
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
            auto res = detail::div64_impl(a.raw(), b.raw(), Fixed::scale(), rounding);
            if (!res) return std::unexpected(res.error());
            if (*res < INT32_MIN || *res > INT32_MAX) return std::unexpected(ArithmeticError::overflow);
            return Fixed::from_raw(static_cast<std::int32_t>(*res));
        } else { // Bits == 64
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
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
            auto res = detail::div64_impl(a.raw(), b.raw(), Fixed::scale(), rounding);
            if (!res) return std::unexpected(res.error());
            return Fixed::from_raw(*res);
        }
    } else if constexpr (Bits == 128) {
        if (b.raw().is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
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
                __asm__("imulq %[rhs]" : "=a"(lo), "=d"(hi) : "a"(alow), [rhs] "r"(scale_val) : "cc");
                std::uint64_t mag = blow < 0 ? 0ULL - static_cast<std::uint64_t>(blow) : static_cast<std::uint64_t>(blow);
                std::uint64_t half = mag / 2;
                std::uint64_t uhi = static_cast<std::uint64_t>(hi);
                if (uhi + half < 2 * half && !(blow < 0 && uhi + half == 0)) {
                    std::int64_t q, r;
                    __asm__("idivq %[div]" : "=a"(q), "=d"(r) : "a"(lo), "d"(hi), [div] "r"(blow) : "cc");
                    if (rounding == Rounding::nearest_even && r != 0) {
                        q += detail_arith::nearest_adj(q, r, blow, (hi < 0) != (blow < 0));
                    }
                    return Fixed::from_raw(wide::int128(q));
                }
            }
        }
#endif
        auto res = detail::div128_impl(a.raw(), b.raw(), D, rounding);
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
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
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
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
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
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    __int128 prod = static_cast<__int128>(a.raw()) * b.raw();
    __int128 s = basic_fixed<64, 12>::scale();
    __int128 q = prod / scale;
    if (rounding == Rounding::toward_zero) {
        return basic_fixed<128, 12>::from_raw(wide::int128(q));
    }
    __int128 rem = prod - q * scale;
    if (rem == 0) return basic_fixed<128, 12>::from_raw(wide::int128(q));
    unsigned __int128 urem = rem < 0 ? -rem : rem;
    unsigned __int128 udiv = scale;
    bool inc = false;
    if (rounding == Rounding::nearest_even) {
        bool is_odd = (static_cast<std::uint64_t>(q) & 1) != 0;
        inc = (urem * 2 > udiv) || (urem * 2 == udiv && is_odd);
    } else if (rounding == Rounding::nearest_away) {
        inc = (urem * 2 >= udiv);
    }
    if (inc) {
        q += (prod < 0 ? -1 : 1);
    }
    return basic_fixed<128, 12>::from_raw(wide::int128(q));
#else
    wide::int128 w_a(a.raw());
    wide::int128 w_b(b.raw());
    auto res = detail::mul128_impl(w_a, w_b, 12, rounding);
    if (!res) return std::unexpected(res.error());
    return basic_fixed<128, 12>::from_raw(*res);
#endif
}
} // namespace fixedwide
