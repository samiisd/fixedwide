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
#include <expected>
#include <compare>

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
