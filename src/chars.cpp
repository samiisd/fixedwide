#include <fixedwide/chars.hpp>
#include "detail.hpp"
#include "limbs.hpp"
#include "text_detail.hpp"
#include <cstring>
#include <string_view>

namespace fixedwide::detail {

namespace {

u512_limbs pow10_512(unsigned exp) noexcept {
    u512_limbs v(1ULL);
    for (unsigned i = 0; i < exp; ++i) {
        v = (v << 3) + (v << 1);
    }
    return v;
}

std::int64_t max_digits_for_bits(std::size_t bits) noexcept {
    if (bits == 8) return 3;
    if (bits == 16) return 5;
    if (bits == 32) return 10;
    if (bits == 64) return 19;
    if (bits == 128) return 39;
    return 78; // 256
}

u256_limbs limit_for_bits_u256(std::size_t bits, bool negative) noexcept {
    auto lim256 = detail::limit_for_bits(bits, negative);
    u256_limbs lim{};
    for (int i = 0; i < 4; ++i) lim.limbs[i] = lim256.limbs[i];
    return lim;
}

} // namespace

std::expected<wide::int256, ParseError>
parse_fixed_kernel(std::string_view text, unsigned decimals,
                   Rounding rounding, std::size_t bits) noexcept {
    if (text.empty()) return std::unexpected(ParseError::empty);
    if (text.size() > 4096) return std::unexpected(ParseError::invalid);

    bool negative = text.front() == '-';
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
        const auto cap = static_cast<std::int64_t>(text.size()) + 256;
        for (; pos < text.size(); ++pos) {
            const char c = text[pos];
            if (c < '0' || c > '9') return std::unexpected(ParseError::invalid);
            const int digit = c - '0';
            exponent = exponent > (cap - digit) / 10 ? cap : exponent * 10 + digit;
        }
        if (exponent_negative) exponent = -exponent;
    }

    if (significant == 0) return wide::int256(0);

    const std::int64_t keep = significant + static_cast<std::int64_t>(decimals) + exponent - fractional;
    const std::int64_t max_d = max_digits_for_bits(bits);
    if (keep > max_d) return std::unexpected(ParseError::overflow);

    if (bits <= 128) {
        auto lim256 = detail::limit_for_bits(bits, negative);
        std::uint64_t lim_lo = lim256.limbs[0];
        std::uint64_t lim_hi = lim256.limbs[1];
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
        unsigned __int128 limit = (static_cast<unsigned __int128>(lim_hi) << 64) | lim_lo;
        unsigned __int128 cutoff = limit / 10;
        unsigned last_digit = static_cast<unsigned>(limit % 10);

        unsigned __int128 value = 0;
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
                if (value > cutoff || (value == cutoff && digit > last_digit)) {
                    return std::unexpected(ParseError::overflow);
                }
                value = value * 10 + digit;
            } else {
                discarded_nonzero |= (digit != 0);
                if (index == keep) first_discarded = digit;
                else tail_nonzero |= (digit != 0);
            }
            ++index;
        }
        for (; index < keep; ++index) {
            if (value > cutoff) return std::unexpected(ParseError::overflow);
            value = value * 10;
        }
        unsigned __int128 remainder = 0;
        if (discarded_nonzero) {
            remainder = 1;
            if (keep >= 0 && first_discarded >= 5) {
                remainder = (first_discarded > 5 || tail_nonzero) ? 6 : 5;
            }
        }
        auto rounded = round_magnitude(wide::uint128(static_cast<std::uint64_t>(value), static_cast<std::uint64_t>(value >> 64)),
                                       wide::uint128(static_cast<std::uint64_t>(remainder), static_cast<std::uint64_t>(remainder >> 64)),
                                       wide::uint128(10, 0), negative, rounding,
                                       wide::uint128(lim_lo, lim_hi));
        if (!rounded) return std::unexpected(rounded.error() == ArithmeticError::overflow ? ParseError::overflow : ParseError::too_precise);
        wide::int128 s(rounded->low, rounded->high);
        if (negative) s = -s;
        return wide::int256(s.low, s.high, (s.is_negative() ? ~0ULL : 0ULL), (s.is_negative() ? ~0ULL : 0ULL));
