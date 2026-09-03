#include <fixedwide/integer_chars.hpp>
#include "detail.hpp"
#include "division.hpp"
#include "text_detail.hpp"
#include <charconv>
#include <cstring>

namespace fixedwide::detail {
namespace {
char* append_chunk_reverse(char* end, std::uint64_t chunk, bool pad) noexcept {
    char digits[20];
    const auto result = std::to_chars(digits, digits + sizeof(digits), chunk);
    const auto count = static_cast<std::size_t>(result.ptr - digits);
    const std::size_t width = pad ? 19 : count;
    end -= width;
    std::memset(end, '0', width - count);
    std::memcpy(end + width - count, digits, count);
    return end;
}
}
char* integer_digits(char* end, u128 value) noexcept {
    constexpr std::uint64_t base = 10'000'000'000'000'000'000ULL;
    do {
        const auto result = divide128by64(value, base);
        end = append_chunk_reverse(end, static_cast<std::uint64_t>(result.remainder), result.quotient != 0);
        value = result.quotient;
    } while (value != 0);
    return end;
}
char* integer_digits(char* end, u256 value) noexcept {
    constexpr std::uint64_t base = 10'000'000'000'000'000'000ULL;
    do {
        const auto result = divide_unsigned(value, base);
        end = append_chunk_reverse(end, static_cast<std::uint64_t>(result.remainder), result.quotient != 0);
        value = result.quotient;
    } while (value != 0);
    return end;
}
} // namespace fixedwide::detail

namespace fixedwide {
namespace {
template<class U>
std::expected<U, ParseError> unsigned_digits(std::string_view text, U limit) noexcept {
    constexpr std::uint64_t base = 1'000'000'000'000'000'000ULL;
    const U cutoff = limit / base;
    U value = 0;
    std::size_t count = text.size() % 18;
    if (count == 0) count = 18;
    while (!text.empty()) {
        std::uint64_t chunk{};
        const char* const end = text.data() + count;
        const auto parsed = std::from_chars(text.data(), end, chunk);
        if (parsed.ec != std::errc{} || parsed.ptr != end) return std::unexpected(ParseError::invalid);
        if (value > cutoff) return std::unexpected(ParseError::overflow);
        value *= base;
        if (value > limit - chunk) return std::unexpected(ParseError::overflow);
        value += chunk;
        text.remove_prefix(count);
        count = 18;
    }
    return value;
}
template<class U>
std::expected<U, ParseError> parse_unsigned(std::string_view text, U limit) noexcept {
    if (text.empty()) return std::unexpected(ParseError::empty);
    if (text.front() == '+') text.remove_prefix(1);
    if (text.empty() || text.front() == '-') return std::unexpected(ParseError::invalid);
    return unsigned_digits(text, limit);
}
template<class S, class U>
std::expected<S, ParseError> parse_signed(std::string_view text, U positive_limit) noexcept {
    if (text.empty()) return std::unexpected(ParseError::empty);
    const bool negative = text.front() == '-';
    if (negative || text.front() == '+') text.remove_prefix(1);
    if (text.empty()) return std::unexpected(ParseError::invalid);
    const auto result = unsigned_digits(text, positive_limit + static_cast<unsigned>(negative));
    if (!result) return std::unexpected(result.error());
    return static_cast<S>(negative ? U{0} - *result : *result);
}
template<class U>
std::expected<std::size_t, FormatError> format_integer(char* output, std::size_t capacity, U value, bool negative) noexcept {
    char buffer[integer_text_capacity];
    char* const end = buffer + sizeof(buffer);
    char* begin = detail::integer_digits(end, value);
    if (negative) *--begin = '-';
    const auto size = static_cast<std::size_t>(end - begin);
    if (capacity < size) return std::unexpected(FormatError::buffer_too_small);
    std::memcpy(output, begin, size);
    return size;
}
} // namespace
std::expected<i128, ParseError> parse_i128(std::string_view text) noexcept { return parse_signed<i128>(text, static_cast<u128>(i128_max)); }
std::expected<u128, ParseError> parse_u128(std::string_view text) noexcept { return parse_unsigned(text, u128_max); }
std::expected<i256, ParseError> parse_i256(std::string_view text) noexcept { return parse_signed<i256>(text, static_cast<u256>(i256_max)); }
std::expected<u256, ParseError> parse_u256(std::string_view text) noexcept { return parse_unsigned(text, u256_max); }
std::expected<std::size_t, FormatError> to_chars(char* output, std::size_t capacity, i128 value) noexcept { return format_integer(output, capacity, magnitude(value), value < 0); }
std::expected<std::size_t, FormatError> to_chars(char* output, std::size_t capacity, u128 value) noexcept { return format_integer(output, capacity, value, false); }
std::expected<std::size_t, FormatError> to_chars(char* output, std::size_t capacity, i256 value) noexcept { return format_integer(output, capacity, magnitude(value), value < 0); }
std::expected<std::size_t, FormatError> to_chars(char* output, std::size_t capacity, u256 value) noexcept { return format_integer(output, capacity, value, false); }
} // namespace fixedwide
