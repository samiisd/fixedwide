#pragma once
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
mixed_cast_kernel(wide::int256 src_raw, unsigned src_dec, unsigned dest_dec,
                  Rounding rounding, std::size_t dest_bits) noexcept;

std::expected<wide::int256, ArithmeticError>
mixed_add_sub_kernel(wide::int256 a_raw, unsigned a_dec,
                     wide::int256 b_raw, unsigned b_dec,
                     bool is_sub, unsigned dest_dec,
                     Rounding rounding, std::size_t dest_bits) noexcept;

std::expected<wide::int256, ArithmeticError>
mixed_mul_kernel(wide::int256 a_raw, unsigned a_dec,
                 wide::int256 b_raw, unsigned b_dec,
                 unsigned dest_dec, Rounding rounding, std::size_t dest_bits) noexcept;

std::expected<wide::int256, ArithmeticError>
mixed_div_kernel(wide::int256 a_raw, unsigned a_dec,
                 wide::int256 b_raw, unsigned b_dec,
                 unsigned dest_dec, Rounding rounding, std::size_t dest_bits) noexcept;

std::expected<wide::int256, ArithmeticError>
mixed_mul_div_kernel(wide::int256 a_raw, unsigned a_dec,
                     wide::int256 b_raw, unsigned b_dec,
                     wide::int256 c_raw, unsigned c_dec,
                     unsigned dest_dec, Rounding rounding, std::size_t dest_bits) noexcept;

std::strong_ordering
mixed_compare_kernel(wide::int256 a_raw, unsigned a_dec,
                     wide::int256 b_raw, unsigned b_dec) noexcept;



} // namespace detail

// Automatic exact comparison across different widths/scales
template<std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db>
    requires (BitsA != BitsB || Da != Db)
[[nodiscard]] constexpr bool operator==(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b) noexcept {
    return (a <=> b) == std::strong_ordering::equal;
}

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

// 1. fixed_cast
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

// 2. add_to
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

// 3. sub_to
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

// 4. mul_to
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

// 5. div_to
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

// 6. mul_div_to
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
