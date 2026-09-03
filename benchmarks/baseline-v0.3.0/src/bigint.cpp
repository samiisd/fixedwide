#include <fixedwide/bigint.hpp>
#include "division.hpp"

namespace fixedwide::detail {

FullQuotient divide_unsigned(u256 numerator, u128 divisor) noexcept {
    if (numerator < static_cast<u256>(divisor)) return {0, static_cast<u128>(numerator)};
    const auto dlow = static_cast<std::uint64_t>(divisor);
    const auto dhigh = static_cast<std::uint64_t>(divisor >> 64);
    if (dhigh == 0) {
        // A one-limb divisor never needs normalization or quotient correction.
        if ((numerator >> 128) == 0) {
            const auto result = divide128by64(static_cast<u128>(numerator), dlow);
            return {static_cast<u256>(result.quotient), result.remainder};
        }
        u256 quotient = 0;
        std::uint64_t remainder = 0;
        for (int i = 3; i >= 0; --i) {
            const auto limb = static_cast<std::uint64_t>(numerator >> (i * 64));
            const auto digit = div128by64(remainder, limb, dlow, remainder);
            quotient |= static_cast<u256>(digit) << (i * 64);
        }
        return {quotient, remainder};
    }

    if ((numerator >> 128) == 0) {
        const auto result = divide128(static_cast<u128>(numerator), divisor);
        return {static_cast<u256>(result.quotient), result.remainder};
    }

    // Knuth's radix-2^64 division specialized to a TWO-limb divisor. Normalizing
    // its high bit bounds each quotient estimate to at most two decrements.
    // Unlike the old (remainder << 64) path, the 192-bit partial dividend is
    // represented by THREE limbs; no significant remainder bits are discarded.
    std::uint64_t u[5]{};
    const unsigned words = (numerator >> 192) == 0 ? 3 : 4;
    for (unsigned i = 0; i < 4; ++i) u[i] = static_cast<std::uint64_t>(numerator >> (i * 64));
    const unsigned shift = static_cast<unsigned>(__builtin_clzll(dhigh));
    const u128 normalized_divisor = divisor << shift;
    const auto v0 = static_cast<std::uint64_t>(normalized_divisor);
    const auto v1 = static_cast<std::uint64_t>(normalized_divisor >> 64);
    if (shift != 0) {
        u[words] = u[words - 1] >> (64 - shift);
        for (unsigned i = words - 1; i != 0; --i)
            u[i] = (u[i] << shift) | (u[i - 1] >> (64 - shift));
        u[0] <<= shift;
    }

    u256 quotient = 0;
    unsigned digits = words - 1;
    // Do not estimate a leading zero quotient limb. The prefix comparison is
    // also a proof that every skipped normalized carry limb is zero.
    if (digits == 3 && static_cast<u128>(numerator >> 128) < divisor) --digits;
    if (digits == 2 && (numerator >> 64) < static_cast<u256>(divisor)) --digits;
    for (int j = static_cast<int>(digits) - 1; j >= 0; --j) {
        u128 remainder = (u128{u[j + 2]} << 64) | u[j + 1];
        const auto qhat = divide_digit(remainder, u[j], v0, v1);
        u[j] = static_cast<std::uint64_t>(remainder);
        u[j + 1] = static_cast<std::uint64_t>(remainder >> 64);
        u[j + 2] = 0;
        quotient |= static_cast<u256>(qhat) << (j * 64);
    }
    return {quotient, ((u128{u[1]} << 64) | u[0]) >> shift};
}

} // namespace fixedwide::detail

namespace fixedwide {
std::expected<UnsignedDivision, ArithmeticError> divmod(u256 numerator, u128 divisor) noexcept {
    if (divisor == 0) return std::unexpected(ArithmeticError::division_by_zero);
    const auto result = detail::divide_unsigned(numerator, divisor);
    return UnsignedDivision{result.quotient, result.remainder};
}
std::expected<SignedDivision, ArithmeticError> divmod(i256 numerator, i128 divisor) noexcept {
    if (divisor == 0) return std::unexpected(ArithmeticError::division_by_zero);
    if (numerator == i256_min && divisor == -1) return std::unexpected(ArithmeticError::overflow);
    const bool negative = (numerator < 0) != (divisor < 0);
    const auto result = detail::divide_unsigned(magnitude(numerator), magnitude(divisor));
    const i256 quotient = static_cast<i256>(negative ? u256{0} - result.quotient : result.quotient);
    return SignedDivision{quotient, detail::apply_sign(result.remainder, numerator < 0)};
}
std::expected<i128, ArithmeticError> divide_to_i128(i256 numerator, i128 divisor, Rounding rounding) noexcept {
    const u128 denominator = magnitude(divisor);
    const auto result = detail::divide_narrow(magnitude(numerator), denominator);
    if (!result) return std::unexpected(result.error());
    return detail::finish(*result, denominator, (numerator < 0) != (divisor < 0), rounding);
}
std::expected<i128, ArithmeticError> mul_div(i128 a, i128 b, i128 divisor, Rounding rounding) noexcept {
    if (divisor == 0) return std::unexpected(ArithmeticError::division_by_zero);
    const u128 denominator = magnitude(divisor);
    const u256 product = detail::multiply128(magnitude(a), magnitude(b));
    const bool negative = (a < 0) != ((b < 0) != (divisor < 0));
    if ((product >> 128) == 0) {
        return detail::finish(detail::divide128(static_cast<u128>(product), denominator,
                                                rounding != Rounding::toward_zero),
                              denominator, negative, rounding);
    }
    const auto result = detail::divide_narrow(product, denominator);
    if (!result) return std::unexpected(result.error());
    return detail::finish(*result, denominator, negative, rounding);
}
} // namespace fixedwide
