#include <fixedwide/chars.hpp>
#include "detail.hpp"
#include "division.hpp"
#include "limbs.hpp"
#include "text_detail.hpp"
#include <charconv>
#include <cstring>

namespace fixedwide::detail {

namespace {
char* append_chunk_reverse(char* end, std::uint64_t chunk, bool pad) noexcept {
    char digits[24];
    const auto result = std::to_chars(digits, digits + sizeof(digits), chunk);
    const auto count = static_cast<std::size_t>(result.ptr - digits);
    const std::size_t width = pad ? 19 : count;
    end -= width;
    std::memset(end, '0', width - count);
    std::memcpy(end + width - count, digits, count);
    return end;
}
} // namespace

char* integer_digits(char* end, wide::uint128 value) noexcept {
    if (value.high == 0) {
        return append_chunk_reverse(end, value.low, false);
    }
    constexpr std::uint64_t base = 10'000'000'000'000'000'000ULL; // 10^19
    do {
        const auto result = divide128by64(value, base);
        end = append_chunk_reverse(end, result.remainder.low, !result.quotient.is_zero());
        value = result.quotient;
    } while (!value.is_zero());
    return end;
}

char* integer_digits(char* end, wide::uint256 value) noexcept {
    constexpr std::uint64_t base = 1'000'000'000'000'000'000ULL; // 10^18
    u256_limbs v(value);
    do {
        const auto result = divmod64(v, base);
        end = append_chunk_reverse(end, result.remainder, !result.quotient.is_zero());
        v = result.quotient;
    } while (!v.is_zero());
    return end;
}

} // namespace fixedwide::detail

namespace fixedwide {

namespace {

template<class U>
std::expected<U, ParseError> unsigned_digits(std::string_view text, U limit) noexcept {
    constexpr std::uint64_t base = 1'000'000'000'000'000'000ULL;
    U value{};
    std::size_t count = text.size() % 18;
    if (count == 0) count = 18;
    while (!text.empty()) {
        std::uint64_t chunk{};
        const char* const end = text.data() + count;
        const auto parsed = std::from_chars(text.data(), end, chunk);
        if (parsed.ec != std::errc{} || parsed.ptr != end) return std::unexpected(ParseError::invalid);
        if constexpr (std::is_same_v<U, wide::uint128>) {
            auto divres = detail::divide128by64(limit, base);
            if (value > divres.quotient) return std::unexpected(ParseError::overflow);
            auto prod = detail::multiply128(value, wide::uint128(base));
            if (prod.limbs[2] != 0 || prod.limbs[3] != 0) return std::unexpected(ParseError::overflow);
            value = wide::uint128(prod.limbs[0], prod.limbs[1]);
            if (value > limit - wide::uint128(chunk)) return std::unexpected(ParseError::overflow);
            value += wide::uint128(chunk);
        } else {
            detail::u256_limbs v(value);
            detail::u256_limbs lim(limit);
            auto divres = detail::divmod64(lim, base);
            if (v > divres.quotient) return std::unexpected(ParseError::overflow);
            auto prod_full = detail::mul_full(v, detail::u256_limbs(base));
            for (int i = 4; i < 8; ++i) {
                if (prod_full.limbs[i] != 0) return std::unexpected(ParseError::overflow);
            }
            detail::u256_limbs new_v;
            for (int i = 0; i < 4; ++i) new_v.limbs[i] = prod_full.limbs[i];
            if (new_v > lim - detail::u256_limbs(chunk)) return std::unexpected(ParseError::overflow);
            v = new_v + detail::u256_limbs(chunk);
            value = v.to_uint256();
        }
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
    U limit = positive_limit;
    if (negative) limit += U(1ULL);
    const auto result = unsigned_digits(text, limit);
    if (!result) return std::unexpected(result.error());
    if (negative) {
        if constexpr (std::is_same_v<S, wide::int128>) {
            wide::int128 s(result->low, result->high);
            return -s;
        } else {
            wide::int256 s(result->limbs[0], result->limbs[1], result->limbs[2], result->limbs[3]);
            return -s;
        }
    }
    if constexpr (std::is_same_v<S, wide::int128>) {
        return wide::int128(result->low, result->high);
    } else {
        return wide::int256(result->limbs[0], result->limbs[1], result->limbs[2], result->limbs[3]);
    }
}

template<class U>
std::expected<std::size_t, FormatError> format_integer(char* output, std::size_t capacity, U value, bool negative) noexcept {
    char buffer[128];
    char* const end = buffer + sizeof(buffer);
    char* begin = detail::integer_digits(end, value);
    if (negative) *--begin = '-';
    const auto size = static_cast<std::size_t>(end - begin);
    if (capacity < size) return std::unexpected(FormatError::buffer_too_small);
    std::memcpy(output, begin, size);
    return size;
}

} // namespace

std::expected<wide::int128, ParseError> parse_i128(std::string_view text) noexcept {
    return parse_signed<wide::int128>(text, wide::uint128::max() >> 1);
}

std::expected<wide::uint128, ParseError> parse_u128(std::string_view text) noexcept {
    return parse_unsigned(text, wide::uint128::max());
}

std::expected<wide::int256, ParseError> parse_i256(std::string_view text) noexcept {
    return parse_signed<wide::int256>(text, wide::uint256::max() >> 1);
}

std::expected<wide::uint256, ParseError> parse_u256(std::string_view text) noexcept {
    return parse_unsigned(text, wide::uint256::max());
}

std::expected<std::size_t, FormatError> to_chars(char* output, std::size_t capacity, wide::int128 value) noexcept {
    return format_integer(output, capacity, magnitude(value), value.is_negative());
}

std::expected<std::size_t, FormatError> to_chars(char* output, std::size_t capacity, wide::uint128 value) noexcept {
    return format_integer(output, capacity, value, false);
}

std::expected<std::size_t, FormatError> to_chars(char* output, std::size_t capacity, wide::int256 value) noexcept {
    return format_integer(output, capacity, magnitude(value), value.is_negative());
}

std::expected<std::size_t, FormatError> to_chars(char* output, std::size_t capacity, wide::uint256 value) noexcept {
    return format_integer(output, capacity, value, false);
}

} // namespace fixedwide
