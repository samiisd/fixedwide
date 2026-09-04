#pragma once

/// \file
/// `std::string` conveniences over `<fixedwide/chars.hpp>`. These allocate; the
/// `to_chars` they wrap does not.

#include <fixedwide/chars.hpp>
#include <string>
#include <expected>

namespace fixedwide {

// The inverse of to_string is parse (<fixedwide/chars.hpp>), which takes a
// std::string_view and so accepts a std::string directly. There is no
// `from_string`: it would be a third name for one operation.

/// Render a value as a `std::string`. The one function here that allocates;
/// `to_chars` writes into a caller's buffer and does not.
/// \param options digits, trailing zeros and rounding; see `FormatOptions`.
/// \return the text, or a `FormatError`.
template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::expected<std::string, FormatError> to_string(basic_fixed<Bits, D> val, FormatOptions options = {}) {
    char buf[text_capacity];
    auto res = to_chars(buf, sizeof(buf), val, options);
    if (!res) return std::unexpected(res.error());
    return std::string(buf, *res);
}

/// Render a wide integer as a `std::string` -- no scale, no point.
[[nodiscard]] inline std::expected<std::string, FormatError> to_string(wide::int128 val) {
    char buf[text_capacity];
    auto res = to_chars(buf, sizeof(buf), val);
    if (!res) return std::unexpected(res.error());
    return std::string(buf, *res);
}

[[nodiscard]] inline std::expected<std::string, FormatError> to_string(wide::uint128 val) {
    char buf[text_capacity];
    auto res = to_chars(buf, sizeof(buf), val);
    if (!res) return std::unexpected(res.error());
    return std::string(buf, *res);
}

[[nodiscard]] inline std::expected<std::string, FormatError> to_string(wide::int256 val) {
    char buf[text_capacity];
    auto res = to_chars(buf, sizeof(buf), val);
    if (!res) return std::unexpected(res.error());
    return std::string(buf, *res);
}

[[nodiscard]] inline std::expected<std::string, FormatError> to_string(wide::uint256 val) {
    char buf[text_capacity];
    auto res = to_chars(buf, sizeof(buf), val);
    if (!res) return std::unexpected(res.error());
    return std::string(buf, *res);
}

} // namespace fixedwide
