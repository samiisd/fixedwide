#pragma once
#include <fixedwide/wide.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <cstddef>
#include <cstdint>
#include <expected>

// Compile-time evaluation path for the checked arithmetic.
//
// The runtime kernels live in compiled translation units and use inline
// assembly and native 128-bit types, so they cannot run in a constant
// expression. `mul`, `div`, `mul_div`, `quantize` and `remainder` therefore
// could not be used in one either, while `add` and `sub` could -- an
// inconsistency a caller runs into the first time they try to build a table of
// constants.
//
// This is a deliberately simple reference implementation that a constant
// expression can evaluate. It never runs at runtime: `if consteval` selects it
// only during constant evaluation, so it does not have to be fast, only
// obviously correct. Everything is schoolbook arithmetic on 64-bit limbs, wide
// enough (512 bits) for a full Fixed256 product.
//
// It is also differential-tested against the runtime kernels, which is the
// point: two independent implementations of the same contract that must agree.

namespace fixedwide::detail::ce {

inline constexpr std::size_t limb_count = 8; // 512 bits

// A plain array rather than std::array: <array> costs about 27 ms of parse time
// in every translation unit that includes arithmetic.hpp, which is more than
// this entire header, and nothing here needs an iterator.
struct limbs {
    std::uint64_t value[limb_count]{};
    [[nodiscard]] constexpr std::uint64_t& operator[](std::size_t i) noexcept { return value[i]; }
    [[nodiscard]] constexpr const std::uint64_t& operator[](std::size_t i) const noexcept { return value[i]; }
};

[[nodiscard]] constexpr limbs from_u64(std::uint64_t v) noexcept {
    limbs out{};
    out[0] = v;
    return out;
}

[[nodiscard]] constexpr bool is_zero(const limbs& a) noexcept {
    for (std::size_t i = 0; i < limb_count; ++i) {
        if (a[i] != 0) return false;
    }
    return true;
}

// -1, 0 or +1.
[[nodiscard]] constexpr int compare(const limbs& a, const limbs& b) noexcept {
    for (std::size_t i = limb_count; i-- > 0;) {
        if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
    }
    return 0;
}

[[nodiscard]] constexpr std::size_t bit_width(const limbs& a) noexcept {
    for (std::size_t i = limb_count; i-- > 0;) {
        if (a[i] == 0) continue;
        std::size_t width = 0;
        for (std::uint64_t v = a[i]; v != 0; v >>= 1) ++width;
        return i * 64 + width;
    }
    return 0;
}

[[nodiscard]] constexpr bool test_bit(const limbs& a, std::size_t bit) noexcept {
    return (a[bit / 64] >> (bit % 64)) & 1U;
}

[[nodiscard]] constexpr limbs shift_left_one(const limbs& a) noexcept {
    limbs out{};
    std::uint64_t carry = 0;
    for (std::size_t i = 0; i < limb_count; ++i) {
        out[i] = (a[i] << 1) | carry;
        carry = a[i] >> 63;
    }
    return out;
}

[[nodiscard]] constexpr limbs add(const limbs& a, const limbs& b) noexcept {
    limbs out{};
    std::uint64_t carry = 0;
    for (std::size_t i = 0; i < limb_count; ++i) {
        const std::uint64_t sum = a[i] + b[i];
        const std::uint64_t with_carry = sum + carry;
        out[i] = with_carry;
        carry = (sum < a[i] || with_carry < sum) ? 1U : 0U;
    }
    return out;
}

[[nodiscard]] constexpr limbs subtract(const limbs& a, const limbs& b) noexcept {
    limbs out{};
    std::uint64_t borrow = 0;
    for (std::size_t i = 0; i < limb_count; ++i) {
        const std::uint64_t difference = a[i] - b[i];
        const std::uint64_t with_borrow = difference - borrow;
        out[i] = with_borrow;
        borrow = (a[i] < b[i] || difference < borrow) ? 1U : 0U;
    }
    return out;
}

// Schoolbook 64x64 -> 128 built from 32-bit halves, so it needs no wider type.
constexpr void mul64(std::uint64_t a, std::uint64_t b, std::uint64_t& high, std::uint64_t& low) noexcept {
    const std::uint64_t a0 = a & 0xFFFF'FFFFULL, a1 = a >> 32;
    const std::uint64_t b0 = b & 0xFFFF'FFFFULL, b1 = b >> 32;
    const std::uint64_t p00 = a0 * b0;
    const std::uint64_t p01 = a0 * b1;
    const std::uint64_t p10 = a1 * b0;
    const std::uint64_t p11 = a1 * b1;
    const std::uint64_t middle = (p00 >> 32) + (p01 & 0xFFFF'FFFFULL) + (p10 & 0xFFFF'FFFFULL);
    low = (middle << 32) | (p00 & 0xFFFF'FFFFULL);
    high = p11 + (p01 >> 32) + (p10 >> 32) + (middle >> 32);
}

// Truncating: the caller sizes its operands so nothing above 512 bits matters.
[[nodiscard]] constexpr limbs multiply(const limbs& a, const limbs& b) noexcept {
    limbs out{};
    for (std::size_t i = 0; i < limb_count; ++i) {
        if (a[i] == 0) continue;
        std::uint64_t carry = 0;
        for (std::size_t j = 0; i + j < limb_count; ++j) {
            std::uint64_t high = 0, low = 0;
            mul64(a[i], b[j], high, low);
            std::uint64_t acc = out[i + j] + low;
            std::uint64_t carry_out = (acc < low) ? 1U : 0U;
            acc += carry;
            if (acc < carry) ++carry_out;
            out[i + j] = acc;
            carry = high + carry_out;
        }
    }
    return out;
}

struct division {
    limbs quotient;
    limbs remainder;
};

// Binary long division. Slow by design: it runs only during constant
// evaluation, and it is short enough to be read and trusted.
[[nodiscard]] constexpr division divmod(const limbs& numerator, const limbs& divisor) noexcept {
    division out{};
    const std::size_t width = bit_width(numerator);
    for (std::size_t i = width; i-- > 0;) {
        out.remainder = shift_left_one(out.remainder);
        if (test_bit(numerator, i)) out.remainder[0] |= 1U;
        if (compare(out.remainder, divisor) >= 0) {
            out.remainder = subtract(out.remainder, divisor);
            out.quotient[i / 64] |= std::uint64_t{1} << (i % 64);
        }
    }
    return out;
}

// A plain result rather than std::expected: instantiating std::expected over a
// 64-byte limb array costs compile time in every consumer, and this one never
// crosses an API boundary.
struct rounded {
    limbs value{};
    ArithmeticError error{};
    bool ok = false;
};

// The same rounding contract as the runtime round_magnitude, on limbs.
[[nodiscard]] constexpr rounded round_magnitude(limbs quotient, const limbs& remainder, const limbs& divisor,
                                                bool negative, Rounding rounding, const limbs& limit) noexcept {
    if (compare(quotient, limit) > 0) return {{}, ArithmeticError::overflow, false};
    if (is_zero(remainder)) return {quotient, {}, true};
    if (rounding == Rounding::exact) return {{}, ArithmeticError::inexact, false};

    bool increment = false;
    switch (rounding) {
    case Rounding::toward_zero: break;
    case Rounding::floor: increment = negative; break;
    case Rounding::ceil: increment = !negative; break;
    case Rounding::nearest_even: {
        // Twice the remainder against the divisor, with ties going to even.
        const limbs twice = shift_left_one(remainder);
        const int cmp = compare(twice, divisor);
        increment = cmp > 0 || (cmp == 0 && (quotient[0] & 1U) != 0);
        break;
    }
    case Rounding::nearest_away: {
        const limbs twice = shift_left_one(remainder);
        increment = compare(twice, divisor) >= 0;
        break;
    }
    case Rounding::exact: break;
    }
    if (!increment) return {quotient, {}, true};
    if (compare(quotient, limit) == 0) return {{}, ArithmeticError::overflow, false};
    return {add(quotient, from_u64(1)), {}, true};
}

// Largest representable magnitude for a width and sign.
[[nodiscard]] constexpr limbs magnitude_limit(std::size_t Bits, bool negative) noexcept {
    limbs out{};
    // 2^(Bits-1) for a negative result, 2^(Bits-1) - 1 for a positive one.
    const std::size_t top = Bits - 1;
    out[top / 64] = std::uint64_t{1} << (top % 64);
    if (!negative) out = subtract(out, from_u64(1));
    return out;
}

[[nodiscard]] constexpr bool fits(const limbs& magnitude, std::size_t Bits, bool negative) noexcept {
    return compare(magnitude, magnitude_limit(Bits, negative)) <= 0;
}

// --- conversion between the public raw types and limbs -------------------

template<typename Raw>
[[nodiscard]] constexpr bool raw_is_negative(Raw value) noexcept {
    if constexpr (requires { value.is_negative(); })
        return value.is_negative();
    else
        return value < Raw{0};
}

// Magnitude as limbs, correct for the signed minimum (whose negation does not
// fit the type) because it never negates in the signed domain.
template<std::size_t Bits, typename Raw>
[[nodiscard]] constexpr limbs to_limbs(Raw value) noexcept {
    limbs out{};
    const bool negative = raw_is_negative(value);
    if constexpr (Bits <= 64) {
        // Widening sign-extends, and negating in the unsigned domain gives the
        // magnitude even for the signed minimum, whose negation does not fit.
        const auto bits = static_cast<std::uint64_t>(static_cast<std::int64_t>(value));
        out[0] = negative ? (std::uint64_t{0} - bits) : bits;
    } else if constexpr (Bits == 128) {
        out[0] = value.low;
        out[1] = value.high;
        if (negative) {
            out = subtract(limbs{}, out);
            for (std::size_t i = 2; i < limb_count; ++i) out[i] = 0;
        }
    } else {
        for (std::size_t i = 0; i < 4; ++i) out[i] = value.limbs[i];
        if (negative) {
            out = subtract(limbs{}, out);
            for (std::size_t i = 4; i < limb_count; ++i) out[i] = 0;
        }
    }
    return out;
}

// Rebuild a raw value from a magnitude and a sign. The caller has checked fits().
template<std::size_t Bits, typename Raw>
[[nodiscard]] constexpr Raw from_limbs(const limbs& magnitude, bool negative) noexcept {
    const limbs value = negative ? subtract(limbs{}, magnitude) : magnitude;
    if constexpr (Bits <= 64) {
        return static_cast<Raw>(static_cast<std::int64_t>(value[0]));
    } else if constexpr (Bits == 128) {
        return Raw(value[0], value[1]);
    } else {
        return Raw(value[0], value[1], value[2], value[3]);
    }
}

[[nodiscard]] constexpr limbs pow10(unsigned exponent) noexcept {
    limbs out = from_u64(1);
    const limbs ten = from_u64(10);
    for (unsigned i = 0; i < exponent; ++i) out = multiply(out, ten);
    return out;
}

// --- the operations, in the same contract as the runtime kernels ----------
//
// These take the width and scale as ordinary arguments rather than template
// parameters. They are still evaluated in constant expressions; the difference
// is that a translation unit using five different Fixed types instantiates one
// copy of each operation instead of five.

struct outcome {
    limbs value{};
    ArithmeticError error{};
    bool ok = false;
};

[[nodiscard]] constexpr outcome mul_limbs(const limbs& a, const limbs& b, bool negative, std::size_t bits,
                                          unsigned decimals, Rounding rounding) noexcept {
    const limbs product = multiply(a, b);
    const limbs scale = pow10(decimals);
    const auto split = divmod(product, scale);
    const rounded r =
        round_magnitude(split.quotient, split.remainder, scale, negative, rounding, magnitude_limit(bits, negative));
    return {r.value, r.error, r.ok};
}

[[nodiscard]] constexpr outcome div_limbs(const limbs& a, const limbs& b, bool negative, std::size_t bits,
                                          unsigned decimals, Rounding rounding) noexcept {
    if (is_zero(b)) return {{}, ArithmeticError::division_by_zero, false};
    const limbs numerator = multiply(a, pow10(decimals));
    const auto split = divmod(numerator, b);
    const rounded r =
        round_magnitude(split.quotient, split.remainder, b, negative, rounding, magnitude_limit(bits, negative));
    return {r.value, r.error, r.ok};
}

[[nodiscard]] constexpr outcome mul_div_limbs(const limbs& a, const limbs& b, const limbs& c, bool negative,
                                              std::size_t bits, Rounding rounding) noexcept {
    if (is_zero(c)) return {{}, ArithmeticError::division_by_zero, false};
    const auto split = divmod(multiply(a, b), c);
    const rounded r =
        round_magnitude(split.quotient, split.remainder, c, negative, rounding, magnitude_limit(bits, negative));
    return {r.value, r.error, r.ok};
}

[[nodiscard]] constexpr outcome quantize_limbs(const limbs& a, bool negative, std::size_t bits, unsigned decimals,
                                               unsigned digits, Rounding rounding) noexcept {
    if (digits > decimals) return {{}, ArithmeticError::invalid_precision, false};
    if (digits == decimals) return {a, {}, true};
    const limbs divisor = pow10(decimals - digits);
    const auto split = divmod(a, divisor);
    const rounded r =
        round_magnitude(split.quotient, split.remainder, divisor, negative, rounding, magnitude_limit(bits, negative));
    if (!r.ok) return {{}, r.error, false};
    // Rescaling back to the storage precision can overflow on its own.
    const limbs rescaled = multiply(r.value, divisor);
    if (!fits(rescaled, bits, negative)) return {{}, ArithmeticError::overflow, false};
    return {rescaled, {}, true};
}

// --- typed adapters -------------------------------------------------------

template<std::size_t Bits, unsigned D, typename Raw>
[[nodiscard]] constexpr std::expected<Raw, ArithmeticError> mul(Raw a, Raw b, Rounding rounding) noexcept {
    const bool negative = raw_is_negative(a) != raw_is_negative(b);
    const outcome r = mul_limbs(to_limbs<Bits>(a), to_limbs<Bits>(b), negative, Bits, D, rounding);
    if (!r.ok) return std::unexpected(r.error);
    return from_limbs<Bits, Raw>(r.value, negative);
}

template<std::size_t Bits, unsigned D, typename Raw>
[[nodiscard]] constexpr std::expected<Raw, ArithmeticError> div(Raw a, Raw b, Rounding rounding) noexcept {
    const bool negative = raw_is_negative(a) != raw_is_negative(b);
    const outcome r = div_limbs(to_limbs<Bits>(a), to_limbs<Bits>(b), negative, Bits, D, rounding);
    if (!r.ok) return std::unexpected(r.error);
    return from_limbs<Bits, Raw>(r.value, negative);
}

template<std::size_t Bits, unsigned D, typename Raw>
[[nodiscard]] constexpr std::expected<Raw, ArithmeticError> mul_div(Raw a, Raw b, Raw c, Rounding rounding) noexcept {
    const bool negative = raw_is_negative(a) != (raw_is_negative(b) != raw_is_negative(c));
    const outcome r = mul_div_limbs(to_limbs<Bits>(a), to_limbs<Bits>(b), to_limbs<Bits>(c), negative, Bits, rounding);
    if (!r.ok) return std::unexpected(r.error);
    return from_limbs<Bits, Raw>(r.value, negative);
}

template<std::size_t Bits, unsigned D, typename Raw>
[[nodiscard]] constexpr std::expected<Raw, ArithmeticError> quantize(Raw a, unsigned digits,
                                                                     Rounding rounding) noexcept {
    const bool negative = raw_is_negative(a);
    const outcome r = quantize_limbs(to_limbs<Bits>(a), negative, Bits, D, digits, rounding);
    if (!r.ok) return std::unexpected(r.error);
    return from_limbs<Bits, Raw>(r.value, negative);
}

template<std::size_t Bits, typename Raw>
[[nodiscard]] constexpr std::expected<Raw, ArithmeticError> remainder(Raw a, Raw b) noexcept {
    const limbs denominator = to_limbs<Bits>(b);
    if (is_zero(denominator)) return std::unexpected(ArithmeticError::division_by_zero);
    // The mathematical remainder takes the numerator's sign (truncation).
    const auto split = divmod(to_limbs<Bits>(a), denominator);
    return from_limbs<Bits, Raw>(split.remainder, raw_is_negative(a));
}

} // namespace fixedwide::detail::ce
