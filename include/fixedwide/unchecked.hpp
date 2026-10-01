#pragma once

/// \file
/// Caller-responsibility arithmetic, returning values rather than expected.
/// Results must fit, divisors must be nonzero, precisions must be valid, and
/// Rounding::exact requires an exact result. Violating these preconditions has
/// undefined behaviour; wrapping, saturation and recovery are not provided.
/// Include this header (or all.hpp) for ordinary arithmetic operators too.
/// See docs/unchecked.md for the contract and which paths remove kernel checks.

#include <fixedwide/arithmetic.hpp>
#include <fixedwide/mixed.hpp>
#include <cassert>
#include <type_traits>
#include <utility>

namespace fixedwide::detail {

template<class T>
[[nodiscard]] constexpr T unchecked_result(std::expected<T, ArithmeticError> result) noexcept {
    assert(result.has_value());
    return *result;
}

constexpr void unchecked_precondition(bool condition) noexcept {
    assert(condition);
    if (!condition) std::unreachable();
}

[[nodiscard]] constexpr std::int64_t unchecked_round64(std::int64_t q, std::int64_t r, std::int64_t divisor,
                                                       bool negative, Rounding rounding) noexcept {
    if (r == 0) return q;
    const auto magnitude = r < 0 ? 0ULL - static_cast<std::uint64_t>(r) : static_cast<std::uint64_t>(r);
    const auto denominator =
        divisor < 0 ? 0ULL - static_cast<std::uint64_t>(divisor) : static_cast<std::uint64_t>(divisor);
    bool increment = false;
    switch (rounding) {
    case Rounding::toward_zero: return q;
    case Rounding::floor: increment = negative; break;
    case Rounding::ceil: increment = !negative; break;
    case Rounding::nearest_even:
        increment = magnitude > denominator - magnitude || (magnitude == denominator - magnitude && (q & 1) != 0);
        break;
    case Rounding::nearest_away: increment = magnitude >= denominator - magnitude; break;
    case Rounding::exact: unchecked_precondition(false); return q;
    default: unchecked_precondition(false); return q;
    }
    return increment ? q + (negative ? -1 : 1) : q;
}

template<std::size_t Bits>
inline constexpr bool has_unchecked_product = Bits <= 32
#if defined(FIXEDWIDE_HAS_X86_64_ASM)
                                              || Bits == 64
#endif
    ;

// The full product is retained. Only the final, rounded result has to fit.
template<class Fixed>
[[nodiscard]] inline Fixed unchecked_product_quotient(std::int64_t a, std::int64_t b, std::int64_t c,
                                                      Rounding rounding) noexcept {
    std::int64_t q, r;
    if constexpr (Fixed::bits <= 32) {
        const std::int64_t product = a * b;
        q = product / c;
        r = product % c;
    } else {
#if defined(FIXEDWIDE_HAS_X86_64_ASM)
        std::uint64_t low;
        std::int64_t high;
        __asm__("imulq %[rhs]" : "=a"(low), "=d"(high) : "a"(a), [rhs] "r"(b) : "cc");
        __asm__("idivq %[divisor]" : "=a"(q), "=d"(r) : "a"(low), "d"(high), [divisor] "r"(c) : "cc");
#else
        static_assert(Fixed::bits <= 32);
#endif
    }
    const bool negative = ((a < 0) != (b < 0)) != (c < 0);
    return Fixed::from_raw(static_cast<typename Fixed::raw_type>(unchecked_round64(q, r, c, negative, rounding)));
}

} // namespace fixedwide::detail

namespace fixedwide::unchecked {

/// Same-type addition. The exact sum must fit.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> add(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b) noexcept {
    if consteval {
        return detail::unchecked_result(fixedwide::add(a, b));
    }
    assert(fixedwide::add(a, b).has_value());
    using F = basic_fixed<Bits, D>;
    return F::from_raw(static_cast<typename F::raw_type>(a.raw() + b.raw()));
}

/// Same-type subtraction. The exact difference must fit.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> sub(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b) noexcept {
    if consteval {
        return detail::unchecked_result(fixedwide::sub(a, b));
    }
    assert(fixedwide::sub(a, b).has_value());
    using F = basic_fixed<Bits, D>;
    return F::from_raw(static_cast<typename F::raw_type>(a.raw() - b.raw()));
}

