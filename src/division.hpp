#pragma once
#include "detail.hpp"
#include <fixedwide/wide.hpp>
#include <bit>

namespace fixedwide::detail {

[[nodiscard]] inline wide::uint256 multiply128(wide::uint128 a, wide::uint128 b) noexcept {
#if defined(__clang__) && defined(__x86_64__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    if (((a.high | b.high)) == 0) {
        unsigned __int128 p = static_cast<unsigned __int128>(a.low) * b.low;
        return wide::uint256(static_cast<std::uint64_t>(p), static_cast<std::uint64_t>(p >> 64), 0, 0);
    }
    using u256_internal = unsigned _BitInt(256);
    unsigned __int128 ua = (static_cast<unsigned __int128>(a.high) << 64) | a.low;
    unsigned __int128 ub = (static_cast<unsigned __int128>(b.high) << 64) | b.low;
    u256_internal res = static_cast<u256_internal>(ua) * static_cast<u256_internal>(ub);
    return wide::uint256(
        static_cast<std::uint64_t>(res),
        static_cast<std::uint64_t>(res >> 64),
        static_cast<std::uint64_t>(res >> 128),
        static_cast<std::uint64_t>(res >> 192)
    );
#elif defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    if (((a.high | b.high)) == 0) {
        unsigned __int128 p = static_cast<unsigned __int128>(a.low) * b.low;
        return wide::uint256(static_cast<std::uint64_t>(p), static_cast<std::uint64_t>(p >> 64), 0, 0);
    }
    std::uint64_t ll_h, ll_l;
    mul64x64(a.low, b.low, ll_h, ll_l);
    std::uint64_t lh_h, lh_l;
    mul64x64(a.low, b.high, lh_h, lh_l);
    std::uint64_t hl_h, hl_l;
    mul64x64(a.high, b.low, hl_h, hl_l);
    std::uint64_t hh_h, hh_l;
    mul64x64(a.high, b.high, hh_h, hh_l);

    std::uint64_t l0 = ll_l;
    std::uint64_t carry = 0;
    std::uint64_t l1 = ll_h + lh_l;
    if (l1 < ll_h) ++carry;
    l1 += hl_l;
    if (l1 < hl_l) ++carry;

    std::uint64_t l2 = hh_l + lh_h + carry;
    carry = 0;
    if (l2 < hh_l || (hh_l + lh_h < hh_l)) ++carry;
    l2 += hl_h;
    if (l2 < hl_h) ++carry;

    std::uint64_t l3 = hh_h + carry;
    return wide::uint256(l0, l1, l2, l3);
#else
    std::uint64_t ll_h, ll_l;
    mul64x64(a.low, b.low, ll_h, ll_l);
    std::uint64_t lh_h, lh_l;
    mul64x64(a.low, b.high, lh_h, lh_l);
    std::uint64_t hl_h, hl_l;
    mul64x64(a.high, b.low, hl_h, hl_l);
    std::uint64_t hh_h, hh_l;
    mul64x64(a.high, b.high, hh_h, hh_l);

    std::uint64_t l0 = ll_l;
    std::uint64_t carry = 0;
    std::uint64_t l1 = ll_h + lh_l;
    if (l1 < ll_h) ++carry;
    l1 += hl_l;
    if (l1 < hl_l) ++carry;

    std::uint64_t l2 = hh_l + lh_h + carry;
    carry = 0;
    if (l2 < hh_l || (hh_l + lh_h < hh_l)) ++carry;
    l2 += hl_h;
    if (l2 < hl_h) ++carry;

    std::uint64_t l3 = hh_h + carry;
    return wide::uint256(l0, l1, l2, l3);
#endif
}

struct FullQuotient { wide::uint256 quotient; wide::uint128 remainder; };

[[nodiscard]] inline std::uint64_t divide_digit(wide::uint128& remainder, std::uint64_t next,
                                                std::uint64_t v0, std::uint64_t v1) noexcept {
    const auto high = remainder.high;
    const auto low = remainder.low;
    std::uint64_t quotient, rhat;
    bool carry = false;
    if (high == v1) {
        quotient = ~0ULL;
        rhat = low + v1;
        carry = rhat < low;
    } else {
        quotient = div128by64(high, low, v1, rhat);
    }
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    while (!carry && (static_cast<unsigned __int128>(quotient) * v0 >
                     ((static_cast<unsigned __int128>(rhat) << 64) | next))) {
        --quotient;
        const std::uint64_t previous = rhat;
        rhat += v1;
        carry = rhat < previous;
    }
    unsigned __int128 diff = ((static_cast<unsigned __int128>(rhat) << 64) | next) -
                             static_cast<unsigned __int128>(quotient) * v0;
    remainder = wide::uint128(static_cast<std::uint64_t>(diff), static_cast<std::uint64_t>(diff >> 64));
#else
    while (!carry) {
        std::uint64_t ph, pl;
        mul64x64(quotient, v0, ph, pl);
        wide::uint128 r_comp(next, rhat);
        wide::uint128 prod(pl, ph);
        if (prod <= r_comp) break;
        --quotient;
        const std::uint64_t previous = rhat;
        rhat += v1;
        carry = rhat < previous;
    }
    std::uint64_t ph, pl;
    mul64x64(quotient, v0, ph, pl);
    remainder = wide::uint128(next, rhat) - wide::uint128(pl, ph);
#endif
    return quotient;
}

