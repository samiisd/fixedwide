#pragma once

#include <fixedwide/mixed.hpp>
#include <cstdint>
#include <expected>
#include <type_traits>

// Arithmetic used by the batch interface. The ordinary scalar entry points
// deliberately keep their existing dispatch and code generation.
namespace fixedwide::detail::batch {

#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)

using u128 = unsigned __int128;
using i128 = __int128;

template<typename Raw>
[[nodiscard]] constexpr u128 native_magnitude(Raw raw) noexcept {
    const auto value = static_cast<i128>(raw);
    const auto bits = static_cast<u128>(value);
    return value < 0 ? u128{0} - bits : bits;
}

// The divisor is invariant for a broadcast. For a one-limb numerator,
// floor(n * floor(2^64/d) / 2^64) is at most one below floor(n/d).
// One multiply-high and one correction replace a hardware division. d==1 is
// separate because its reciprocal would need 65 bits.
class prepared_divisor {
    std::uint64_t divisor_ = 1;
    std::uint64_t reciprocal_ = 0;

    [[nodiscard]] constexpr mixed_native::Divided divide_limb(std::uint64_t value) const noexcept {
        const auto quotient = static_cast<std::uint64_t>((static_cast<u128>(value) * reciprocal_) >> 64);
        const auto remainder = value - quotient * divisor_;
        const bool correction = remainder >= divisor_;
        return {static_cast<u128>(quotient) + correction, static_cast<u128>(remainder - (correction ? divisor_ : 0))};
    }

public:
    constexpr prepared_divisor() noexcept = default;

    explicit constexpr prepared_divisor(std::uint64_t divisor) noexcept : divisor_(divisor) {
        if (divisor != 1) reciprocal_ = static_cast<std::uint64_t>((u128{1} << 64) / divisor);
    }

    [[nodiscard]] constexpr std::uint64_t value() const noexcept { return divisor_; }

#if defined(__clang__)
    [[gnu::always_inline]]
#endif
    [[nodiscard]] constexpr mixed_native::Divided operator()(u128 value) const noexcept {
        if (divisor_ == 1) return {value, 0};
        const auto high = static_cast<std::uint64_t>(value >> 64);
        if (high == 0) return divide_limb(static_cast<std::uint64_t>(value));
#if defined(FIXEDWIDE_HAS_X86_64_ASM)
        if !consteval {
            // A rounded one-limb product usually has high < divisor_. It
            // already fits divq's remainder input, so avoid a reciprocal
            // multiply and correction unless the high limb needs reducing.
            std::uint64_t quotient_high = 0;
            std::uint64_t remainder_high = high;
            if (high >= divisor_) {
                const auto upper = divide_limb(high);
                quotient_high = static_cast<std::uint64_t>(upper.quotient);
                remainder_high = static_cast<std::uint64_t>(upper.remainder);
            }
            std::uint64_t quotient_low, remainder;
            // remainder_high < divisor_, so the quotient fits one limb.
            __asm__("divq %[divisor]"
                    : "=a"(quotient_low), "=d"(remainder)
                    : "a"(static_cast<std::uint64_t>(value)), "d"(remainder_high), [divisor] "r"(divisor_)
                    : "cc");
            return {(static_cast<u128>(quotient_high) << 64) | quotient_low, remainder};
        }
#endif
        return mixed_native::divide_magnitude(value, divisor_);
    }
};

template<unsigned Scale>
struct constant_divisor {
    static_assert(Scale <= 19);
    static constexpr auto denominator = static_cast<std::uint64_t>(mixed_native::pow10_v<Scale>);

    [[nodiscard]] static constexpr std::uint64_t value() noexcept { return denominator; }

