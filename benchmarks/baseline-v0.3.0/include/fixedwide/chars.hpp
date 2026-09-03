#pragma once
#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <cstddef>
#include <expected>
#include <string_view>

namespace fixedwide {
inline namespace FIXEDWIDE_SCALE_NAMESPACE {
// Strict ASCII decimal/scientific grammar; consumes the entire view, no whitespace.
// Excess zero digits are exact; excess NONZERO digits require an explicit rounding mode.
[[nodiscard]] std::expected<FP64, ParseError> parse64(std::string_view, Rounding = Rounding::exact) noexcept;
[[nodiscard]] std::expected<FP128, ParseError> parse128(std::string_view, Rounding = Rounding::exact) noexcept;

struct FormatOptions {
    unsigned digits = fractional_digits;
    bool trim_trailing_zeros = false;
    Rounding rounding = Rounding::toward_zero;
};
inline constexpr std::size_t text_capacity = 48; // enough for either signed type, dot and zero padding
// Writes no NUL. On failure the ENTIRE destination remains unchanged.
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char* buffer, std::size_t capacity, FP64, FormatOptions = {}) noexcept;
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char* buffer, std::size_t capacity, FP128, FormatOptions = {}) noexcept;
} // inline namespace
} // namespace fixedwide
