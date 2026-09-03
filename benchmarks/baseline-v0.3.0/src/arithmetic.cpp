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
// This signed instruction requires a signed-64 quotient. The caller proves
// the interval first; it is never an unchecked arithmetic API.
std::int64_t div_signed64(i128 numerator, std::int64_t divisor) noexcept {
    std::int64_t quotient, remainder;
    __asm__("idivq %[divisor]" : "=a"(quotient), "=d"(remainder)
        : "a"(static_cast<std::uint64_t>(numerator)),
          "d"(static_cast<std::uint64_t>(static_cast<u128>(numerator) >> 64)),
          [divisor] "r"(divisor) : "cc");
    return quotient;
}
bool quotient_fits_signed64(i128 numerator, std::int64_t divisor) noexcept {
    const auto bits = static_cast<std::uint64_t>(divisor);
    const auto magnitude = divisor < 0 ? std::uint64_t{0} - bits : bits;
    const auto half = magnitude / 2;
    const auto high = static_cast<std::uint64_t>(static_cast<u128>(numerator) >> 64);
    // Write N = signed_high * 2^64 + low and h = floor(abs(d)/2).
    // -h <= signed_high < h proves -h*2^64 <= N < h*2^64, hence a
    // signed-64 quotient for positive d. The unsigned addition below encodes
    // that signed interval without signed overflow. For negative d we exclude
    // the whole lowest slab (conservatively), so +2^63 is impossible as well.
    // Odd d, abs(d)=1, and boundary values may fall back even when valid.
    return high + half < 2 * half && !(divisor < 0 && high + half == 0);
}
[[gnu::noinline]] std::expected<FP64, ArithmeticError> quotient64_general(
    i128 numerator, std::int64_t divisor, Rounding rounding) noexcept {
    return quotient64(magnitude(numerator), static_cast<std::uint64_t>(magnitude(i128{divisor})),
        (numerator < 0) != (divisor < 0), rounding);
}
std::expected<FP64, ArithmeticError> quotient64_signed(
    i128 numerator, std::int64_t divisor, Rounding rounding) noexcept {
    if (rounding == Rounding::toward_zero && quotient_fits_signed64(numerator, divisor))
        return FP64::from_raw(div_signed64(numerator, divisor));
    return quotient64_general(numerator, divisor, rounding);
}
// Keep the proven 64x64 multiply as one signed full-width instruction.
// Clang 17 otherwise re-widens operands after the FP128 range test and emits
// a 128x128 multiply with redundant cross-products (see performance notes).
i128 multiply64(std::int64_t a, std::int64_t b) noexcept {
    std::uint64_t low, high;
    __asm__("imulq %[rhs]" : "=a"(low), "=d"(high)
        : "a"(a), [rhs] "r"(b) : "cc");
    return static_cast<i128>((u128{high} << 64) | low);
}
bool fits64(i128 value) noexcept { return value == static_cast<std::int64_t>(value); }
// Callers prove divisor != 0 and numerator != signed128_min before entering.
// The 64x64 product is in [-2^126+2^63, 2^126], so it satisfies that proof.
i128 divide_native(i128 numerator, i128 divisor) noexcept {
    if (fits64(divisor) && quotient_fits_signed64(numerator, static_cast<std::int64_t>(divisor)))
        return div_signed64(numerator, static_cast<std::int64_t>(divisor));
    return numerator / divisor;
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
    return quotient64_signed(product, scale, rounding);
}
[[gnu::noinline]] static std::expected<FP128, ArithmeticError> mul_general(FP128 a, FP128 b, Rounding rounding) noexcept {
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
std::expected<FP128, ArithmeticError> mul(FP128 a, FP128 b, Rounding rounding) noexcept {
    if (rounding == Rounding::toward_zero && fits64(a.raw()) && fits64(b.raw())) {
        const i128 product = multiply64(static_cast<std::int64_t>(a.raw()), static_cast<std::int64_t>(b.raw()));
        if constexpr (fractional_digits <= 9) {
            // At these scales small economic operands commonly produce a
            // signed-64 product. Preserve the compiler's constant-division
            // optimization instead of forcing an unnecessary idivq.
            if (fits64(product)) return FP128::from_raw(static_cast<std::int64_t>(product) / scale);
        }
        return FP128::from_raw(divide_native(product, scale));
    }
    return mul_general(a, b, rounding);
}
std::expected<FP128, ArithmeticError> mul_wide(FP64 a, FP64 b, Rounding rounding) noexcept {
    const i128 product = i128{a.raw()} * b.raw();
    return wrap<FP128>(detail::finish(detail::divide128by64(magnitude(product), scale), scale,
                                      product < 0, rounding));
}
std::expected<FP64, ArithmeticError> div(FP64 a, FP64 b, Rounding rounding) noexcept {
    if (b.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
    return quotient64_signed(i128{a.raw()} * scale, b.raw(), rounding);
}
// The public entry point has already proved that b is nonzero.
[[gnu::noinline]] static std::expected<FP128, ArithmeticError> div_general(FP128 a, FP128 b, Rounding rounding) noexcept {
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
std::expected<FP128, ArithmeticError> div(FP128 a, FP128 b, Rounding rounding) noexcept {
    if (b.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
    // A range check before multiplication avoids forming or copying a 256-bit
    // intermediate on the common path, and avoids multiplying twice on fallback.
    constexpr u128 bound = static_cast<u128>(i128_max / scale);
    if (rounding == Rounding::toward_zero && static_cast<u128>(a.raw()) + bound <= 2 * bound) {
        static_assert(scale % 5 == 0);
        // scale contains a factor of five: a*scale cannot be signed_min.
        // Consequently signed_min / -1 is impossible in this branch.
        return FP128::from_raw((a.raw() * scale) / b.raw());
    }
    return div_general(a, b, rounding);
}
std::expected<FP64, ArithmeticError> mul_div(FP64 a, FP64 b, FP64 divisor, Rounding rounding) noexcept {
    if (divisor.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
    return quotient64_signed(i128{a.raw()} * b.raw(), divisor.raw(), rounding);
}
std::expected<FP128, ArithmeticError> mul_div(FP128 a, FP128 b, FP128 divisor, Rounding rounding) noexcept {
    if (divisor.raw() == 0) return std::unexpected(ArithmeticError::division_by_zero);
    if (rounding == Rounding::toward_zero && fits64(a.raw()) && fits64(b.raw()))
        return FP128::from_raw(divide_native(multiply64(static_cast<std::int64_t>(a.raw()), static_cast<std::int64_t>(b.raw())), divisor.raw()));
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