    [[nodiscard]] constexpr mixed_native::Divided operator()(u128 value) const noexcept {
        if constexpr (Scale == 0) return {value, 0};
        // Keep one-limb division visibly constant so the compiler uses its
        // ordinary multiply/shift sequence instead of a 128-bit divide call.
        if ((value >> 64) == 0) {
            const auto low = static_cast<std::uint64_t>(value);
            return {low / denominator, low % denominator};
        }
        return mixed_native::divide_magnitude(value, denominator);
    }
};

[[nodiscard]] constexpr std::uint64_t gcd(std::uint64_t a, std::uint64_t b) noexcept {
    while (b != 0) {
        const auto next = a % b;
        a = b;
        b = next;
    }
    return a;
}

// q is a magnitude of a signed <=128-bit source divided by a positive
// denominator. Its high limb is at most 2^63. Each cross product and their
// addition therefore fits unsigned128; a nonzero third limb is a real output
// overflow, not just an oversized pre-rescale intermediate.
[[nodiscard]] constexpr bool multiply_limb(u128 q, std::uint64_t factor, u128& product) noexcept {
    const u128 low = static_cast<u128>(static_cast<std::uint64_t>(q)) * factor;
    const u128 upper = static_cast<u128>(static_cast<std::uint64_t>(q >> 64)) * factor + (low >> 64);
    if ((upper >> 64) != 0) return false;
    product = (upper << 64) | static_cast<std::uint64_t>(low);
    return true;
}

template<typename Dest>
#if defined(__clang__)
[[gnu::always_inline]]
#endif
[[nodiscard]] constexpr std::expected<Dest, ArithmeticError>
finish(u128 quotient, u128 remainder, std::uint64_t denominator, bool negative, Rounding rounding) noexcept {
    const auto limit = mixed_native::limit_magnitude_u128<Dest::bits>(negative);
    if (quotient > limit) return std::unexpected(ArithmeticError::overflow);
    if (remainder != 0) {
        if (rounding == Rounding::exact) return std::unexpected(ArithmeticError::inexact);
        bool increment = false;
        switch (rounding) {
        case Rounding::toward_zero: break;
        case Rounding::floor: increment = negative; break;
        case Rounding::ceil: increment = !negative; break;
        case Rounding::nearest_even:
            increment = remainder * 2 > denominator || (remainder * 2 == denominator && (quotient & 1) != 0);
            break;
        case Rounding::nearest_away: increment = remainder * 2 >= denominator; break;
        case Rounding::exact: break;
        }
        if (increment && quotient == limit) return std::unexpected(ArithmeticError::overflow);
        quotient += static_cast<u128>(increment);
    }
    const auto raw = static_cast<i128>(negative ? u128{0} - quotient : quotient);
    return Dest::from_raw(static_cast<typename Dest::raw_type>(raw));
}

// a = q*d+r, hence floor(a*b/d) = q*b+floor(r*b/d). All terms
// are nonnegative. An overflowing quotient product cannot be cancelled, and
// the remainder product fits unsigned128 because r and b each fit uint64.
// This covers the full signed128 x signed64 domain without constructing a
// 192-bit product. Exact scale-certified inputs have r==0, so there is only
// one rescale before the multiply and no rounding work afterwards.
template<typename Dest, typename Divider>
#if defined(__clang__)
// Keep the prepared divisor and constant rounding mode in the row kernel.
// Outlining this boundary materializes expected and reloads the plan per row.
[[gnu::always_inline]]
#endif
[[nodiscard]] constexpr std::expected<Dest, ArithmeticError>
multiply_split(u128 a, std::uint64_t b, bool negative, Rounding rounding, const Divider& divide) noexcept {
    // Two one-limb magnitudes have an exact unsigned128 product, even when
    // that product exceeds signed128. Rounded products need only one divide;
    // splitting first would add a quotient multiply and a second rescale.
    // Exact scale-certified slopes keep the division-before-multiply path.
    if (rounding != Rounding::exact && (a >> 64) == 0) {
        const auto product = static_cast<u128>(static_cast<std::uint64_t>(a)) * b;
        const auto divided = divide(product);
        return finish<Dest>(divided.quotient, divided.remainder, divide.value(), negative, rounding);
    }
    const auto divided = divide(a);
    const auto limit = mixed_native::limit_magnitude_u128<Dest::bits>(negative);
    u128 quotient;
    if (!multiply_limb(divided.quotient, b, quotient) || quotient > limit)
        return std::unexpected(ArithmeticError::overflow);
    if (divided.remainder == 0) return finish<Dest>(quotient, 0, divide.value(), negative, rounding);
    const auto tail = divide(divided.remainder * b);
    if (tail.quotient > limit - quotient) return std::unexpected(ArithmeticError::overflow);
    return finish<Dest>(quotient + tail.quotient, tail.remainder, divide.value(), negative, rounding);
}

#endif

template<typename Dest, typename A, typename B>
#if defined(__clang__) && defined(FIXEDWIDE_HAS_MIXED_NATIVE)
[[gnu::always_inline]]
#endif
[[nodiscard]] inline std::expected<Dest, ArithmeticError> multiply_to(A a, B b, Rounding rounding) noexcept {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    constexpr unsigned product_scale = A::fractional_digits + B::fractional_digits;
    if constexpr (A::bits <= 128 && B::bits <= 128 && Dest::bits <= 128 && product_scale >= Dest::fractional_digits &&
                  product_scale - Dest::fractional_digits <= 19) {
        constexpr unsigned gap = product_scale - Dest::fractional_digits;
        const auto lhs = native_magnitude(a.raw());
        const auto rhs = native_magnitude(b.raw());
        const bool negative = (static_cast<i128>(a.raw()) < 0) != (static_cast<i128>(b.raw()) < 0);
        if ((rhs >> 64) == 0)
            return multiply_split<Dest>(lhs, static_cast<std::uint64_t>(rhs), negative, rounding,
                                        constant_divisor<gap>{});
        if ((lhs >> 64) == 0)
            return multiply_split<Dest>(rhs, static_cast<std::uint64_t>(lhs), negative, rounding,
                                        constant_divisor<gap>{});
    }
#endif
    return fixedwide::mul_to<Dest>(a, b, rounding);
}

// An owned callable keeps the rounding mode visible through a batch map.
// Clang can otherwise outline a factory lambda's operator after the checked
// multiply grows, even when multiply_to itself is forced inline.
template<typename Dest>
class binary_multiplier {
    Rounding rounding_;

public:
    explicit constexpr binary_multiplier(Rounding rounding = Rounding::nearest_even) noexcept : rounding_(rounding) {}

