#pragma once

/// \file
/// Arithmetic and comparison ACROSS fixed-point types of different widths and
/// scales.
///
/// Every operation here names its destination -- `mul_to<Dest>(a, b)` -- because
/// two different scales have no single obvious result scale. The expression is
/// evaluated exactly and rounded once, directly into the destination: there is
/// no intermediate type to lose a digit in. Comparison is the exception; it
/// needs no destination and is always exact.

#include <fixedwide/fixed.hpp>
#include <fixedwide/detail/mixed_native.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <array>
#include <compare>
#include <expected>

namespace fixedwide {

namespace detail {

// Forward declarations for mixed arithmetic kernel functions
std::expected<wide::int256, ArithmeticError>
mixed_cast_kernel(wide::int256 src_raw, unsigned src_decimals, unsigned dest_decimals,
                  Rounding rounding, std::size_t dest_bits) noexcept;

std::expected<wide::int256, ArithmeticError>
mixed_add_sub_kernel(wide::int256 a_raw, unsigned a_decimals,
                     wide::int256 b_raw, unsigned b_decimals,
                     bool subtract, unsigned dest_decimals,
                     Rounding rounding, std::size_t dest_bits) noexcept;

std::expected<wide::int256, ArithmeticError>
mixed_mul_kernel(wide::int256 a_raw, unsigned a_decimals,
                 wide::int256 b_raw, unsigned b_decimals,
                 unsigned dest_decimals, Rounding rounding, std::size_t dest_bits) noexcept;

std::expected<wide::int256, ArithmeticError>
mixed_div_kernel(wide::int256 a_raw, unsigned a_decimals,
                 wide::int256 b_raw, unsigned b_decimals,
                 unsigned dest_decimals, Rounding rounding, std::size_t dest_bits) noexcept;

std::expected<wide::int256, ArithmeticError>
mixed_mul_div_kernel(wide::int256 a_raw, unsigned a_decimals,
                     wide::int256 b_raw, unsigned b_decimals,
                     wide::int256 c_raw, unsigned c_decimals,
                     unsigned dest_decimals, Rounding rounding, std::size_t dest_bits) noexcept;

std::strong_ordering
mixed_compare_kernel(wide::int256 a_raw, unsigned a_decimals,
                     wide::int256 b_raw, unsigned b_decimals) noexcept;



// Exact cross-scale comparison during constant evaluation.
//
// The runtime path below is `mixed_compare_kernel`, which lives in the compiled
// library and so cannot run in a constant expression. Without this, the
// `constexpr` on the two comparison operators was only true for the narrow
// `mixed_native` fast path -- any comparison involving a Fixed128 or Fixed256
// failed to compile inside a `static_assert`, while advertising that it would
// not. This is the same algorithm, written to run at compile time.
//
// Widths: a magnitude is under 2^255 and the largest scale gap is 10^76, which
// is under 2^253, so an aligned value needs at most 508 bits. Nine limbs is 576.
inline constexpr std::size_t compare_limbs = 9;
using compare_magnitude = std::array<std::uint64_t, compare_limbs>;

/// Magnitude of a 256-bit raw value, widened to the comparison width.
[[nodiscard]] constexpr compare_magnitude to_compare_magnitude(wide::int256 v) noexcept {
    compare_magnitude out{};
    if (v.is_negative()) {
        // Two's complement negation across the four limbs.
        std::uint64_t borrow = 1;
        for (std::size_t i = 0; i < 4; ++i) {
            const std::uint64_t inverted = ~v.limbs[i];
            out[i] = inverted + borrow;
            borrow = (out[i] < borrow) ? 1u : 0u;
        }
    } else {
        for (std::size_t i = 0; i < 4; ++i) out[i] = v.limbs[i];
    }
    return out;
}

/// Multiply in place by ten. Split into 32-bit halves so no 128-bit type and no
/// compiler intrinsic is needed -- this has to work on MSVC and in constexpr.
constexpr void multiply_by_ten(compare_magnitude& value) noexcept {
    std::uint64_t carry = 0;
    for (std::size_t i = 0; i < compare_limbs; ++i) {
        const std::uint64_t low  = (value[i] & 0xFFFF'FFFFULL) * 10u + carry;
        const std::uint64_t high = (value[i] >> 32) * 10u + (low >> 32);
        value[i] = (high << 32) | (low & 0xFFFF'FFFFULL);
        carry = high >> 32;
    }
}

/// Compare two magnitudes, most significant limb first.
[[nodiscard]] constexpr std::strong_ordering
compare_magnitudes(const compare_magnitude& a, const compare_magnitude& b) noexcept {
    for (std::size_t i = compare_limbs; i-- > 0;) {
        if (a[i] != b[i]) return a[i] < b[i] ? std::strong_ordering::less
                                             : std::strong_ordering::greater;
    }
    return std::strong_ordering::equal;
}

[[nodiscard]] constexpr std::strong_ordering
constexpr_mixed_compare(wide::int256 a_raw, unsigned a_decimals,
                        wide::int256 b_raw, unsigned b_decimals) noexcept {
    const bool a_negative = a_raw.is_negative();
    const bool b_negative = b_raw.is_negative();
    if (a_negative != b_negative) {
        return a_negative ? std::strong_ordering::less : std::strong_ordering::greater;
    }
    if (a_raw.is_zero() && b_raw.is_zero()) return std::strong_ordering::equal;

    // Lift both sides to the coarser of the two scales. No division, so nothing
    // is rounded and the answer is exact.
    const unsigned common = a_decimals < b_decimals ? a_decimals : b_decimals;
    compare_magnitude a_magnitude = to_compare_magnitude(a_raw);
    compare_magnitude b_magnitude = to_compare_magnitude(b_raw);
    for (unsigned i = 0; i < b_decimals - common; ++i) multiply_by_ten(a_magnitude);
    for (unsigned i = 0; i < a_decimals - common; ++i) multiply_by_ten(b_magnitude);

    const auto ordering = compare_magnitudes(a_magnitude, b_magnitude);
    if (!a_negative) return ordering;
    // Both are negative, so the larger magnitude is the smaller value.
    if (ordering == std::strong_ordering::less) return std::strong_ordering::greater;
    if (ordering == std::strong_ordering::greater) return std::strong_ordering::less;
    return std::strong_ordering::equal;
}

} // namespace detail

/// Exact equality across widths and scales.
///
/// Unlike arithmetic, comparison needs no destination type and cannot lose
/// anything: both sides are lifted to a common exponent and the integers are
/// compared. `Fixed64<2>(1.50) == Fixed32<4>(1.5000)` is true, and no division
/// happens.
template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db>
    requires (BitsA != BitsB || Da != Db)
[[nodiscard]] constexpr bool operator==(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b) noexcept {
    return (a <=> b) == std::strong_ordering::equal;
}

/// Exact ordering across widths and scales.
///
/// Needs no destination type and cannot lose anything: both sides are lifted to
/// a common exponent and the integers are compared. No division happens.
template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db>
    requires (BitsA != BitsB || Da != Db)
[[nodiscard]] constexpr std::strong_ordering operator<=>(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b) noexcept {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    // Comparing across scales needs no division, only a common exponent. The
    // general kernel reaches for 1024-bit limbs to do it.
    if constexpr (detail::mixed_native::alignment_fits<BitsA, Da, BitsB, Db>()) {
        return detail::mixed_native::compare<BitsA, Da, BitsB, Db>(a.raw(), b.raw());
    }
#endif
    if consteval {
        // The general kernel is compiled into the library and cannot run here.
        return detail::constexpr_mixed_compare(detail::to_int256_raw(a.raw()), Da,
                                               detail::to_int256_raw(b.raw()), Db);
    }
    return detail::mixed_compare_kernel(detail::to_int256_raw(a.raw()), Da,
                                        detail::to_int256_raw(b.raw()), Db);
}

/// Convert to another fixed-point type: any width, any scale.
///
/// The default rounding is `Rounding::exact`, so a conversion that would drop a
/// digit is an error rather than a silent loss. Pass a rounding mode when you
/// mean to round.
///
/// \tparam Dest the destination type.
/// \param rounding how to resolve a narrowing scale; `exact` by default.
/// \return the converted value, or `ArithmeticError::overflow` / `inexact`.
template<typename Dest, std::size_t BitsA, unsigned Da>
[[nodiscard]] inline std::expected<Dest, ArithmeticError>
fixed_cast(basic_fixed<BitsA, Da> a, Rounding rounding = Rounding::exact) noexcept {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    if constexpr (detail::mixed_native::cast_fits<BitsA, Da, Dest::bits, Dest::fractional_digits>()) {
        const auto native = detail::mixed_native::cast<BitsA, Da, Dest::bits, Dest::fractional_digits>(a.raw(), rounding);
        if (!native) return std::unexpected(native.error());
        return Dest::from_raw(static_cast<typename Dest::raw_type>(*native));
    }
#endif
    auto res = detail::mixed_cast_kernel(detail::to_int256_raw(a.raw()), Da, Dest::fractional_digits,
                                         rounding, Dest::bits);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<Dest>(*res);
}

/// `a + b`, evaluated exactly and rounded once into `Dest`.
///
/// The sum is formed at the wider of the two scales, so the only rounding is the one that lands it in `Dest`.
///
/// \tparam Dest the destination type, which must be named: there is no single
///              obvious result type for two different scales, so the library
///              refuses to guess.
/// \param rounding how to resolve the single rounding.
/// \return the result, or an `ArithmeticError`.
template<typename Dest, std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db>
[[nodiscard]] inline std::expected<Dest, ArithmeticError>
add_to(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b, Rounding rounding = Rounding::nearest_even) noexcept {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    if constexpr (detail::mixed_native::add_fits<BitsA, Da, BitsB, Db, Dest::bits, Dest::fractional_digits>()) {
        const auto native = detail::mixed_native::add_sub<BitsA, Da, BitsB, Db, Dest::bits, Dest::fractional_digits>(a.raw(), b.raw(), false, rounding);
        if (!native) return std::unexpected(native.error());
        return Dest::from_raw(static_cast<typename Dest::raw_type>(*native));
    }
#endif
    auto res = detail::mixed_add_sub_kernel(detail::to_int256_raw(a.raw()), Da,
                                            detail::to_int256_raw(b.raw()), Db,
                                            false, Dest::fractional_digits, rounding, Dest::bits);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<Dest>(*res);
}

/// `a - b`, evaluated exactly and rounded once into `Dest`.
///
/// The difference is formed at the wider of the two scales, so the only rounding is the one that lands it in `Dest`.
///
/// \tparam Dest the destination type, which must be named: there is no single
///              obvious result type for two different scales, so the library
///              refuses to guess.
/// \param rounding how to resolve the single rounding.
/// \return the result, or an `ArithmeticError`.
template<typename Dest, std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db>
[[nodiscard]] inline std::expected<Dest, ArithmeticError>
sub_to(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b, Rounding rounding = Rounding::nearest_even) noexcept {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    if constexpr (detail::mixed_native::add_fits<BitsA, Da, BitsB, Db, Dest::bits, Dest::fractional_digits>()) {
        const auto native = detail::mixed_native::add_sub<BitsA, Da, BitsB, Db, Dest::bits, Dest::fractional_digits>(a.raw(), b.raw(), true, rounding);
        if (!native) return std::unexpected(native.error());
        return Dest::from_raw(static_cast<typename Dest::raw_type>(*native));
    }
#endif
    auto res = detail::mixed_add_sub_kernel(detail::to_int256_raw(a.raw()), Da,
                                            detail::to_int256_raw(b.raw()), Db,
                                            true, Dest::fractional_digits, rounding, Dest::bits);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<Dest>(*res);
}

/// `a * b`, evaluated exactly and rounded once into `Dest`.
///
/// The product carries the sum of the two scales and is rescaled to `Dest` in a single step; no intermediate type truncates it.
///
/// \tparam Dest the destination type, which must be named: there is no single
///              obvious result type for two different scales, so the library
///              refuses to guess.
/// \param rounding how to resolve the single rounding.
/// \return the result, or an `ArithmeticError`.
template<typename Dest, std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db>
[[nodiscard]] inline std::expected<Dest, ArithmeticError>
mul_to(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b, Rounding rounding = Rounding::nearest_even) noexcept {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    if constexpr (detail::mixed_native::mul_fits<BitsA, Da, BitsB, Db, Dest::bits, Dest::fractional_digits>()) {
        const auto native = detail::mixed_native::mul<BitsA, Da, BitsB, Db, Dest::bits, Dest::fractional_digits>(a.raw(), b.raw(), rounding);
        if (!native) return std::unexpected(native.error());
        return Dest::from_raw(static_cast<typename Dest::raw_type>(*native));
    }
#endif
    auto res = detail::mixed_mul_kernel(detail::to_int256_raw(a.raw()), Da,
                                        detail::to_int256_raw(b.raw()), Db,
                                        Dest::fractional_digits, rounding, Dest::bits);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<Dest>(*res);
}

/// `a / b`, evaluated exactly and rounded once into `Dest`.
///
/// The quotient is produced directly at `Dest`'s scale rather than at either operand's, so nothing is lost on the way.
///
/// \tparam Dest the destination type, which must be named: there is no single
///              obvious result type for two different scales, so the library
///              refuses to guess.
/// \param rounding how to resolve the single rounding.
/// \return the result, or an `ArithmeticError`.
template<typename Dest, std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db>
[[nodiscard]] inline std::expected<Dest, ArithmeticError>
div_to(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b, Rounding rounding = Rounding::nearest_even) noexcept {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    if constexpr (detail::mixed_native::div_fits<BitsA, Da, BitsB, Db, Dest::bits, Dest::fractional_digits>()) {
        const auto native = detail::mixed_native::div<BitsA, Da, BitsB, Db, Dest::bits, Dest::fractional_digits>(a.raw(), b.raw(), rounding);
        if (!native) return std::unexpected(native.error());
        return Dest::from_raw(static_cast<typename Dest::raw_type>(*native));
    }
#endif
    auto res = detail::mixed_div_kernel(detail::to_int256_raw(a.raw()), Da,
                                        detail::to_int256_raw(b.raw()), Db,
                                        Dest::fractional_digits, rounding, Dest::bits);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<Dest>(*res);
}

/// `a * b / c`, evaluated exactly and rounded once into `Dest`.
///
/// One rounding for the whole expression, across three operands that need share
/// neither width nor scale. Doing it as `mul_to` then `div_to` would round
/// twice.
///
/// \tparam Dest the destination type.
/// \param rounding how to resolve the single rounding.
/// \return the result, or `ArithmeticError::division_by_zero` / `overflow`
///         / `inexact`.
template<typename Dest, std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db, std::size_t BitsC, unsigned Dc>
[[nodiscard]] inline std::expected<Dest, ArithmeticError>
mul_div_to(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b, basic_fixed<BitsC, Dc> c, Rounding rounding = Rounding::nearest_even) noexcept {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    if constexpr (detail::mixed_native::mul_div_fits<BitsA, Da, BitsB, Db, BitsC, Dc, Dest::bits, Dest::fractional_digits>()) {
        const auto native = detail::mixed_native::mul_div<BitsA, Da, BitsB, Db, BitsC, Dc, Dest::bits, Dest::fractional_digits>(a.raw(), b.raw(), c.raw(), rounding);
        if (!native) return std::unexpected(native.error());
        return Dest::from_raw(static_cast<typename Dest::raw_type>(*native));
    }
#endif
    auto res = detail::mixed_mul_div_kernel(detail::to_int256_raw(a.raw()), Da,
                                            detail::to_int256_raw(b.raw()), Db,
                                            detail::to_int256_raw(c.raw()), Dc,
                                            Dest::fractional_digits, rounding, Dest::bits);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<Dest>(*res);
}

} // namespace fixedwide
