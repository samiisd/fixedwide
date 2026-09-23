#pragma once
#include <fixedwide/error.hpp>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>

namespace fixedwide::detail {

struct DecimalScan {
    bool negative{false};
    std::int64_t significant{0};
    std::int64_t fractional{0};
    std::int64_t exponent{0};
    std::size_t mantissa_end{0};
};

// One grammar for runtime parsing and source constants. The view is advanced
// past the sign; mantissa_end is relative to that unsigned view.
[[nodiscard]] constexpr std::expected<DecimalScan, ParseError> scan_decimal(std::string_view& text) noexcept {
    if (text.empty()) return std::unexpected(ParseError::empty);
    if (text.size() > 4096) return std::unexpected(ParseError::invalid);

    DecimalScan out;
    out.negative = text.front() == '-';
    if (out.negative || text.front() == '+') text.remove_prefix(1);
    if (text.empty()) return std::unexpected(ParseError::invalid);

    bool dot = false;
    std::int64_t digits = 0;
    for (; out.mantissa_end < text.size(); ++out.mantissa_end) {
        const char c = text[out.mantissa_end];
        if (c >= '0' && c <= '9') {
            ++digits;
            out.fractional += static_cast<int>(dot);
            if (out.significant != 0 || c != '0') ++out.significant;
        } else if (c == '.' && !dot) {
            dot = true;
        } else if (c == 'e' || c == 'E') {
            break;
        } else {
            return std::unexpected(ParseError::invalid);
        }
    }
    if (digits == 0) return std::unexpected(ParseError::invalid);

    if (out.mantissa_end != text.size()) {
        std::size_t pos = out.mantissa_end + 1;
        bool exponent_negative = false;
        if (pos < text.size() && (text[pos] == '+' || text[pos] == '-')) {
            exponent_negative = text[pos] == '-';
            ++pos;
        }
        if (pos == text.size()) return std::unexpected(ParseError::invalid);
        const auto cap = static_cast<std::int64_t>(text.size()) + 256;
        for (; pos < text.size(); ++pos) {
            const char c = text[pos];
            if (c < '0' || c > '9') return std::unexpected(ParseError::invalid);
            const int digit = c - '0';
            out.exponent = out.exponent > (cap - digit) / 10 ? cap : out.exponent * 10 + digit;
        }
        if (exponent_negative) out.exponent = -out.exponent;
    }
    return out;
}

[[nodiscard]] constexpr std::int64_t max_digits_for_bits(std::size_t bits) noexcept {
    if (bits == 8) return 3;
    if (bits == 16) return 5;
    if (bits == 32) return 10;
    if (bits == 64) return 19;
    if (bits == 128) return 39;
    return 78; // 256
}

} // namespace fixedwide::detail
