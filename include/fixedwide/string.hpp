#pragma once
#include <fixedwide/chars.hpp>
#include <string>

namespace fixedwide {

template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::string to_string(basic_fixed<Bits, D> val, FormatOptions options = {}) {
    char buf[text_capacity];
    auto res = to_chars(buf, sizeof(buf), val, options);
    if (!res) return {};
    return std::string(buf, *res);
}

[[nodiscard]] inline std::string to_string(wide::int128 val) {
    char buf[128];
    auto res = to_chars(buf, sizeof(buf), val);
    if (!res) return {};
    return std::string(buf, *res);
}

[[nodiscard]] inline std::string to_string(wide::uint128 val) {
    char buf[128];
    auto res = to_chars(buf, sizeof(buf), val);
    if (!res) return {};
    return std::string(buf, *res);
}

[[nodiscard]] inline std::string to_string(wide::int256 val) {
    char buf[128];
    auto res = to_chars(buf, sizeof(buf), val);
    if (!res) return {};
    return std::string(buf, *res);
}

[[nodiscard]] inline std::string to_string(wide::uint256 val) {
    char buf[128];
    auto res = to_chars(buf, sizeof(buf), val);
    if (!res) return {};
    return std::string(buf, *res);
}

} // namespace fixedwide
