#pragma once
#include <fixedwide/chars.hpp>
#include <format>

template<std::size_t Bits, unsigned D>
struct std::formatter<fixedwide::basic_fixed<Bits, D>> : std::formatter<std::string_view> {
    auto format(const fixedwide::basic_fixed<Bits, D>& val, std::format_context& ctx) const {
        char buf[fixedwide::text_capacity];
        auto res = fixedwide::to_chars(buf, sizeof(buf), val);
        if (!res) return std::formatter<std::string_view>::format("", ctx);
        return std::formatter<std::string_view>::format(std::string_view(buf, *res), ctx);
    }
};