    template<typename A, typename B>
#if defined(__clang__)
    [[gnu::always_inline]]
#endif
    [[nodiscard]] inline std::expected<Dest, ArithmeticError> operator()(A a, B b) const noexcept {
        return multiply_to<Dest>(a, b, rounding_);
    }
};

template<typename Dest, typename B>
class broadcast_multiplier {
    B factor_;
    Rounding rounding_;
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    prepared_divisor divisor_;
    std::conditional_t<(B::bits <= 64), std::uint64_t, u128> reduced_factor_ = 0;
    bool negative_ = false;
#endif

public:
    explicit constexpr broadcast_multiplier(B factor, Rounding rounding = Rounding::nearest_even) noexcept
        : factor_(factor), rounding_(rounding) {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
        if constexpr (B::bits <= 128 && B::fractional_digits <= 19 && Dest::bits <= 128) {
            constexpr auto denominator = static_cast<std::uint64_t>(mixed_native::pow10_v<B::fractional_digits>);
            const auto mag = native_magnitude(factor.raw());
            const auto remainder = mixed_native::divide_magnitude(mag, denominator).remainder;
            const auto common = gcd(static_cast<std::uint64_t>(remainder), denominator);
            reduced_factor_ =
                static_cast<decltype(reduced_factor_)>(mixed_native::divide_magnitude(mag, common).quotient);
            divisor_ = prepared_divisor{denominator / common};
            negative_ = static_cast<i128>(factor.raw()) < 0;
        }
#endif
    }

    template<typename A>
#if defined(__clang__) && defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    [[gnu::always_inline]]
#endif
    [[nodiscard]] inline std::expected<Dest, ArithmeticError> operator()(A a) const noexcept {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
        if constexpr (A::bits <= 128 && B::bits <= 128 && Dest::bits <= 128 &&
                      A::fractional_digits == Dest::fractional_digits && B::fractional_digits <= 19) {
            if constexpr (B::bits <= 64) {
                const bool negative = (static_cast<i128>(a.raw()) < 0) != negative_;
                return multiply_split<Dest>(native_magnitude(a.raw()), reduced_factor_, negative, rounding_, divisor_);
            } else if ((reduced_factor_ >> 64) == 0) {
                const bool negative = (static_cast<i128>(a.raw()) < 0) != negative_;
                return multiply_split<Dest>(native_magnitude(a.raw()), static_cast<std::uint64_t>(reduced_factor_),
                                            negative, rounding_, divisor_);
            }
        }
#endif
        return multiply_to<Dest>(a, factor_, rounding_);
    }
};

} // namespace fixedwide::detail::batch
