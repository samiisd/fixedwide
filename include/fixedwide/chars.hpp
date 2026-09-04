#pragma once
#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <string_view>
#include <expected>
#include <cstddef>

namespace fixedwide {

/// How `to_chars` should render a value.
struct FormatOptions {
    // `digits` is the number of DECIMALS to print, the same quantity the rest of
    // the library calls `decimals`. The name is 0.4's and is kept because a
    // designated initialiser -- `{.digits = 2}` -- is written by callers, and
    // the paired benchmark writes one in source that must stay byte-identical.
    unsigned digits{0};
    /// Drop trailing zeros in the fraction: 1.2300 prints as "1.23".
    bool trim_trailing_zeros{false};
    /// How to resolve the rounding when `digits` is fewer than the type carries.
    /// `Rounding::exact` returns `FormatError::inexact` rather than round.
    Rounding rounding{Rounding::nearest_even};
    /// Set this to print zero decimals. Without it, `digits == 0` means "all of
    /// them", which is what a default-constructed `FormatOptions` asks for.
    bool explicit_digits{false};
};

/// A buffer of this size is always enough for any `basic_fixed`, sign, point
/// and all, so `to_chars` into `char[text_capacity]` cannot return
/// `FormatError::buffer_too_small`.
inline constexpr std::size_t text_capacity = 128;

namespace detail {

// Explicitly instantiated per width in chars.cpp, so the destination's limits
// are constants inside the kernel rather than a runtime switch.
template<std::size_t Bits>
std::expected<wide::int256, ParseError> parse_fixed_kernel(std::string_view text, unsigned decimals,
                                                           Rounding rounding) noexcept;

// One entry point per storage width: routing a Fixed64<12> through the 256-bit
// kernel widened its raw value to 32 bytes and passed it through memory.
std::expected<std::size_t, FormatError> format_fixed_kernel(char* buffer, std::size_t capacity, std::int64_t raw,
                                                            unsigned decimals, FormatOptions options) noexcept;

std::expected<std::size_t, FormatError> format_fixed_kernel(char* buffer, std::size_t capacity, wide::int128 raw,
                                                            unsigned decimals, FormatOptions options) noexcept;

// One entry point per storage width: routing a Fixed64<12> through the 256-bit
// kernel widened its raw value to 32 bytes and passed it through memory.
std::expected<std::size_t, FormatError> format_fixed_kernel(char* buffer, std::size_t capacity, std::int64_t raw,
                                                            unsigned decimals, FormatOptions options) noexcept;

std::expected<std::size_t, FormatError> format_fixed_kernel(char* buffer, std::size_t capacity, wide::int128 raw,
                                                            unsigned decimals, FormatOptions options) noexcept;

std::expected<std::size_t, FormatError> format_fixed_kernel(char* buffer, std::size_t capacity, wide::int256 raw,
                                                            unsigned decimals, FormatOptions options,
                                                            std::size_t bits) noexcept;

} // namespace detail

template<typename T>
[[nodiscard]] inline std::expected<T, ParseError>
/// Parse decimal text in `[first, last)`, in the shape of `std::from_chars`.
///
/// Accepts an optional sign, digits, an optional point and an optional
/// exponent. The default rounding is `Rounding::exact`, so text that does not
/// land on the type's decimal grid is an error rather than a silent round.
///
/// \tparam T the fixed-point type to produce.
/// \return the value, or `ParseError::empty` / `invalid` / `too_precise` /
///         `overflow`.
/// \see parse for the `std::string_view` spelling.
from_chars(const char* first, const char* last, Rounding rounding = Rounding::exact) noexcept {
    if (first == nullptr || last == nullptr || first >= last) return std::unexpected(ParseError::empty);
    std::string_view text(first, static_cast<std::size_t>(last - first));
    auto res = detail::parse_fixed_kernel<T::bits>(text, T::fractional_digits, rounding);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<T>(*res);
}

template<typename T>
[[nodiscard]] inline std::expected<T, ParseError>
/// Parse decimal text. The inverse of `to_string`, and the same parser as
/// `from_chars` with a friendlier argument.
///
/// \tparam T the fixed-point type to produce.
/// \param rounding `Rounding::exact` by default: text carrying more decimals
///                 than `T` can hold is `ParseError::too_precise`, not a
///                 silent round.
/// \return the value, or a `ParseError`.
parse(std::string_view text, Rounding rounding = Rounding::exact) noexcept {
    auto res = detail::parse_fixed_kernel<T::bits>(text, T::fractional_digits, rounding);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<T>(*res);
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::expected<std::size_t, FormatError>
/// Write `value` as decimal text into `buffer`.
///
/// Writes no terminator, in the shape of `std::to_chars`. A
/// `char[text_capacity]` is always large enough.
///
/// \param capacity bytes available at `buffer`.
/// \param options  digits, trailing zeros and rounding; see `FormatOptions`.
/// \return the number of bytes written, or `FormatError::buffer_too_small` /
///         `invalid_precision` / `inexact`.
to_chars(char* buffer, std::size_t capacity, basic_fixed<Bits, D> value, FormatOptions options = {}) noexcept {
    if (!options.explicit_digits && options.digits == 0) {
        options.digits = D;
    }
    if constexpr (Bits <= 64) {
        return detail::format_fixed_kernel(buffer, capacity, static_cast<std::int64_t>(value.raw()), D, options);
    } else if constexpr (Bits == 128) {
        return detail::format_fixed_kernel(buffer, capacity, value.raw(), D, options);
    } else {
        return detail::format_fixed_kernel(buffer, capacity, detail::to_int256_raw(value.raw()), D, options, Bits);
    }
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::expected<char*, FormatError>
/// Write `value` as decimal text into `[first, last)`.
/// \return a pointer one past the last byte written, or a `FormatError`.
to_chars(char* first, char* last, basic_fixed<Bits, D> value, FormatOptions options = {}) noexcept {
    if (first == nullptr || last == nullptr || first >= last) return std::unexpected(FormatError::buffer_too_small);
    std::size_t cap = static_cast<std::size_t>(last - first);
    auto res = to_chars(first, cap, value, options);
    if (!res) return std::unexpected(res.error());
    return first + *res;
}

// Wide integer text functions
/// Parse a plain decimal integer into a wide integer -- no scale, no rounding.
/// \return the value, or `ParseError::empty` / `invalid` / `overflow`.
[[nodiscard]] std::expected<wide::int128, ParseError> parse_i128(std::string_view text) noexcept;
/// Parse a plain decimal integer into a wide integer -- no scale, no rounding.
/// \return the value, or `ParseError::empty` / `invalid` / `overflow`.
[[nodiscard]] std::expected<wide::uint128, ParseError> parse_u128(std::string_view text) noexcept;
/// Parse a plain decimal integer into a wide integer -- no scale, no rounding.
/// \return the value, or `ParseError::empty` / `invalid` / `overflow`.
[[nodiscard]] std::expected<wide::int256, ParseError> parse_i256(std::string_view text) noexcept;
/// Parse a plain decimal integer into a wide integer -- no scale, no rounding.
/// \return the value, or `ParseError::empty` / `invalid` / `overflow`.
[[nodiscard]] std::expected<wide::uint256, ParseError> parse_u256(std::string_view text) noexcept;

/// Write a wide integer as decimal text -- no scale, no point.
/// \return the number of bytes written, or `FormatError::buffer_too_small`.
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char* buffer, std::size_t capacity,
                                                               wide::int128 value) noexcept;
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char* buffer, std::size_t capacity,
                                                               wide::uint128 value) noexcept;
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char* buffer, std::size_t capacity,
                                                               wide::int256 value) noexcept;
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char* buffer, std::size_t capacity,
                                                               wide::uint256 value) noexcept;

// 0.4 compatibility surface: fixed at 12 digits. Generic replacement:
// parse<T>(text, rounding). See fixed.hpp.
[[nodiscard]] inline std::expected<Fixed64<12>, ParseError> parse64(std::string_view text,
                                                                    Rounding rounding = Rounding::exact) noexcept {
    return parse<Fixed64<12>>(text, rounding);
}
[[nodiscard]] inline std::expected<Fixed128<12>, ParseError> parse128(std::string_view text,
                                                                      Rounding rounding = Rounding::exact) noexcept {
    return parse<Fixed128<12>>(text, rounding);
}
} // namespace fixedwide
