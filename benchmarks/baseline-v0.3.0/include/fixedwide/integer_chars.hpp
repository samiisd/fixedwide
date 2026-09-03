#pragma once
#include <fixedwide/wide_integer.hpp>
#include <fixedwide/error.hpp>
#include <cstddef>
#include <expected>
#include <string_view>

namespace fixedwide {
inline constexpr std::size_t integer_text_capacity = 80;
[[nodiscard]] std::expected<i128, ParseError> parse_i128(std::string_view) noexcept;
[[nodiscard]] std::expected<u128, ParseError> parse_u128(std::string_view) noexcept;
[[nodiscard]] std::expected<i256, ParseError> parse_i256(std::string_view) noexcept;
[[nodiscard]] std::expected<u256, ParseError> parse_u256(std::string_view) noexcept;
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char*, std::size_t, i128) noexcept;
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char*, std::size_t, u128) noexcept;
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char*, std::size_t, i256) noexcept;
[[nodiscard]] std::expected<std::size_t, FormatError> to_chars(char*, std::size_t, u256) noexcept;
} // namespace fixedwide
