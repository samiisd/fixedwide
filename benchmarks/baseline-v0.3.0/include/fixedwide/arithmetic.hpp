#pragma once
#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <expected>

namespace fixedwide {
inline namespace FIXEDWIDE_SCALE_NAMESPACE {
// Keep the primitive operations inline: they are only a native operation + flag test.
[[nodiscard]] constexpr std::expected<FP64, ArithmeticError> add(FP64 a, FP64 b) noexcept {
    std::int64_t result;
    if (__builtin_add_overflow(a.raw(), b.raw(), &result)) return std::unexpected(ArithmeticError::overflow);
    return FP64::from_raw(result);
}
[[nodiscard]] constexpr std::expected<FP128, ArithmeticError> add(FP128 a, FP128 b) noexcept {
    i128 result;
    if (__builtin_add_overflow(a.raw(), b.raw(), &result)) return std::unexpected(ArithmeticError::overflow);
    return FP128::from_raw(result);
}
[[nodiscard]] constexpr std::expected<FP64, ArithmeticError> sub(FP64 a, FP64 b) noexcept {
    std::int64_t result;
    if (__builtin_sub_overflow(a.raw(), b.raw(), &result)) return std::unexpected(ArithmeticError::overflow);
    return FP64::from_raw(result);
}
[[nodiscard]] constexpr std::expected<FP128, ArithmeticError> sub(FP128 a, FP128 b) noexcept {
    i128 result;
    if (__builtin_sub_overflow(a.raw(), b.raw(), &result)) return std::unexpected(ArithmeticError::overflow);
    return FP128::from_raw(result);
}
[[nodiscard]] constexpr std::expected<FP64, ArithmeticError> negate(FP64 value) noexcept { return sub(FP64{}, value); }
[[nodiscard]] constexpr std::expected<FP128, ArithmeticError> negate(FP128 value) noexcept { return sub(FP128{}, value); }
[[nodiscard]] constexpr std::expected<FP64, ArithmeticError> abs(FP64 value) noexcept {
    return value.raw() < 0 ? negate(value) : std::expected<FP64, ArithmeticError>(value);
}
[[nodiscard]] constexpr std::expected<FP128, ArithmeticError> abs(FP128 value) noexcept {
    return value.raw() < 0 ? negate(value) : std::expected<FP128, ArithmeticError>(value);
}
[[nodiscard]] constexpr std::expected<FP64, ArithmeticError> from_integer64(std::int64_t value) noexcept {
    std::int64_t result;
    if (__builtin_mul_overflow(value, scale, &result)) return std::unexpected(ArithmeticError::overflow);
    return FP64::from_raw(result);
}
[[nodiscard]] constexpr std::expected<FP128, ArithmeticError> from_integer128(i128 value) noexcept {
    i128 result;
    if (__builtin_mul_overflow(value, i128{scale}, &result)) return std::unexpected(ArithmeticError::overflow);
    return FP128::from_raw(result);
}
[[nodiscard]] constexpr std::expected<FP64, ArithmeticError> narrow(FP128 value) noexcept {
    if (value.raw() < INT64_MIN || value.raw() > INT64_MAX) return std::unexpected(ArithmeticError::overflow);
    return FP64::from_raw(static_cast<std::int64_t>(value.raw()));
}

[[nodiscard]] std::expected<FP64, ArithmeticError> mul(FP64, FP64, Rounding) noexcept;
[[nodiscard]] std::expected<FP128, ArithmeticError> mul(FP128, FP128, Rounding) noexcept;
[[nodiscard]] std::expected<FP64, ArithmeticError> div(FP64, FP64, Rounding) noexcept;
[[nodiscard]] std::expected<FP128, ArithmeticError> div(FP128, FP128, Rounding) noexcept;
// A * B / C with ONE rounding, no intermediate fixed-point overflow or double rounding.
[[nodiscard]] std::expected<FP64, ArithmeticError> mul_div(FP64, FP64, FP64, Rounding) noexcept;
[[nodiscard]] std::expected<FP128, ArithmeticError> mul_div(FP128, FP128, FP128, Rounding) noexcept;
[[nodiscard]] std::expected<FP128, ArithmeticError> mul_wide(FP64, FP64, Rounding) noexcept;
[[nodiscard]] std::expected<FP64, ArithmeticError> remainder(FP64, FP64) noexcept;
[[nodiscard]] std::expected<FP128, ArithmeticError> remainder(FP128, FP128) noexcept;
// Round to decimal places while RETAINING the storage scale. Result may overflow.
[[nodiscard]] std::expected<FP64, ArithmeticError> quantize(FP64, unsigned digits, Rounding) noexcept;
[[nodiscard]] std::expected<FP128, ArithmeticError> quantize(FP128, unsigned digits, Rounding) noexcept;
} // inline namespace
} // namespace fixedwide