[[nodiscard]] inline std::expected<Quotient128, ArithmeticError> divide_narrow(
    wide::uint256 numerator, wide::uint128 divisor) noexcept {
    if (divisor.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    unsigned __int128 div_u = (static_cast<unsigned __int128>(divisor.high) << 64) | divisor.low;
    unsigned __int128 high = (static_cast<unsigned __int128>(numerator.limbs[3]) << 64) | numerator.limbs[2];
    unsigned __int128 low = (static_cast<unsigned __int128>(numerator.limbs[1]) << 64) | numerator.limbs[0];
    if (high >= div_u) return std::unexpected(ArithmeticError::overflow);
    if (high == 0) return divide128(wide::uint128(static_cast<std::uint64_t>(low), static_cast<std::uint64_t>(low >> 64)), divisor);
    if (divisor.high == 0) {
        const auto d = divisor.low;
        auto rem = static_cast<std::uint64_t>(high);
        const auto qhigh = div128by64(rem, static_cast<std::uint64_t>(low >> 64), d, rem);
        const auto qlow = div128by64(rem, static_cast<std::uint64_t>(low), d, rem);
        return Quotient128{wide::uint128(qlow, qhigh), wide::uint128(rem, 0ULL)};
    }
    const unsigned shift = static_cast<unsigned>(std::countl_zero(divisor.high));
    const unsigned __int128 normalized_divisor = div_u << shift;
    const auto v0 = static_cast<std::uint64_t>(normalized_divisor);
    const auto v1 = static_cast<std::uint64_t>(normalized_divisor >> 64);
    unsigned __int128 remainder = shift == 0 ? high : (high << shift) | (low >> (128 - shift));
    const unsigned __int128 normalized_low = low << shift;
    const auto middle = static_cast<std::uint64_t>(normalized_low >> 64);
    std::uint64_t qhigh = 0;
    bool fits_64 = (numerator.limbs[3] == 0) &&
                   ((numerator.limbs[2] < divisor.high) ||
                    (numerator.limbs[2] == divisor.high && numerator.limbs[1] < divisor.low));
    wide::uint128 rem_w;
    if (fits_64) {
        remainder = (remainder << 64) | middle;
        rem_w = wide::uint128(static_cast<std::uint64_t>(remainder), static_cast<std::uint64_t>(remainder >> 64));
    } else {
        rem_w = wide::uint128(static_cast<std::uint64_t>(remainder), static_cast<std::uint64_t>(remainder >> 64));
        qhigh = divide_digit(rem_w, middle, v0, v1);
    }
    const auto qlow = divide_digit(rem_w, static_cast<std::uint64_t>(normalized_low), v0, v1);
    unsigned __int128 final_rem = ((static_cast<unsigned __int128>(rem_w.high) << 64) | rem_w.low) >> shift;
    return Quotient128{wide::uint128(qlow, qhigh),
                       wide::uint128(static_cast<std::uint64_t>(final_rem), static_cast<std::uint64_t>(final_rem >> 64))};
#else
    const wide::uint128 high(numerator.limbs[2], numerator.limbs[3]);
    const wide::uint128 low(numerator.limbs[0], numerator.limbs[1]);
    if (high >= divisor) return std::unexpected(ArithmeticError::overflow);
    if (high.is_zero()) return divide128(low, divisor);
    if (divisor.high == 0) {
        const auto d = divisor.low;
        auto rem = high.low;
        const auto qhigh = div128by64(rem, low.high, d, rem);
        const auto qlow = div128by64(rem, low.low, d, rem);
        return Quotient128{wide::uint128(qlow, qhigh), wide::uint128(rem, 0ULL)};
    }
    const unsigned shift = static_cast<unsigned>(std::countl_zero(divisor.high));
    const wide::uint128 normalized_divisor = divisor << shift;
    const auto v0 = normalized_divisor.low;
    const auto v1 = normalized_divisor.high;
    wide::uint128 remainder = shift == 0 ? high : (high << shift) | (low >> (128 - shift));
    const wide::uint128 normalized_low = low << shift;
    const auto middle = normalized_low.high;
    std::uint64_t qhigh = 0;
    bool fits_64 = (numerator.limbs[3] == 0) &&
                   ((numerator.limbs[2] < divisor.high) ||
                    (numerator.limbs[2] == divisor.high && numerator.limbs[1] < divisor.low));
    if (fits_64) {
        remainder = (remainder << 64) | wide::uint128(middle, 0ULL);
    } else {
        qhigh = divide_digit(remainder, middle, v0, v1);
    }
    const auto qlow = divide_digit(remainder, normalized_low.low, v0, v1);
    return Quotient128{wide::uint128(qlow, qhigh), remainder >> shift};
#endif
}

} // namespace fixedwide::detail
