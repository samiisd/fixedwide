#include <fixedwide/mixed.hpp>
#include "detail.hpp"
#include "limbs.hpp"
#include <algorithm>

namespace fixedwide::detail {

namespace {


u1024_limbs make_u1024(wide::uint256 val) noexcept {
    u1024_limbs res{};
    for (int i = 0; i < 4; ++i) res.limbs[i] = val.limbs[i];
    return res;
}

u1024_limbs limit_magnitude_u256(std::size_t bits, bool negative) noexcept {
    auto lim256 = detail::limit_magnitude_u256(bits, negative);
    u1024_limbs lim{};
    for (int i = 0; i < 4; ++i) lim.limbs[i] = lim256.limbs[i];
    return lim;
}

std::expected<wide::int256, ArithmeticError>
evaluate_rational(bool negative, const u1024_limbs& num, const u1024_limbs& den,
                  Rounding rounding, std::size_t dest_bits) noexcept {
    if (den.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    auto divres = divmod_knuth(num, den);
    auto limit = limit_magnitude_u256(dest_bits, negative);

    if (divres.quotient > limit) return std::unexpected(ArithmeticError::overflow);

    auto rounded = round_magnitude(divres.quotient, divres.remainder, den, negative, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());

    wide::uint256 uq(rounded->limbs[0], rounded->limbs[1], rounded->limbs[2], rounded->limbs[3]);
    if (negative) {
        wide::uint256 neg_uq = ~uq + wide::uint256(1ULL);
        return wide::int256(neg_uq.limbs[0], neg_uq.limbs[1], neg_uq.limbs[2], neg_uq.limbs[3]);
    }
    return wide::int256(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
}

} // namespace

std::strong_ordering
mixed_compare_kernel(wide::int256 a_raw, unsigned a_decimals,
                     wide::int256 b_raw, unsigned b_decimals) noexcept {
    bool a_neg = a_raw.is_negative();
    bool b_neg = b_raw.is_negative();
    if (a_neg != b_neg) {
        return a_neg ? std::strong_ordering::less : std::strong_ordering::greater;
    }
    if (a_raw.is_zero() && b_raw.is_zero()) return std::strong_ordering::equal;

    unsigned m = std::min(a_decimals, b_decimals);
    unsigned expA = b_decimals - m;
    unsigned expB = a_decimals - m;

    u1024_limbs ma = make_u1024(magnitude(a_raw));
    u1024_limbs mb = make_u1024(magnitude(b_raw));

    auto sA = pow10_limbs(expA);
    auto sB = pow10_limbs(expB);

    // Multiplications within u1024_limbs
    auto prodA_full = mul_full(ma, sA);
    auto prodB_full = mul_full(mb, sB);

    u1024_limbs prodA, prodB;
    for (int i = 0; i < 16; ++i) {
        prodA.limbs[i] = prodA_full.limbs[i];
        prodB.limbs[i] = prodB_full.limbs[i];
    }

    auto cmp = prodA <=> prodB;
    if (a_neg) {
        if (cmp == std::strong_ordering::less) return std::strong_ordering::greater;
        if (cmp == std::strong_ordering::greater) return std::strong_ordering::less;
        return std::strong_ordering::equal;
    }
    return cmp;
}

std::expected<wide::int256, ArithmeticError>
mixed_cast_kernel(wide::int256 src_raw, unsigned src_decimals, unsigned dest_decimals,
                  Rounding rounding, std::size_t dest_bits) noexcept {
    bool neg = src_raw.is_negative();
    u1024_limbs num = make_u1024(magnitude(src_raw));
    u1024_limbs den(1ULL);

    if (dest_decimals > src_decimals) {
        auto s = pow10_limbs(dest_decimals - src_decimals);
        auto p = mul_full(num, s);
        for (int i = 0; i < 16; ++i) num.limbs[i] = p.limbs[i];
    } else if (dest_decimals < src_decimals) {
        den = pow10_limbs(src_decimals - dest_decimals);
    }
    return evaluate_rational(neg, num, den, rounding, dest_bits);
}

std::expected<wide::int256, ArithmeticError>
mixed_add_sub_kernel(wide::int256 a_raw, unsigned a_decimals,
                     wide::int256 b_raw, unsigned b_decimals,
                     bool subtract, unsigned dest_decimals,
                     Rounding rounding, std::size_t dest_bits) noexcept {
    unsigned m = std::min(a_decimals, b_decimals);
    unsigned expA = b_decimals - m;
    unsigned expB = a_decimals - m;
    unsigned base_scale = std::max(a_decimals, b_decimals);

    u1024_limbs ma = make_u1024(magnitude(a_raw));
    u1024_limbs mb = make_u1024(magnitude(b_raw));

    auto sA = pow10_limbs(expA);
    auto sB = pow10_limbs(expB);

    auto pA_full = mul_full(ma, sA);
    auto pB_full = mul_full(mb, sB);

    u1024_limbs termA{}, termB{};
    for (int i = 0; i < 16; ++i) {
        termA.limbs[i] = pA_full.limbs[i];
        termB.limbs[i] = pB_full.limbs[i];
    }

    bool a_neg = a_raw.is_negative();
    bool b_neg = subtract ? !b_raw.is_negative() : b_raw.is_negative();

    u1024_limbs sum_mag{};
    bool sum_neg = false;

    if (a_neg == b_neg) {
        sum_mag = termA + termB;
        sum_neg = a_neg;
    } else {
        if (termA >= termB) {
            sum_mag = termA - termB;
            sum_neg = a_neg;
        } else {
            sum_mag = termB - termA;
            sum_neg = b_neg;
        }
    }

    u1024_limbs num = sum_mag;
    u1024_limbs den(1ULL);

    if (dest_decimals >= base_scale) {
        auto s = pow10_limbs(dest_decimals - base_scale);
        auto p = mul_full(num, s);
        for (int i = 0; i < 16; ++i) num.limbs[i] = p.limbs[i];
    } else {
        den = pow10_limbs(base_scale - dest_decimals);
    }

    return evaluate_rational(sum_neg, num, den, rounding, dest_bits);
}

std::expected<wide::int256, ArithmeticError>
mixed_mul_kernel(wide::int256 a_raw, unsigned a_decimals,
                 wide::int256 b_raw, unsigned b_decimals,
                 unsigned dest_decimals, Rounding rounding, std::size_t dest_bits) noexcept {
    bool neg = a_raw.is_negative() != b_raw.is_negative();
    u1024_limbs ma = make_u1024(magnitude(a_raw));
    u1024_limbs mb = make_u1024(magnitude(b_raw));

    auto prod_full = mul_full(ma, mb);
    u1024_limbs num{};
    for (int i = 0; i < 16; ++i) num.limbs[i] = prod_full.limbs[i];

    u1024_limbs den(1ULL);
    int exp = static_cast<int>(dest_decimals) - static_cast<int>(a_decimals + b_decimals);

    if (exp >= 0) {
        auto s = pow10_limbs(static_cast<unsigned>(exp));
        auto p = mul_full(num, s);
        for (int i = 0; i < 16; ++i) num.limbs[i] = p.limbs[i];
    } else {
        den = pow10_limbs(static_cast<unsigned>(-exp));
    }

    return evaluate_rational(neg, num, den, rounding, dest_bits);
}

std::expected<wide::int256, ArithmeticError>
mixed_div_kernel(wide::int256 a_raw, unsigned a_decimals,
                 wide::int256 b_raw, unsigned b_decimals,
                 unsigned dest_decimals, Rounding rounding, std::size_t dest_bits) noexcept {
    if (b_raw.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    bool neg = a_raw.is_negative() != b_raw.is_negative();

    u1024_limbs ma = make_u1024(magnitude(a_raw));
    u1024_limbs mb = make_u1024(magnitude(b_raw));

    int exp = static_cast<int>(b_decimals) + static_cast<int>(dest_decimals) - static_cast<int>(a_decimals);
    u1024_limbs num = ma;
    u1024_limbs den = mb;

    if (exp >= 0) {
        auto s = pow10_limbs(static_cast<unsigned>(exp));
        auto p = mul_full(num, s);
        for (int i = 0; i < 16; ++i) num.limbs[i] = p.limbs[i];
    } else {
        auto s = pow10_limbs(static_cast<unsigned>(-exp));
        auto p = mul_full(den, s);
        for (int i = 0; i < 16; ++i) den.limbs[i] = p.limbs[i];
    }

    return evaluate_rational(neg, num, den, rounding, dest_bits);
}

std::expected<wide::int256, ArithmeticError>
mixed_mul_div_kernel(wide::int256 a_raw, unsigned a_decimals,
                     wide::int256 b_raw, unsigned b_decimals,
                     wide::int256 c_raw, unsigned c_decimals,
                     unsigned dest_decimals, Rounding rounding, std::size_t dest_bits) noexcept {
    if (c_raw.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    bool neg = a_raw.is_negative() != (b_raw.is_negative() != c_raw.is_negative());

    u1024_limbs ma = make_u1024(magnitude(a_raw));
    u1024_limbs mb = make_u1024(magnitude(b_raw));
    u1024_limbs mc = make_u1024(magnitude(c_raw));

    auto prod_full = mul_full(ma, mb);
    u1024_limbs num{};
    for (int i = 0; i < 16; ++i) num.limbs[i] = prod_full.limbs[i];

    u1024_limbs den = mc;
    int exp = static_cast<int>(c_decimals) + static_cast<int>(dest_decimals) - static_cast<int>(a_decimals + b_decimals);

    if (exp >= 0) {
        auto s = pow10_limbs(static_cast<unsigned>(exp));
        auto p = mul_full(num, s);
        for (int i = 0; i < 16; ++i) num.limbs[i] = p.limbs[i];
    } else {
        auto s = pow10_limbs(static_cast<unsigned>(-exp));
        auto p = mul_full(den, s);
        for (int i = 0; i < 16; ++i) den.limbs[i] = p.limbs[i];
    }

    return evaluate_rational(neg, num, den, rounding, dest_bits);
}

} // namespace fixedwide::detail
