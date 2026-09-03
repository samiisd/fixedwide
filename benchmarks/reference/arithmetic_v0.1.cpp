#include <fixedwide/arithmetic.hpp>
#include <fixedwide/bigint.hpp>
#include "division.hpp"

namespace fixedwide {
inline namespace FIXEDWIDE_SCALE_NAMESPACE {
namespace {
template<class FP>
std::expected<FP, ArithmeticError> wrap(std::expected<i128, ArithmeticError> result) noexcept {
    if (!result) return std::unexpected(result.error());
    return FP::from_raw(static_cast<decltype(FP{}.raw())>(*result));
}
// For a signed-64 result, any unsigned quotient >=2^64 already overflows.
// Check that before issuing divq, then inspect the sign-specific signed bound.
std::expected<FP64, ArithmeticError> quotient64(u128 numerator, std::uint64_t divisor,
                                               bool negative, Rounding rounding) noexcept {
    const auto high = static_cast<std::uint64_t>(numerator >> 64);
    if (high >= divisor) return std::unexpected(ArithmeticError::overflow);
    std::uint64_t remainder;
    const auto quotient = detail::div128by64(high, static_cast<std::uint64_t>(numerator), divisor, remainder);
    return wrap<FP64>(detail::finish({quotient, remainder}, divisor, negative, rounding, INT64_MAX));
}
template<class FP>
std::expected<FP, ArithmeticError> quantize_impl(FP value, unsigned digits, Rounding rounding) noexcept {
    if (digits > fractional_digits) return std::unexpected(ArithmeticError::invalid_precision);
    const auto divisor = detail::pow10(fractional_digits - digits);
    const auto magnitude = fixedwide::magnitude(static_cast<i128>(value.raw()));
    const bool negative = value.raw() < 0;
    constexpr u128 limit = sizeof(FP) == 8 ? static_cast<u128>(INT64_MAX) : static_cast<u128>(i128_max);
    auto rounded = detail::round_magnitude(magnitude / divisor, magnitude % divisor, divisor, negative,
                                           rounding, limit + static_cast<unsigned>(negative));
    if (!rounded) return std::unexpected(rounded.error());
    // Dividing the bound avoids overflowing the attempted rescaling itself.
    if (*rounded > (limit + static_cast<unsigned>(negative)) / divisor)
        return std::unexpected(ArithmeticError::overflow);
    return FP::from_raw(static_cast<decltype(value.raw())>(detail::apply_sign(*rounded * divisor, negative)));
}
template<class FP>
std::expected<FP, ArithmeticError> remainder_impl(FP a, FP b) noexcept {
    if (b.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
    // The mathematical remainder is zero, even for signed_min % -1 (native UB).
    if (b.raw() == -1) return FP{};
    return FP::from_raw(a.raw() % b.raw());
}
} // namespace

std::expected<FP64, ArithmeticError> mul(FP64 a, FP64 b, Rounding rounding) noexcept {
    // A signed 64x64 product always fits signed 128 bits.
    const i128 product = i128{a.raw()} * b.raw();
    return quotient64(magnitude(product), scale, product < 0, rounding);
}
std::expected<FP128, ArithmeticError> mul(FP128 a, FP128 b, Rounding rounding) noexcept {
    const bool negative = (a.raw() < 0) != (b.raw() < 0);
    const u256 product = detail::multiply128(magnitude(a.raw()), magnitude(b.raw()));
    if ((product >> 128) == 0) {
        // Constant-scale division remains in this translation unit. The narrow
        // quotient path uses one divq; larger native quotients use two limbs.
        const auto result = detail::divide128by64(static_cast<u128>(product), scale);
        return wrap<FP128>(detail::finish(result, scale, negative, rounding));
    }
    const auto result = detail::divide_narrow(product, scale);
    if (!result) return std::unexpected(result.error());
    return wrap<FP128>(detail::finish(*result, scale, negative, rounding));
}
std::expected<FP128, ArithmeticError> mul_wide(FP64 a, FP64 b, Rounding rounding) noexcept {
    const i128 product = i128{a.raw()} * b.raw();
    return wrap<FP128>(detail::finish(detail::divide128by64(magnitude(product), scale), scale,
                                      product < 0, rounding));
}
std::expected<FP64, ArithmeticError> div(FP64 a, FP64 b, Rounding rounding) noexcept {
    if (b.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
    const u128 numerator = magnitude(i128{a.raw()}) * scale;
    const auto denominator = static_cast<std::uint64_t>(magnitude(i128{b.raw()}));
    return quotient64(numerator, denominator, (a.raw() < 0) != (b.raw() < 0), rounding);
}
std::expected<FP128, ArithmeticError> div(FP128 a, FP128 b, Rounding rounding) noexcept {
    if (b.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
    const u256 numerator = static_cast<u256>(magnitude(a.raw())) * scale;
    const u128 denominator = magnitude(b.raw());
    const bool negative = (a.raw() < 0) != (b.raw() < 0);
    if ((numerator >> 128) == 0) {
        return wrap<FP128>(detail::finish(detail::divide128(static_cast<u128>(numerator), denominator,
                                                           rounding != Rounding::toward_zero),
                                           denominator, negative, rounding));
    }
    const auto result = detail::divide_narrow(numerator, denominator);
    if (!result) return std::unexpected(result.error());
    return wrap<FP128>(detail::finish(*result, denominator, negative, rounding));
}
std::expected<FP64, ArithmeticError> mul_div(FP64 a, FP64 b, FP64 divisor, Rounding rounding) noexcept {
    if (divisor.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
    const u128 numerator = magnitude(i128{a.raw()} * b.raw());
    const auto denominator = static_cast<std::uint64_t>(magnitude(i128{divisor.raw()}));
    const bool negative = (a.raw() < 0) != ((b.raw() < 0) != (divisor.raw() < 0));
    return quotient64(numerator, denominator, negative, rounding);
}
std::expected<FP128, ArithmeticError> mul_div(FP128 a, FP128 b, FP128 divisor, Rounding rounding) noexcept {
    return wrap<FP128>(fixedwide::mul_div(a.raw(), b.raw(), divisor.raw(), rounding));
}
std::expected<FP64, ArithmeticError> remainder(FP64 a, FP64 b) noexcept { return remainder_impl(a, b); }
std::expected<FP128, ArithmeticError> remainder(FP128 a, FP128 b) noexcept { return remainder_impl(a, b); }
std::expected<FP64, ArithmeticError> quantize(FP64 value, unsigned digits, Rounding rounding) noexcept {
    return quantize_impl(value, digits, rounding);
}
std::expected<FP128, ArithmeticError> quantize(FP128 value, unsigned digits, Rounding rounding) noexcept {
    return quantize_impl(value, digits, rounding);
}
} // inline namespace
} // namespace fixedwide