#endif
    }

    auto limit = limit_for_bits_u256(bits, negative);
    auto cutoff = divmod64(limit, 10).quotient;
    unsigned last_digit = static_cast<unsigned>(divmod64(limit, 10).remainder);

    u256_limbs value{};
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
            if (value > cutoff || (value == cutoff && digit > last_digit)) {
                return std::unexpected(ParseError::overflow);
            }
            value = (value << 3) + (value << 1) + u256_limbs(static_cast<std::uint64_t>(digit));
        } else {
            discarded_nonzero |= (digit != 0);
            if (index == keep) first_discarded = digit;
            else tail_nonzero |= (digit != 0);
        }
        ++index;
    }

    for (; index < keep; ++index) {
        if (value > cutoff) return std::unexpected(ParseError::overflow);
        value = (value << 3) + (value << 1);
    }

    u256_limbs remainder{};
    if (discarded_nonzero) {
        remainder.limbs[0] = 1;
        if (keep >= 0 && first_discarded >= 5) {
            remainder.limbs[0] = (first_discarded > 5 || tail_nonzero) ? 6 : 5;
        }
    }

    u256_limbs u10(10ULL);
    auto rounded = round_magnitude(value, remainder, u10, negative, rounding, limit);
    if (!rounded) {
        return std::unexpected(rounded.error() == ArithmeticError::overflow ? ParseError::overflow : ParseError::too_precise);
    }

    wide::uint256 uq = rounded->to_uint256();
    if (negative) {
        wide::uint256 neg_uq = ~uq + wide::uint256(1ULL);
        return wide::int256(neg_uq.limbs[0], neg_uq.limbs[1], neg_uq.limbs[2], neg_uq.limbs[3]);
    }
    return wide::int256(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
}

