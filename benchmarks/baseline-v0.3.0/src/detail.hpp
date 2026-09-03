#pragma once
#include <fixedwide/error.hpp>
#include <fixedwide/types.hpp>
#include <expected>

namespace fixedwide::detail {
struct Quotient128 { u128 quotient; u128 remainder; };

// x86-64's 128/64 -> 64 instruction. The high < divisor precondition is essential:
// violating it raises #DE even when divisor is nonzero. Every caller proves it.
[[nodiscard]] inline std::uint64_t div128by64(std::uint64_t high, std::uint64_t low,
                                            std::uint64_t divisor, std::uint64_t& remainder) noexcept {
    std::uint64_t quotient;
    __asm__("divq %[divisor]"
            : "=a"(quotient), "=d"(remainder)
            : "a"(low), "d"(high), [divisor] "r"(divisor)
            : "cc");
    return quotient;
}

// General 128/64: first reduce the high limb, then the divq precondition holds.
[[nodiscard]] inline Quotient128 divide128by64(u128 numerator, std::uint64_t divisor) noexcept {
    const auto high = static_cast<std::uint64_t>(numerator >> 64);
    const auto low = static_cast<std::uint64_t>(numerator);
    if (high == 0) return {low / divisor, low % divisor};
    if (high < divisor) {
        std::uint64_t remainder;
        const auto quotient = div128by64(high, low, divisor, remainder);
        return {quotient, remainder};
    }
    const std::uint64_t qhigh = high / divisor;
    std::uint64_t remainder = high % divisor;
    const std::uint64_t qlow = div128by64(remainder, low, divisor, remainder);
    return {(u128{qhigh} << 64) | qlow, remainder};
}

// Use the compiler's native-width division before considering a limb algorithm.
// Recover the remainder by multiplication: this avoids a second runtime divide.
[[nodiscard]] inline Quotient128 divide128(u128 numerator, u128 divisor, bool need_remainder = true) noexcept {
    if ((divisor >> 64) == 0) return divide128by64(numerator, static_cast<std::uint64_t>(divisor));
    if (numerator < divisor) return {0, numerator};
    // divisor >= 2^64 proves that this quotient fits one 64-bit limb.
    const auto quotient = static_cast<std::uint64_t>(numerator / divisor);
    return {quotient, need_remainder ? numerator - u128{quotient} * divisor : 0};
}

[[nodiscard]] inline std::expected<u128, ArithmeticError> round_magnitude(
    u128 quotient, u128 remainder, u128 divisor, bool negative, Rounding rounding, u128 limit) noexcept {
    if (quotient > limit) return std::unexpected(ArithmeticError::overflow);
    if (remainder == 0) return quotient;
    bool increment = false;
    switch (rounding) {
    case Rounding::toward_zero: break;
    case Rounding::floor: increment = negative; break;
    case Rounding::ceil: increment = !negative; break;
    case Rounding::nearest_even:
        // Do not compute 2*remainder: that can overflow even though r < d.
        increment = remainder > divisor - remainder ||
                    (remainder == divisor - remainder && (quotient & 1) != 0);
        break;
    case Rounding::nearest_away: increment = remainder >= divisor - remainder; break;
    case Rounding::exact: return std::unexpected(ArithmeticError::inexact);
    }
    if (increment && quotient == limit) return std::unexpected(ArithmeticError::overflow);
    return quotient + static_cast<unsigned>(increment);
}

[[nodiscard]] inline i128 apply_sign(u128 value, bool negative) noexcept {
    // Negation happens in unsigned arithmetic, including magnitude 2^127.
    return static_cast<i128>(negative ? u128{0} - value : value);
}
[[nodiscard]] inline std::expected<i128, ArithmeticError> finish(
    Quotient128 value, u128 divisor, bool negative, Rounding rounding,
    u128 positive_limit = static_cast<u128>(i128_max)) noexcept {
    const auto rounded = round_magnitude(value.quotient, value.remainder, divisor,
                                        negative, rounding, positive_limit + static_cast<unsigned>(negative));
    if (!rounded) return std::unexpected(rounded.error());
    return apply_sign(*rounded, negative);
}
[[nodiscard]] constexpr std::uint64_t pow10(unsigned exponent) noexcept {
    std::uint64_t value = 1;
    for (unsigned i = 0; i < exponent; ++i) value *= 10;
    return value;
}
} // namespace fixedwide::detail
