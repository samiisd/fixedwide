#pragma once
#include "detail.hpp"
#include <fixedwide/wide_integer.hpp>

namespace fixedwide::detail {
// Calculate the full product ONCE. Probing signed 128-bit overflow and then
// multiplying again at 256 bits duplicated expensive work in measured builds.
[[nodiscard]] inline u256 multiply128(u128 a, u128 b) noexcept {
    if (((a | b) >> 64) == 0)
        return static_cast<u256>(u128{static_cast<std::uint64_t>(a)} * static_cast<std::uint64_t>(b));
    return static_cast<u256>(a) * static_cast<u256>(b);
}
struct FullQuotient { u256 quotient; u128 remainder; };
// Nonzero divisor required. Returns the full (up to 256-bit) quotient.
[[nodiscard]] FullQuotient divide_unsigned(u256 numerator, u128 divisor) noexcept;
// Append one radix-2^64 digit to a remainder smaller than the normalized
// two-limb divisor. Returns one quotient digit and replaces the remainder.
[[nodiscard]] inline std::uint64_t divide_digit(u128& remainder, std::uint64_t next,
                                                std::uint64_t v0, std::uint64_t v1) noexcept {
    const auto high = static_cast<std::uint64_t>(remainder >> 64);
    const auto low = static_cast<std::uint64_t>(remainder);
    std::uint64_t quotient, rhat;
    bool carry = false;
    if (high == v1) {
        quotient = UINT64_MAX;
        rhat = low + v1;
        carry = rhat < low;
    } else {
        quotient = div128by64(high, low, v1, rhat);
    }
    while (!carry && u128{quotient} * v0 > ((u128{rhat} << 64) | next)) {
        --quotient;
        const std::uint64_t previous = rhat;
        rhat += v1;
        carry = rhat < previous;
    }
    // For a TWO-limb divisor, this correction checks the entire product.
    // No general Algorithm-D add-back stage is needed. If rhat carried, the
    // unrepresented 2^128 cancels the product borrow; unsigned subtraction
    // still produces the exact remainder, known to be < normalized divisor.
    remainder = ((u128{rhat} << 64) | next) - u128{quotient} * v0;
    return quotient;
}

// Private and inline: callers in .cpp files can specialize the constant-scale
// case without copying 256-bit temporaries through an extra function-call ABI.
[[nodiscard, gnu::always_inline]] inline std::expected<Quotient128, ArithmeticError> divide_narrow(u256 numerator, u128 divisor) noexcept {
    if (divisor == 0) return std::unexpected(ArithmeticError::division_by_zero);
    const u128 high = static_cast<u128>(numerator >> 128);
    const u128 low = static_cast<u128>(numerator);
    // floor(N/d) < 2^128 iff floor(N/2^128) < d: exact, not a heuristic.
    if (high >= divisor) return std::unexpected(ArithmeticError::overflow);
    if (high == 0) return divide128(low, divisor);
    if ((divisor >> 64) == 0) {
        const auto d = static_cast<std::uint64_t>(divisor);
        // high < d <= 2^64-1: exactly two divq instructions suffice.
        auto remainder = static_cast<std::uint64_t>(high);
        const auto qhigh = div128by64(remainder, static_cast<std::uint64_t>(low >> 64), d, remainder);
        const auto qlow = div128by64(remainder, static_cast<std::uint64_t>(low), d, remainder);
        return Quotient128{(u128{qhigh} << 64) | qlow, remainder};
    }
    const unsigned shift = static_cast<unsigned>(__builtin_clzll(static_cast<std::uint64_t>(divisor >> 64)));
    const u128 normalized_divisor = divisor << shift;
    const auto v0 = static_cast<std::uint64_t>(normalized_divisor);
    const auto v1 = static_cast<std::uint64_t>(normalized_divisor >> 64);
    u128 remainder = shift == 0 ? high : (high << shift) | (low >> (128 - shift));
    const u128 normalized_low = low << shift;
    const auto middle = static_cast<std::uint64_t>(normalized_low >> 64);
    std::uint64_t qhigh = 0;
    if ((numerator >> 64) < static_cast<u256>(divisor)) {
        // The quotient fits 64 bits; skip its known-zero leading digit.
        remainder = (remainder << 64) | middle;
    } else {
        qhigh = divide_digit(remainder, middle, v0, v1);
    }
    const auto qlow = divide_digit(remainder, static_cast<std::uint64_t>(normalized_low), v0, v1);
    return Quotient128{(u128{qhigh} << 64) | qlow, remainder >> shift};
}
} // namespace fixedwide::detail