// Width-specific entry points. alpha.3 routed every width through one kernel
// taking wide::int256 by value, so formatting a Fixed64<12> widened its raw
// value to 32 bytes and passed it through memory on every call. That alone was
// most of the reduced-digit formatting regression against 0.4.
std::expected<std::size_t, FormatError>
format_fixed_kernel(char* buffer, std::size_t capacity, std::int64_t raw, unsigned decimals,
                    FormatOptions options) noexcept {
    if (options.digits > decimals) return std::unexpected(FormatError::invalid_precision);
    const bool negative = raw < 0;
    const std::uint64_t mag_u64 = negative ? 0ULL - static_cast<std::uint64_t>(raw)
                                           : static_cast<std::uint64_t>(raw);
    unsigned reduce = decimals - options.digits;
    std::uint64_t d = (reduce < 19) ? pow10(reduce) : 0ULL;
    std::uint64_t m = mag_u64;
    std::uint64_t q = 0, r = 0;
    if (d != 0) {
        q = m / d;
        r = m % d;
    } else {
        auto divres = divide128(wide::uint128(m, 0ULL), pow10_wide<wide::uint128>(reduce), false);
        q = divres.quotient.low;
        r = divres.remainder.low;
        d = 0;
    }
    auto rounded = round_magnitude(q, r, d, negative, options.rounding, UINT64_MAX);
    if (!rounded) return std::unexpected(FormatError::inexact);

    char digits_buf[32];
    char* const end = digits_buf + sizeof(digits_buf);
    const char* const begin = integer_digits(end, wide::uint128(*rounded));
    const auto count = static_cast<unsigned>(end - begin);

    char output[text_capacity];
    char* cursor = output;
    if (negative && *rounded != 0) *cursor++ = '-';

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
        if (options.digits != 0) {
            *cursor++ = '.';
            const auto zeros = options.digits - count;
            std::memset(cursor, '0', zeros);
            cursor += zeros;
            std::memcpy(cursor, begin, count);
            cursor += count;
        }
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

std::expected<std::size_t, FormatError>
format_fixed_kernel(char* buffer, std::size_t capacity, wide::int128 raw, unsigned decimals,
                    FormatOptions options) noexcept {
    if (options.digits > decimals) return std::unexpected(FormatError::invalid_precision);
    const bool negative = raw.is_negative();
    const wide::uint128 mag128 = magnitude(raw);
    unsigned reduce = decimals - options.digits;
    auto divisor128 = (reduce < 19) ? wide::uint128(pow10(reduce), 0ULL)
                                    : pow10_wide<wide::uint128>(reduce);
    wide::uint128 q, r;
    if (divisor128.high == 0) {
        std::uint64_t d = divisor128.low;
        if (d == 1) {
            q = mag128;
            r = wide::uint128(0ULL, 0ULL);
        } else if (mag128.high == 0) {
            q = wide::uint128(mag128.low / d, 0);
            r = wide::uint128(mag128.low % d, 0);
        } else {
            std::uint64_t qhi = mag128.high / d;
            std::uint64_t rem_hi = mag128.high % d;
            std::uint64_t rem;
            std::uint64_t qlo = detail::div128by64(rem_hi, mag128.low, d, rem);
            q = wide::uint128(qlo, qhi);
            r = wide::uint128(rem, 0);
        }
    } else {
        auto divres = divide128(mag128, divisor128, false);
        q = divres.quotient;
        r = divres.remainder;
    }
    wide::uint128 limit = wide::uint128::max();
    auto rounded = round_magnitude(q, r, divisor128, negative, options.rounding, limit);
    if (!rounded) return std::unexpected(FormatError::inexact);

    char digits_buf[80];
    char* const end = digits_buf + sizeof(digits_buf);
    const char* const begin = integer_digits(end, *rounded);
    const auto count = static_cast<unsigned>(end - begin);

    char output[text_capacity];
    char* cursor = output;
    if (negative && !rounded->is_zero()) *cursor++ = '-';

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
        if (options.digits != 0) {
            *cursor++ = '.';
            const auto zeros = options.digits - count;
            std::memset(cursor, '0', zeros);
            cursor += zeros;
            std::memcpy(cursor, begin, count);
            cursor += count;
        }
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

std::expected<std::size_t, FormatError>
format_fixed_kernel(char* buffer, std::size_t capacity,
                    wide::int256 raw, unsigned decimals,
                    FormatOptions options, std::size_t bits) noexcept {
    if (bits <= 64) {
        return format_fixed_kernel(buffer, capacity, static_cast<std::int64_t>(raw.limbs[0]),
                                   decimals, options);
    }
    if (bits <= 128) {
        return format_fixed_kernel(buffer, capacity, wide::int128(raw.limbs[0], raw.limbs[1]),
                                   decimals, options);
    }
    if (options.digits > decimals) return std::unexpected(FormatError::invalid_precision);
    auto mag = magnitude(raw);

    u256_limbs vmag(mag);
    unsigned reduce = decimals - options.digits;
    u256_limbs divisor = pow10_512(reduce).to_uint256();

    auto divres = divmod_knuth(vmag, divisor);
    u256_limbs max_lim(~0ULL);
    for (int i = 0; i < 4; ++i) max_lim.limbs[i] = ~0ULL;

    auto rounded = round_magnitude(divres.quotient, divres.remainder, divisor,
                                   raw.is_negative(), options.rounding, max_lim);
    if (!rounded) return std::unexpected(FormatError::inexact);

    char digits_buf[128];
    char* const end = digits_buf + sizeof(digits_buf);
    const char* const begin = integer_digits(end, rounded->to_uint256());
    const auto count = static_cast<unsigned>(end - begin);

    char output[text_capacity];
    char* cursor = output;
    if (raw.is_negative() && !rounded->is_zero()) *cursor++ = '-';

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
        if (options.digits != 0) {
            *cursor++ = '.';
            const auto zeros = options.digits - count;
            std::memset(cursor, '0', zeros);
            cursor += zeros;
            std::memcpy(cursor, begin, count);
            cursor += count;
        }
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

} // namespace fixedwide::detail
