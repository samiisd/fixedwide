#pragma once
#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/rounding.hpp>
#include <string_view>
#include <expected>
#include <cstddef>

namespace fixedwide {

struct FormatOptions {
    unsigned digits{0};
    bool trim_trailing_zeros{false};
    Rounding rounding{Rounding::nearest_even};
    bool explicit_digits{false};
};

inline constexpr std::size_t text_capacity = 128;

namespace detail {

// Explicitly instantiated per width in chars.cpp, so the destination's limits
// are constants inside the kernel rather than a runtime switch.
template<std::size_t Bits>
std::expected<wide::int256, ParseError>
parse_fixed_kernel(std::string_view text, unsigned decimals, Rounding rounding) noexcept;

// One entry point per storage width: routing a Fixed64<12> through the 256-bit
// kernel widened its raw value to 32 bytes and passed it through memory.
std::expected<std::size_t, FormatError>
format_fixed_kernel(char* buffer, std::size_t capacity, std::int64_t raw, unsigned decimals,
                    FormatOptions options) noexcept;

std::expected<std::size_t, FormatError>
format_fixed_kernel(char* buffer, std::size_t capacity, wide::int128 raw, unsigned decimals,
                    FormatOptions options) noexcept;

// One entry point per storage width: routing a Fixed64<12> through the 256-bit
// kernel widened its raw value to 32 bytes and passed it through memory.
std::expected<std::size_t, FormatError>
format_fixed_kernel(char* buffer, std::size_t capacity, std::int64_t raw, unsigned decimals,
                    FormatOptions options) noexcept;

std::expected<std::size_t, FormatError>
format_fixed_kernel(char* buffer, std::size_t capacity, wide::int128 raw, unsigned decimals,
                    FormatOptions options) noexcept;

std::expected<std::size_t, FormatError>
format_fixed_kernel(char* buffer, std::size_t capacity,
                    wide::int256 raw, unsigned decimals,
                    FormatOptions options, std::size_t bits) noexcept;

} // namespace detail

template<typename T>
[[nodiscard]] inline std::expected<T, ParseError>
from_chars(const char* first, const char* last, Rounding rounding = Rounding::exact) noexcept {
    if (first == nullptr || last == nullptr || first >= last) return std::unexpected(ParseError::empty);
    std::string_view text(first, static_cast<std::size_t>(last - first));
    auto res = detail::parse_fixed_kernel<T::bits>(text, T::fractional_digits, rounding);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<T>(*res);
}

template<typename T>
[[nodiscard]] inline std::expected<T, ParseError>
parse(std::string_view text, Rounding rounding = Rounding::exact) noexcept {
    auto res = detail::parse_fixed_kernel<T::bits>(text, T::fractional_digits, rounding);
    if (!res) return std::unexpected(res.error());
    return detail::from_int256_raw<T>(*res);
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::expected<std::size_t, FormatError>
to_chars(char* buffer, std::size_t capacity, basic_fixed<Bits, D> value, FormatOptions options = {}) noexcept {
    if (!options.explicit_digits && options.digits == 0) {
        options.digits = D;
    }
    if constexpr (Bits <= 64) {
        return detail::format_fixed_kernel(buffer, capacity, static_cast<std::int64_t>(value.raw()),
                                           D, options);
    } else if constexpr (Bits == 128) {
        return detail::format_fixed_kernel(buffer, capacity, value.raw(), D, options);
    } else {
        return detail::format_fixed_kernel(buffer, capacity, detail::to_int256_raw(value.raw()),
                                           D, options, Bits);
    }
}

template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::expected<char*, FormatError>
to_chars(char* first, char* last, basic_fixed<Bits, D> value, FormatOptions options = {}) noexcept {
    if (first == nullptr || last == nullptr || first >= last) return std::unexpected(FormatError::buffer_too_small);
    std::size_t cap = static_cast<std::size_t>(last - first);
    auto res = to_chars(first, cap, value, options);
    if (!res) return std::unexpected(res.error());
    return first + *res;
}

// Wide integer text functions
[[nodiscard]] std::expected<wide::int128, ParseError> parse_i128(std::string_view text) noexcept;
[[nodiscard]] std::expected<wide::uint128, ParseError> parse_u128(std::string_view text) noexcept;
[[nodiscard]] std::expected<wide::int256, ParseError> parse_i256(std::string_view text) noexcept;
[[nodiscard]] std::expected<wide::uint256, ParseError> parse_u256(std::string_view text) noexcept;

[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char* output, std::size_t capacity, wide::int128 value) noexcept;
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char* output, std::size_t capacity, wide::uint128 value) noexcept;
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char* output, std::size_t capacity, wide::int256 value) noexcept;
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char* output, std::size_t capacity, wide::uint256 value) noexcept;


[[nodiscard]] inline std::expected<Fixed64<12>, ParseError> parse64(std::string_view s, Rounding r = Rounding::exact) noexcept {
    return parse<Fixed64<12>>(s, r);
}
[[nodiscard]] inline std::expected<Fixed128<12>, ParseError> parse128(std::string_view s, Rounding r = Rounding::exact) noexcept {
    return parse<Fixed128<12>>(s, r);
}
} // namespace fixedwide
