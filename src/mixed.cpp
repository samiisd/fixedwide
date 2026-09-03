#include <fixedwide/mixed.hpp>
#include "detail.hpp"
#include "limbs.hpp"

namespace fixedwide::detail {

namespace {

u1024_limbs pow10_limbs(unsigned exp) noexcept {
    u1024_limbs v(1ULL);
    for (unsigned i = 0; i < exp; ++i) {
        v = (v << 3) + (v << 1);
    }
    return v;
}

u1024_limbs make_u1024(wide::uint256 val) noexcept {
    u1024_limbs res{};
    for (int i = 0; i < 4; ++i) res.limbs[i] = val.limbs[i];
    return res;
}

u1024_limbs limit_for_bits(std::size_t bits, bool negative) noexcept {
    u1024_limbs lim{};
    if (bits == 8) {
        lim.limbs[0] = static_cast<std::uint64_t>(INT8_MAX) + (negative ? 1 : 0);
    } else if (bits == 16) {
        lim.limbs[0] = static_cast<std::uint64_t>(INT16_MAX) + (negative ? 1 : 0);
    } else if (bits == 32) {
        lim.limbs[0] = static_cast<std::uint64_t>(INT32_MAX) + (negative ? 1 : 0);
    } else if (bits == 64) {
        lim.limbs[0] = static_cast<std::uint64_t>(INT64_MAX) + (negative ? 1 : 0);
    } else if (bits == 128) {
        lim.limbs[0] = ~0ULL;
        lim.limbs[1] = 0x7FFF'FFFF'FFFF'FFFFULL;
        if (negative) lim.limbs[0] += 1;
    } else { // 256
        lim.limbs[0] = ~0ULL;
        lim.limbs[1] = ~0ULL;
        lim.limbs[2] = ~0ULL;
        lim.limbs[3] = 0x7FFF'FFFF'FFFF'FFFFULL;
        if (negative) lim.limbs[0] += 1;
    }
    return lim;
}

std::expected<wide::int256, ArithmeticError>
evaluate_rational(bool negative, const u1024_limbs& num, const u1024_limbs& den,
                  Rounding rounding, std::size_t dest_bits) noexcept {
    if (den.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    auto divres = divmod_knuth(num, den);
    auto limit = limit_for_bits(dest_bits, negative);

    if (divres.quotient > limit) return std::unexpected(ArithmeticError::overflow);

    auto rounded = round_magnitude(divres.quotient, divres.remainder, den, negative, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());

    wide::uint256 uq(rounded->limbs[0], rounded->limbs[1], rounded->limbs[2], rounded->limbs[3]);
    if (negative) {
        wide::int256 s(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
        return -s;
    }
    return wide::int256(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
}

} // namespace

std::strong_ordering
mixed_compare_kernel(wide::int256 a_raw, unsigned a_dec,
                     wide::int256 b_raw, unsigned b_dec) noexcept {
    bool a_neg = a_raw.is_negative();
    bool b_neg = b_raw.is_negative();
    if (a_neg != b_neg) {
        return a_neg ? std::strong_ordering::less : std::strong_ordering::greater;
    }
    if (a_raw.is_zero() && b_raw.is_zero()) return std::strong_ordering::equal;

    unsigned m = std::min(a_dec, b_dec);
    unsigned expA = b_dec - m;
    unsigned expB = a_dec - m;

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
mixed_cast_kernel(wide::int256 src_raw, unsigned src_dec, unsigned dest_dec,
                  Rounding rounding, std::size_t dest_bits) noexcept {
    bool neg = src_raw.is_negative();
    u1024_limbs num = make_u1024(magnitude(src_raw));
    u1024_limbs den(1ULL);

    if (dest_dec > src_dec) {
        auto s = pow10_limbs(dest_dec - src_dec);
        auto p = mul_full(num, s);
        for (int i = 0; i < 16; ++i) num.limbs[i] = p.limbs[i];
    } else if (dest_dec < src_dec) {
        den = pow10_limbs(src_dec - dest_dec);
    }
    return evaluate_rational(neg, num, den, rounding, dest_bits);
}

std::expected<wide::int256, ArithmeticError>
mixed_add_sub_kernel(wide::int256 a_raw, unsigned a_dec,
                     wide::int256 b_raw, unsigned b_dec,
                     bool is_sub, unsigned dest_dec,
                     Rounding rounding, std::size_t dest_bits) noexcept {
    unsigned m = std::min(a_dec, b_dec);
    unsigned expA = b_dec - m;
    unsigned expB = a_dec - m;
    unsigned base_scale = std::max(a_dec, b_dec);

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
    bool b_neg = is_sub ? !b_raw.is_negative() : b_raw.is_negative();

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

    if (dest_dec >= base_scale) {
        auto s = pow10_limbs(dest_dec - base_scale);
        auto p = mul_full(num, s);
        for (int i = 0; i < 16; ++i) num.limbs[i] = p.limbs[i];
    } else {
        den = pow10_limbs(base_scale - dest_dec);
    }

    return evaluate_rational(sum_neg, num, den, rounding, dest_bits);
}

std::expected<wide::int256, ArithmeticError>
mixed_mul_kernel(wide::int256 a_raw, unsigned a_dec,
                 wide::int256 b_raw, unsigned b_dec,
                 unsigned dest_dec, Rounding rounding, std::size_t dest_bits) noexcept {
    bool neg = a_raw.is_negative() != b_raw.is_negative();
    u1024_limbs ma = make_u1024(magnitude(a_raw));
    u1024_limbs mb = make_u1024(magnitude(b_raw));

    auto prod_full = mul_full(ma, mb);
    u1024_limbs num{};
    for (int i = 0; i < 16; ++i) num.limbs[i] = prod_full.limbs[i];

    u1024_limbs den(1ULL);
    int exp = static_cast<int>(dest_dec) - static_cast<int>(a_dec + b_dec);

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
mixed_div_kernel(wide::int256 a_raw, unsigned a_dec,
                 wide::int256 b_raw, unsigned b_dec,
                 unsigned dest_dec, Rounding rounding, std::size_t dest_bits) noexcept {
    if (b_raw.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    bool neg = a_raw.is_negative() != b_raw.is_negative();

    u1024_limbs ma = make_u1024(magnitude(a_raw));
    u1024_limbs mb = make_u1024(magnitude(b_raw));

    int exp = static_cast<int>(b_dec) + static_cast<int>(dest_dec) - static_cast<int>(a_dec);
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
mixed_mul_div_kernel(wide::int256 a_raw, unsigned a_dec,
                     wide::int256 b_raw, unsigned b_dec,
                     wide::int256 c_raw, unsigned c_dec,
                     unsigned dest_dec, Rounding rounding, std::size_t dest_bits) noexcept {
    if (c_raw.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);
    bool neg = a_raw.is_negative() != (b_raw.is_negative() != c_raw.is_negative());

    u1024_limbs ma = make_u1024(magnitude(a_raw));
    u1024_limbs mb = make_u1024(magnitude(b_raw));
    u1024_limbs mc = make_u1024(magnitude(c_raw));

    auto prod_full = mul_full(ma, mb);
    u1024_limbs num{};
    for (int i = 0; i < 16; ++i) num.limbs[i] = prod_full.limbs[i];

    u1024_limbs den = mc;
    int exp = static_cast<int>(c_dec) + static_cast<int>(dest_dec) - static_cast<int>(a_dec + b_dec);

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