/// The operand must not be min().
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> negate(basic_fixed<Bits, D> a) noexcept {
    if consteval {
        return detail::unchecked_result(fixedwide::negate(a));
    }
    using F = basic_fixed<Bits, D>;
    assert(a != F::min());
    return F::from_raw(static_cast<typename F::raw_type>(-a.raw()));
}

/// The operand must not be min().
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> abs(basic_fixed<Bits, D> a) noexcept {
    return a < basic_fixed<Bits, D>{} ? unchecked::negate(a) : a;
}

/// Widened product, rounded once. The rounded destination must fit.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> mul(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b,
                                                 Rounding rounding = Rounding::nearest_even) noexcept {
    if consteval {
        return detail::unchecked_result(fixedwide::mul(a, b, rounding));
    }
    if constexpr (detail::has_unchecked_product<Bits>) {
        assert(fixedwide::mul(a, b, rounding).has_value());
        using F = basic_fixed<Bits, D>;
        return detail::unchecked_product_quotient<F>(a.raw(), b.raw(), F::scale(), rounding);
    } else {
        return detail::unchecked_result(fixedwide::mul(a, b, rounding));
    }
}

/// Widened numerator, rounded once. The divisor must be nonzero and the rounded result must fit.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> div(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b,
                                                 Rounding rounding = Rounding::nearest_even) noexcept {
    if consteval {
        return detail::unchecked_result(fixedwide::div(a, b, rounding));
    }
    if constexpr (detail::has_unchecked_product<Bits>) {
        assert(fixedwide::div(a, b, rounding).has_value());
        using F = basic_fixed<Bits, D>;
        return detail::unchecked_product_quotient<F>(a.raw(), F::scale(), b.raw(), rounding);
    } else {
        return detail::unchecked_result(fixedwide::div(a, b, rounding));
    }
}

/// Fused a*b/c: one final rounding, not a rounded multiply followed by a divide.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> mul_div(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b,
                                                     basic_fixed<Bits, D> c,
                                                     Rounding rounding = Rounding::nearest_even) noexcept {
    if consteval {
        return detail::unchecked_result(fixedwide::mul_div(a, b, c, rounding));
    }
    if constexpr (detail::has_unchecked_product<Bits>) {
        assert(fixedwide::mul_div(a, b, c, rounding).has_value());
        return detail::unchecked_product_quotient<basic_fixed<Bits, D>>(a.raw(), b.raw(), c.raw(), rounding);
    } else {
        return detail::unchecked_result(fixedwide::mul_div(a, b, c, rounding));
    }
}

/// Exact signed remainder. The divisor must be nonzero; min() % from_raw(-1) is zero.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> remainder(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b) noexcept {
    if consteval {
        return detail::unchecked_result(fixedwide::remainder(a, b));
    }
    if constexpr (Bits <= 64) {
        assert(b.raw() != 0);
        using F = basic_fixed<Bits, D>;
        if (b.raw() == -1) return F{};
        return F::from_raw(static_cast<typename F::raw_type>(a.raw() % b.raw()));
    } else {
        return detail::unchecked_result(fixedwide::remainder(a, b));
    }
}

