#pragma once
#include <fixedwide/chars.hpp>
#include <string>
#include <expected>

namespace fixedwide {

template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::expected<std::string, FormatError> to_string(basic_fixed<Bits, D> val, FormatOptions options = {}) {
    char buf[text_capacity];
    auto res = to_chars(buf, sizeof(buf), val, options);
    if (!res) return std::unexpected(res.error());
    return std::string(buf, *res);
}

[[nodiscard]] inline std::expected<std::string, FormatError> to_string(wide::int128 val) {
    char buf[128];
    auto res = to_chars(buf, sizeof(buf), val);
    if (!res) return std::unexpected(res.error());
    return std::string(buf, *res);
}

[[nodiscard]] inline std::expected<std::string, FormatError> to_string(wide::uint128 val) {
    char buf[128];
    auto res = to_chars(buf, sizeof(buf), val);
    if (!res) return std::unexpected(res.error());
    return std::string(buf, *res);
}

[[nodiscard]] inline std::expected<std::string, FormatError> to_string(wide::int256 val) {
    char buf[128];
    auto res = to_chars(buf, sizeof(buf), val);
    if (!res) return std::unexpected(res.error());
    return std::string(buf, *res);
}

[[nodiscard]] inline std::expected<std::string, FormatError> to_string(wide::uint256 val) {
    char buf[128];
    auto res = to_chars(buf, sizeof(buf), val);
    if (!res) return std::unexpected(res.error());
    return std::string(buf, *res);
}

} // namespace fixedwide
