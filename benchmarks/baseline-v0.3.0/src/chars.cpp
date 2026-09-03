#include <fixedwide/chars.hpp>
#include "detail.hpp"
#include "text_detail.hpp"
#include <cstring>

namespace fixedwide {
inline namespace FIXEDWIDE_SCALE_NAMESPACE {
namespace {
std::expected<i128, ParseError> parse_raw(std::string_view text, Rounding rounding, u128 positive_limit) noexcept {
    if (text.empty()) return std::unexpected(ParseError::empty);
    // Counts/exponents below use signed arithmetic. This bound is far beyond any
    // realizable string, but makes the overflow argument explicit.
    if (text.size() > static_cast<std::size_t>(INT64_MAX / 4)) return std::unexpected(ParseError::invalid);
    const bool negative = text.front() == '-';
    if (negative || text.front() == '+') text.remove_prefix(1);
    if (text.empty()) return std::unexpected(ParseError::invalid);

    bool dot = false;
    std::int64_t digits = 0, fractional = 0, significant = 0;
    std::size_t mantissa_end = 0;
    for (; mantissa_end < text.size(); ++mantissa_end) {
        const char c = text[mantissa_end];
        if (c >= '0' && c <= '9') {
            ++digits;
            fractional += static_cast<int>(dot);
            if (significant != 0 || c != '0') ++significant;
        } else if (c == '.' && !dot) {
            dot = true;
        } else if (c == 'e' || c == 'E') {
            break;
        } else {
            return std::unexpected(ParseError::invalid);
        }
    }
    if (digits == 0) return std::unexpected(ParseError::invalid);

    std::int64_t exponent = 0;
    if (mantissa_end != text.size()) {
        std::size_t pos = mantissa_end + 1;
        bool exponent_negative = false;
        if (pos < text.size() && (text[pos] == '+' || text[pos] == '-')) {
            exponent_negative = text[pos] == '-';
            ++pos;
        }
        if (pos == text.size()) return std::unexpected(ParseError::invalid);
        // Saturation, not rejection: 0e<huge> is zero and 1e-<huge> may round to
        // zero (or one unit under directed rounding). Still validate ALL digits.
        const auto cap = static_cast<std::int64_t>(text.size()) + 128;
        for (; pos < text.size(); ++pos) {
            const char c = text[pos];
            if (c < '0' || c > '9') return std::unexpected(ParseError::invalid);
            const int digit = c - '0';
            exponent = exponent > (cap - digit) / 10 ? cap : exponent * 10 + digit;
        }
        if (exponent_negative) exponent = -exponent;
    }
    if (significant == 0) return i128{0};

    // Work from the FINAL decimal point, not an overflowing intermediate
    // mantissa. Thus a 1000-digit mantissa followed by e-999 can be valid.
    const std::int64_t keep = significant + fractional_digits + exponent - fractional;
    const std::int64_t maximum_digits = positive_limit == static_cast<u128>(INT64_MAX) ? 19 : 39;
    if (keep > maximum_digits) return std::unexpected(ParseError::overflow);
    const u128 limit = positive_limit + static_cast<unsigned>(negative);
    const u128 cutoff = limit / 10;
    const unsigned last_digit = static_cast<unsigned>(limit % 10);
    u128 value = 0;
    std::int64_t index = 0;
    bool started = false, discarded_nonzero = false, tail_nonzero = false;
    unsigned first_discarded = 0;
    for (std::size_t pos = 0; pos < mantissa_end; ++pos) {
        const char c = text[pos];
        if (c == '.') continue;
        const unsigned digit = static_cast<unsigned>(c - '0');
        if (!started && digit == 0) continue;
        started = true;
        if (index < keep) {
            if (value > cutoff || (value == cutoff && digit > last_digit)) return std::unexpected(ParseError::overflow);
            value = value * 10 + digit;
        } else {
            discarded_nonzero |= digit != 0;
            if (index == keep) first_discarded = digit;
            else tail_nonzero |= digit != 0;
        }
        ++index;
    }
    for (; index < keep; ++index) {
        if (value > cutoff) return std::unexpected(ParseError::overflow);
        value *= 10;
    }
    // Rounding only needs zero/below-half/half/above-half, not all discarded
    // digits. Map these classes onto synthetic remainders with denominator 10.
    u128 remainder = 0;
    if (discarded_nonzero) {
        remainder = 1;
        if (keep >= 0 && first_discarded >= 5)
            remainder = first_discarded > 5 || tail_nonzero ? 6 : 5;
    }
    const auto result = detail::round_magnitude(value, remainder, 10, negative, rounding, limit);
    if (!result) return std::unexpected(result.error() == ArithmeticError::overflow ? ParseError::overflow : ParseError::too_precise);
    return detail::apply_sign(*result, negative);
}

std::expected<std::size_t, FormatError> format_raw(char* buffer, std::size_t capacity, i128 value, FormatOptions options) noexcept {
    if (options.digits > fractional_digits) return std::unexpected(FormatError::invalid_precision);
    const auto divisor = detail::pow10(fractional_digits - options.digits);
    const auto mag = magnitude(value);
    const auto rounded = detail::round_magnitude(mag / divisor, mag % divisor, divisor, value < 0,
                                                  options.rounding, u128_max);
    if (!rounded) return std::unexpected(FormatError::inexact);
    char digits[40];
    char* const end = digits + sizeof(digits);
    const char* const begin = detail::integer_digits(end, *rounded);
    const auto count = static_cast<unsigned>(end - begin);
    char output[text_capacity];
    char* cursor = output;
    if (value < 0 && *rounded != 0) *cursor++ = '-';
    if (count > options.digits) {
        const auto integer_count = count - options.digits;
        std::memcpy(cursor, begin, integer_count);
        cursor += integer_count;
        if (options.digits != 0) {
            *cursor++ = '.';
            std::memcpy(cursor, begin + integer_count, options.digits);
            cursor += options.digits;
        }
    } else {
        *cursor++ = '0';
        *cursor++ = '.';
        const auto zeros = options.digits - count;
        std::memset(cursor, '0', zeros);
        cursor += zeros;
        std::memcpy(cursor, begin, count);
        cursor += count;
    }
    if (options.trim_trailing_zeros && options.digits != 0) {
        while (cursor[-1] == '0') --cursor;
        if (cursor[-1] == '.') --cursor;
    }
    const auto size = static_cast<std::size_t>(cursor - output);
    if (capacity < size) return std::unexpected(FormatError::buffer_too_small);
    std::memcpy(buffer, output, size);
    return size;
}
} // namespace
std::expected<FP64, ParseError> parse64(std::string_view text, Rounding rounding) noexcept {
    const auto raw = parse_raw(text, rounding, INT64_MAX);
    if (!raw) return std::unexpected(raw.error());
    return FP64::from_raw(static_cast<std::int64_t>(*raw));
}
std::expected<FP128, ParseError> parse128(std::string_view text, Rounding rounding) noexcept {
    const auto raw = parse_raw(text, rounding, static_cast<u128>(i128_max));
    if (!raw) return std::unexpected(raw.error());
    return FP128::from_raw(*raw);
}
std::expected<std::size_t, FormatError> to_chars(char* buffer, std::size_t capacity, FP64 value, FormatOptions options) noexcept {
    return format_raw(buffer, capacity, value.raw(), options);
}
std::expected<std::size_t, FormatError> to_chars(char* buffer, std::size_t capacity, FP128 value, FormatOptions options) noexcept {
    return format_raw(buffer, capacity, value.raw(), options);
}
} // inline namespace
} // namespace fixedwide