// Value-returning adapters below retain the existing checked numerical kernels.
// No claim is made that every internal check disappears without inlining/LTO.
template<class Target, class Integer>
[[nodiscard]] constexpr Target from_integer(Integer value) noexcept {
    return detail::unchecked_result(fixedwide::from_integer<Target>(value));
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> quantize(basic_fixed<Bits, D> a, unsigned decimals,
                                                      Rounding rounding = Rounding::nearest_even) noexcept {
    return detail::unchecked_result(fixedwide::quantize(a, decimals, rounding));
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> midpoint(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b,
                                                      Rounding rounding = Rounding::nearest_even) noexcept {
    return detail::unchecked_result(fixedwide::midpoint(a, b, rounding));
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> midpoint(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b, unsigned decimals,
                                                      Rounding rounding = Rounding::nearest_even) noexcept {
    return detail::unchecked_result(fixedwide::midpoint(a, b, decimals, rounding));
}

template<class Dest, class Src>
[[nodiscard]] constexpr Dest fixed_cast(Src value, Rounding rounding = Rounding::exact) noexcept {
    return detail::unchecked_result(fixedwide::fixed_cast<Dest>(value, rounding));
}

template<class Dest, class A, class B>
[[nodiscard]] constexpr Dest add_to(A a, B b, Rounding rounding = Rounding::nearest_even) noexcept {
    return detail::unchecked_result(fixedwide::add_to<Dest>(a, b, rounding));
}

template<class Dest, class A, class B>
[[nodiscard]] constexpr Dest sub_to(A a, B b, Rounding rounding = Rounding::nearest_even) noexcept {
    return detail::unchecked_result(fixedwide::sub_to<Dest>(a, b, rounding));
}

template<class Dest, class A, class B>
[[nodiscard]] constexpr Dest mul_to(A a, B b, Rounding rounding = Rounding::nearest_even) noexcept {
    return detail::unchecked_result(fixedwide::mul_to<Dest>(a, b, rounding));
}

template<class Dest, class A, class B>
[[nodiscard]] constexpr Dest div_to(A a, B b, Rounding rounding = Rounding::nearest_even) noexcept {
    return detail::unchecked_result(fixedwide::div_to<Dest>(a, b, rounding));
}

template<class Dest, class A, class B, class C>
[[nodiscard]] constexpr Dest mul_div_to(A a, B b, C c, Rounding rounding = Rounding::nearest_even) noexcept {
    return detail::unchecked_result(fixedwide::mul_div_to<Dest>(a, b, c, rounding));
}

} // namespace fixedwide::unchecked

namespace fixedwide {

/// Operators use the unchecked contract. Mixed widths/scales still require an explicit destination.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> operator+(basic_fixed<Bits, D> a,
                                                       std::type_identity_t<basic_fixed<Bits, D>> b) noexcept {
    return unchecked::add(a, b);
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> operator-(basic_fixed<Bits, D> a,
                                                       std::type_identity_t<basic_fixed<Bits, D>> b) noexcept {
    return unchecked::sub(a, b);
}

/// Multiplication and division operators round once, nearest-even, at the existing scale.
template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> operator*(basic_fixed<Bits, D> a,
                                                       std::type_identity_t<basic_fixed<Bits, D>> b) noexcept {
    return unchecked::mul(a, b);
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> operator/(basic_fixed<Bits, D> a,
                                                       std::type_identity_t<basic_fixed<Bits, D>> b) noexcept {
    return unchecked::div(a, b);
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> operator%(basic_fixed<Bits, D> a,
                                                       std::type_identity_t<basic_fixed<Bits, D>> b) noexcept {
    return unchecked::remainder(a, b);
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> operator+(basic_fixed<Bits, D> a) noexcept {
    return a;
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] constexpr basic_fixed<Bits, D> operator-(basic_fixed<Bits, D> a) noexcept {
    return unchecked::negate(a);
}

template<std::size_t Bits, unsigned D>
constexpr basic_fixed<Bits, D>& operator+=(basic_fixed<Bits, D>& a,
                                           std::type_identity_t<basic_fixed<Bits, D>> b) noexcept {
    return a = a + b;
}

template<std::size_t Bits, unsigned D>
constexpr basic_fixed<Bits, D>& operator-=(basic_fixed<Bits, D>& a,
                                           std::type_identity_t<basic_fixed<Bits, D>> b) noexcept {
    return a = a - b;
}

template<std::size_t Bits, unsigned D>
constexpr basic_fixed<Bits, D>& operator*=(basic_fixed<Bits, D>& a,
                                           std::type_identity_t<basic_fixed<Bits, D>> b) noexcept {
    return a = a * b;
}

template<std::size_t Bits, unsigned D>
constexpr basic_fixed<Bits, D>& operator/=(basic_fixed<Bits, D>& a,
                                           std::type_identity_t<basic_fixed<Bits, D>> b) noexcept {
    return a = a / b;
}

template<std::size_t Bits, unsigned D>
constexpr basic_fixed<Bits, D>& operator%=(basic_fixed<Bits, D>& a,
                                           std::type_identity_t<basic_fixed<Bits, D>> b) noexcept {
    return a = a % b;
}

} // namespace fixedwide
