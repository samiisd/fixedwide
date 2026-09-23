#pragma once

/// \file
/// Exact, compile-time decimal constants: literal<Fixed64<2>>("19.99").

#include <fixedwide/fixed.hpp>
#include <fixedwide/detail/decimal_scan.hpp>
#include <cstdlib>

namespace fixedwide::detail {

// Only exact assembly belongs here. Runtime rounding and its width-specific
// fast paths stay in chars.cpp; both paths use the same scanner and limits.
template<class T>
[[nodiscard]] constexpr std::expected<T, ParseError> parse_literal_exact(std::string_view text) noexcept {
    const auto scanned = scan_decimal(text);
    if (!scanned) return std::unexpected(scanned.error());
    const auto& [negative, significant, fractional, exponent, mantissa_end] = *scanned;
    if (significant == 0) return T{};

    const auto keep = significant + static_cast<std::int64_t>(T::fractional_digits) + exponent - fractional;
    if (keep > max_digits_for_bits(T::bits)) return std::unexpected(ParseError::overflow);

    constexpr auto positive_limit = limit_magnitude_u256<T::bits>(false);
    constexpr auto negative_limit = limit_magnitude_u256<T::bits>(true);
    constexpr auto positive_cutoff = positive_limit / wide::uint256{10};
    constexpr auto negative_cutoff = negative_limit / wide::uint256{10};
    const auto limit = negative ? negative_limit : positive_limit;
    const auto cutoff = negative ? negative_cutoff : positive_cutoff;
    const auto last_digit = static_cast<unsigned>((limit - ((cutoff << 3) + (cutoff << 1))).limbs[0]);
    wide::uint256 value{};
    auto append = [&](unsigned digit) constexpr {
        if (value > cutoff || (value == cutoff && digit > last_digit)) return false;
        value = (value << 3) + (value << 1) + wide::uint256{digit};
        return true;
    };

    std::int64_t index = 0;
    bool started = false, discarded_nonzero = false;
    for (std::size_t pos = 0; pos < mantissa_end; ++pos) {
        const char c = text[pos];
        if (c == '.') continue;
        const auto digit = static_cast<unsigned>(c - '0');
        if (!started && digit == 0) continue;
        started = true;
        if (index < keep) {
            if (!append(digit)) return std::unexpected(ParseError::overflow);
        } else {
            discarded_nonzero |= digit != 0;
        }
        ++index;
    }
    for (; index < keep; ++index) {
        if (!append(0)) return std::unexpected(ParseError::overflow);
    }
    if (discarded_nonzero) return std::unexpected(ParseError::too_precise);

    // Apply the sign in unsigned storage, including the signed minimum.
    if (negative) value = ~value + wide::uint256{1};
    return from_int256_raw<T>(wide::int256{value.limbs[0], value.limbs[1], value.limbs[2], value.limbs[3]});
}

// A failing immediate invocation names the reason in the compiler diagnostic.
// These non-constexpr functions can never be reached by a valid literal call;
// using abort rather than throw also supports -fno-exceptions consumers.
[[noreturn]] inline void literal_empty() noexcept { std::abort(); }
[[noreturn]] inline void literal_invalid() noexcept { std::abort(); }
[[noreturn]] inline void literal_too_precise() noexcept { std::abort(); }
[[noreturn]] inline void literal_overflow() noexcept { std::abort(); }

} // namespace fixedwide::detail

namespace fixedwide {

/// Construct an exact decimal constant, checked during compilation.
///
/// The destination width and scale come from T, never from the spelling of the
/// text. Returns T directly; invalid, out-of-range or inexact constants fail to
/// compile. Accepts the same decimal grammar as parse<T>, including signs and
/// exponents, but deliberately has no rounding option. Use parse<T> for runtime
/// text and checked arithmetic for calculations that require rounding.
///
/// The array parameter preserves embedded NULs instead of silently truncating
/// at the first one. The input must be a string literal or a null-terminated
/// constexpr char array. No floating-point conversion or runtime parser call.
template<class T, std::size_t N>
    requires std::is_same_v<T, basic_fixed<T::bits, T::fractional_digits>>
[[nodiscard]] consteval T literal(const char (&text)[N]) noexcept {
    if (text[N - 1] != '\0') detail::literal_invalid();
    const auto result = detail::parse_literal_exact<T>(std::string_view{text, N - 1});
    if (!result) {
        switch (result.error()) {
        case ParseError::empty: detail::literal_empty();
        case ParseError::invalid: detail::literal_invalid();
        case ParseError::too_precise: detail::literal_too_precise();
        case ParseError::overflow: detail::literal_overflow();
        }
    }
    return *result;
}

} // namespace fixedwide
