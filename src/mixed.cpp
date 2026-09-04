#include <fixedwide/mixed.hpp>
#include "detail.hpp"
#include "limbs.hpp"
#include <algorithm>

namespace fixedwide::detail {

namespace {

// Every kernel in this file used to do all of its arithmetic in u1024_limbs --
// sixteen 64-bit limbs -- no matter how small the operands were. A mixed
// multiply of two Fixed64 values into a Fixed128 needs about eighty bits, and
// it was running a 16x16 schoolbook multiply: 256 partial products, of which
// four were not multiplying by zero. That is where the 160x cliff between the
// native path and this one came from.
//
// The width is now chosen from the operands. Everything below is templated on
// the limb count, and each kernel computes an upper bound on the bits it needs
// and dispatches to the smallest tier that holds them. The 1024-bit path still
// exists and is still correct; it is now reached only by operands that need it.

/// Upper bound on the bit width of 10^e. log2(10) < 10/3, so e*10/3 + 1 never
/// under-estimates, which is the only direction that would be a correctness bug.
[[nodiscard]] constexpr unsigned pow10_bits(unsigned e) noexcept {
    return e * 10u / 3u + 1u;
}

template<std::size_t L>
[[nodiscard]] uint_limbs<L> widen(wide::uint256 val) noexcept {
    uint_limbs<L> res{};
    for (std::size_t i = 0; i < (L < 4 ? L : std::size_t{4}); ++i) res.limbs[i] = val.limbs[i];
    return res;
}

template<std::size_t L>
[[nodiscard]] uint_limbs<L> pow10_at(unsigned exp) noexcept {
    uint_limbs<L> v(1ULL);
    for (unsigned i = 0; i < exp; ++i) v = (v << 3) + (v << 1);
    return v;
}

/// num / den, rounded, range-checked against the destination.
///
/// The quotient is compared against the destination limit in 256 bits rather
/// than at width L, because the result has to fit `wide::int256` to be returned
/// at all. That also makes the check correct for L < 4, where the limit itself
/// would not fit the working width.
template<std::size_t L>
[[nodiscard]] std::expected<wide::int256, ArithmeticError>
evaluate_rational(bool negative, const uint_limbs<L>& num, const uint_limbs<L>& den, Rounding rounding,
                  std::size_t dest_bits) noexcept {
    if (den.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);

    const auto divres = divmod_knuth(num, den);
    const auto limit = widen<L>(detail::limit_magnitude_u256(dest_bits, negative));

    if (divres.quotient > limit) return std::unexpected(ArithmeticError::overflow);

    const auto rounded = round_magnitude(divres.quotient, divres.remainder, den, negative, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());

    wide::uint256 uq{};
    for (std::size_t i = 0; i < (L < 4 ? L : std::size_t{4}); ++i) uq.limbs[i] = rounded->limbs[i];
    if (negative) {
        const wide::uint256 neg_uq = ~uq + wide::uint256(1ULL);
        return wide::int256(neg_uq.limbs[0], neg_uq.limbs[1], neg_uq.limbs[2], neg_uq.limbs[3]);
    }
    return wide::int256(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
}

/// Call `body.operator()<L>()` with the smallest limb count that holds `bits`.
///
/// The tiers are 128 / 256 / 512 / 1024 bits. Anything wider than the operands
/// need is wasted work in every loop below, and anything narrower is a wrong
/// answer, so the bound each caller passes must be an over-estimate.
template<class Body>
[[nodiscard]] auto with_width(unsigned bits, Body&& body) noexcept {
    if (bits <= 128) return body.template operator()<2>();
    if (bits <= 256) return body.template operator()<4>();
    if (bits <= 512) return body.template operator()<8>();
    return body.template operator()<16>();
}

/// Significant bits of a raw value's magnitude.
[[nodiscard]] unsigned magnitude_bits(wide::int256 raw) noexcept {
    return widen<4>(magnitude(raw)).bit_width();
}

} // namespace

std::strong_ordering mixed_compare_kernel(wide::int256 a_raw, unsigned a_decimals, wide::int256 b_raw,
                                          unsigned b_decimals) noexcept {
    const bool a_neg = a_raw.is_negative();
    const bool b_neg = b_raw.is_negative();
    if (a_neg != b_neg) return a_neg ? std::strong_ordering::less : std::strong_ordering::greater;
    if (a_raw.is_zero() && b_raw.is_zero()) return std::strong_ordering::equal;

    const unsigned common = std::min(a_decimals, b_decimals);
    const unsigned expA = b_decimals - common;
    const unsigned expB = a_decimals - common;

    const unsigned bits = std::max(magnitude_bits(a_raw) + pow10_bits(expA), magnitude_bits(b_raw) + pow10_bits(expB));

    const auto cmp = with_width(bits, [&]<std::size_t L>() {
        const auto lhs = widen<L>(magnitude(a_raw)) * pow10_at<L>(expA);
        const auto rhs = widen<L>(magnitude(b_raw)) * pow10_at<L>(expB);
        return lhs <=> rhs;
    });

    if (!a_neg) return cmp;
    // Both negative: the larger magnitude is the smaller value.
    if (cmp == std::strong_ordering::less) return std::strong_ordering::greater;
    if (cmp == std::strong_ordering::greater) return std::strong_ordering::less;
    return std::strong_ordering::equal;
}

std::expected<wide::int256, ArithmeticError> mixed_cast_kernel(wide::int256 src_raw, unsigned src_decimals,
                                                               unsigned dest_decimals, Rounding rounding,
                                                               std::size_t dest_bits) noexcept {
    const unsigned up = dest_decimals > src_decimals ? dest_decimals - src_decimals : 0u;
    const unsigned down = src_decimals > dest_decimals ? src_decimals - dest_decimals : 0u;
    const unsigned bits =
        std::max({magnitude_bits(src_raw) + pow10_bits(up), pow10_bits(down), static_cast<unsigned>(dest_bits)});

    return with_width(bits, [&]<std::size_t L>() {
        const auto num = widen<L>(magnitude(src_raw)) * pow10_at<L>(up);
        const auto den = pow10_at<L>(down);
        return evaluate_rational<L>(src_raw.is_negative(), num, den, rounding, dest_bits);
    });
}

std::expected<wide::int256, ArithmeticError> mixed_add_sub_kernel(wide::int256 a_raw, unsigned a_decimals,
                                                                  wide::int256 b_raw, unsigned b_decimals,
                                                                  bool subtract, unsigned dest_decimals,
                                                                  Rounding rounding, std::size_t dest_bits) noexcept {
    const unsigned common = std::min(a_decimals, b_decimals);
    const unsigned expA = b_decimals - common;
    const unsigned expB = a_decimals - common;
    const unsigned base_scale = std::max(a_decimals, b_decimals);
    const unsigned up = dest_decimals > base_scale ? dest_decimals - base_scale : 0u;
    const unsigned down = base_scale > dest_decimals ? base_scale - dest_decimals : 0u;

    // +1 bit for the sum of the two aligned terms.
    const unsigned aligned =
        std::max(magnitude_bits(a_raw) + pow10_bits(expA), magnitude_bits(b_raw) + pow10_bits(expB)) + 1u;
    const unsigned bits = std::max({aligned + pow10_bits(up), pow10_bits(down), static_cast<unsigned>(dest_bits)});

    return with_width(bits, [&]<std::size_t L>() {
        const auto termA = widen<L>(magnitude(a_raw)) * pow10_at<L>(expA);
        const auto termB = widen<L>(magnitude(b_raw)) * pow10_at<L>(expB);

        const bool a_neg = a_raw.is_negative();
        const bool b_neg = subtract ? !b_raw.is_negative() : b_raw.is_negative();

        uint_limbs<L> sum_mag{};
        bool sum_neg = false;
        if (a_neg == b_neg) {
            sum_mag = termA + termB;
            sum_neg = a_neg;
        } else if (termA >= termB) {
            sum_mag = termA - termB;
            sum_neg = a_neg;
        } else {
            sum_mag = termB - termA;
            sum_neg = b_neg;
        }

        const auto num = sum_mag * pow10_at<L>(up);
        const auto den = pow10_at<L>(down);
        return evaluate_rational<L>(sum_neg, num, den, rounding, dest_bits);
    });
}

std::expected<wide::int256, ArithmeticError> mixed_mul_kernel(wide::int256 a_raw, unsigned a_decimals,
                                                              wide::int256 b_raw, unsigned b_decimals,
                                                              unsigned dest_decimals, Rounding rounding,
                                                              std::size_t dest_bits) noexcept {
    const unsigned product_scale = a_decimals + b_decimals;
    const unsigned up = dest_decimals > product_scale ? dest_decimals - product_scale : 0u;
    const unsigned down = product_scale > dest_decimals ? product_scale - dest_decimals : 0u;
    const unsigned bits = std::max({magnitude_bits(a_raw) + magnitude_bits(b_raw) + pow10_bits(up), pow10_bits(down),
                                    static_cast<unsigned>(dest_bits)});

    return with_width(bits, [&]<std::size_t L>() {
        const auto num = widen<L>(magnitude(a_raw)) * widen<L>(magnitude(b_raw)) * pow10_at<L>(up);
        const auto den = pow10_at<L>(down);
        return evaluate_rational<L>(a_raw.is_negative() != b_raw.is_negative(), num, den, rounding, dest_bits);
    });
}

std::expected<wide::int256, ArithmeticError> mixed_div_kernel(wide::int256 a_raw, unsigned a_decimals,
                                                              wide::int256 b_raw, unsigned b_decimals,
                                                              unsigned dest_decimals, Rounding rounding,
                                                              std::size_t dest_bits) noexcept {
    if (b_raw.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);

    const int exp = static_cast<int>(b_decimals) + static_cast<int>(dest_decimals) - static_cast<int>(a_decimals);
    const unsigned up = exp > 0 ? static_cast<unsigned>(exp) : 0u;
    const unsigned down = exp < 0 ? static_cast<unsigned>(-exp) : 0u;
    const unsigned bits = std::max({magnitude_bits(a_raw) + pow10_bits(up), magnitude_bits(b_raw) + pow10_bits(down),
                                    static_cast<unsigned>(dest_bits)});

    return with_width(bits, [&]<std::size_t L>() {
        const auto num = widen<L>(magnitude(a_raw)) * pow10_at<L>(up);
        const auto den = widen<L>(magnitude(b_raw)) * pow10_at<L>(down);
        return evaluate_rational<L>(a_raw.is_negative() != b_raw.is_negative(), num, den, rounding, dest_bits);
    });
}

std::expected<wide::int256, ArithmeticError> mixed_mul_div_kernel(wide::int256 a_raw, unsigned a_decimals,
                                                                  wide::int256 b_raw, unsigned b_decimals,
                                                                  wide::int256 c_raw, unsigned c_decimals,
                                                                  unsigned dest_decimals, Rounding rounding,
                                                                  std::size_t dest_bits) noexcept {
    if (c_raw.is_zero()) return std::unexpected(ArithmeticError::division_by_zero);

    // (a*b/c) at dest_decimals: numerator carries a*b scaled up by
    // dest_decimals + c_decimals, denominator carries c scaled by a+b's.
    const unsigned product_scale = a_decimals + b_decimals;
    const unsigned target = dest_decimals + c_decimals;
    const unsigned up = target > product_scale ? target - product_scale : 0u;
    const unsigned down = product_scale > target ? product_scale - target : 0u;

    const unsigned bits = std::max({magnitude_bits(a_raw) + magnitude_bits(b_raw) + pow10_bits(up),
                                    magnitude_bits(c_raw) + pow10_bits(down), static_cast<unsigned>(dest_bits)});

    const bool negative = (a_raw.is_negative() != b_raw.is_negative()) != c_raw.is_negative();
    return with_width(bits, [&]<std::size_t L>() {
        const auto num = widen<L>(magnitude(a_raw)) * widen<L>(magnitude(b_raw)) * pow10_at<L>(up);
        const auto den = widen<L>(magnitude(c_raw)) * pow10_at<L>(down);
        return evaluate_rational<L>(negative, num, den, rounding, dest_bits);
    });
}

} // namespace fixedwide::detail
