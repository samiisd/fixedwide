#include <fixedwide/bigint.hpp>
#include <fixedwide/fixed.hpp>
#include "detail.hpp"
#include "limbs.hpp"

namespace fixedwide {

std::expected<UnsignedDivision, ArithmeticError> divmod(u256 numerator, u128 divisor) noexcept {
    if (divisor.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    detail::u256_limbs num(numerator);
    if (divisor.high == 0) {
        auto res = detail::divmod64(num, divisor.low);
        return UnsignedDivision{res.quotient.to_uint256(), wide::uint128(res.remainder, 0ULL)};
    }
    detail::u128_limbs den(divisor);
    auto res = detail::divmod_knuth(num, den);
    return UnsignedDivision{res.quotient.to_uint256(), res.remainder.to_uint128()};
}

std::expected<SignedDivision, ArithmeticError> divmod(i256 numerator, i128 divisor) noexcept {
    if (divisor.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    bool neg_num = numerator.is_negative();
    bool neg = neg_num != divisor.is_negative();
    auto unum = magnitude(numerator);
    auto udiv = magnitude(divisor);
    auto res = divmod(unum, udiv);
    if (!res) return std::unexpected(res.error());

    wide::int256 q(res->quotient.limbs[0], res->quotient.limbs[1], res->quotient.limbs[2], res->quotient.limbs[3]);
    if (neg) q = -q;

    wide::int128 r(res->remainder.low, res->remainder.high);
    if (neg_num) r = -r;

    return SignedDivision{q, r};
}

std::expected<i128, ArithmeticError> divide_to_i128(i256 numerator, i128 divisor, Rounding rounding) noexcept {
    if (divisor.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    bool negative = numerator.is_negative() != divisor.is_negative();
    detail::u256_limbs num(magnitude(numerator));
    detail::u128_limbs den(magnitude(divisor));
    auto divres = detail::divmod_knuth(num, den);

    auto lim256 = detail::limit_magnitude_u256(128, negative);
    detail::u256_limbs limit(wide::uint256(lim256.limbs[0], lim256.limbs[1], 0, 0));
    if (divres.quotient > limit) return std::unexpected(ArithmeticError::overflow);

    detail::u256_limbs rem256(divres.remainder);
    detail::u256_limbs den256(den);
    auto rounded = detail::round_magnitude(divres.quotient, rem256, den256, negative, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());

    wide::uint128 uq = rounded->to_uint128();
    if (negative) {
        wide::uint128 neg_uq = ~uq + wide::uint128(1ULL, 0ULL);
        return wide::int128(neg_uq.low, neg_uq.high);
    }
    return wide::int128(uq.low, uq.high);
}

std::expected<i128, ArithmeticError> mul_div(i128 a, i128 b, i128 divisor, Rounding rounding) noexcept {
    if (divisor.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    bool negative = (a.is_negative() != b.is_negative()) != divisor.is_negative();
    detail::u128_limbs ua(magnitude(a));
    detail::u128_limbs ub(magnitude(b));
    detail::u128_limbs udiv(magnitude(divisor));
    detail::u256_limbs num = detail::mul_full(ua, ub);
    auto divres = detail::divmod_knuth(num, udiv);

    auto lim256 = detail::limit_magnitude_u256(128, negative);
    detail::u256_limbs limit(wide::uint256(lim256.limbs[0], lim256.limbs[1], 0, 0));
    if (divres.quotient > limit) return std::unexpected(ArithmeticError::overflow);

    detail::u256_limbs rem256(divres.remainder);
    detail::u256_limbs den256(udiv);
    auto rounded = detail::round_magnitude(divres.quotient, rem256, den256, negative, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());

    wide::uint128 uq = rounded->to_uint128();
    if (negative) {
        wide::uint128 neg_uq = ~uq + wide::uint128(1ULL, 0ULL);
        return wide::int128(neg_uq.low, neg_uq.high);
    }
    return wide::int128(uq.low, uq.high);
}

} // namespace fixedwide
