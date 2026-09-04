#include "competitor_common.hpp"

#include <array>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <random>

namespace fixedwide_competitor {

std::uint64_t validations = 0;

[[noreturn]] void fail(std::string_view what) {
    std::fprintf(stderr,
                 "VALIDATION FAILED: %.*s\n",
                 static_cast<int>(what.size()),
                 what.data());
    std::exit(1);
}

void expect(bool condition, std::string_view what) {
    ++validations;
    if (!condition) fail(what);
}

namespace {

std::uint64_t magnitude(std::int64_t value) {
    if (value >= 0) return static_cast<std::uint64_t>(value);
    return static_cast<std::uint64_t>(-(value + 1)) + 1;
}

std::int64_t signed_value(std::uint64_t magnitude_value,
                          std::size_t index,
                          std::size_t period) {
    const auto value = static_cast<std::int64_t>(magnitude_value);
    return index % period == 0 ? -value : value;
}

template<unsigned Decimals>
Fixtures make_exact_fixtures(std::uint64_t seed) {
    constexpr std::int64_t scale = pow10_i64(Decimals);
    constexpr unsigned left_decimals = Decimals / 2;
    constexpr unsigned right_decimals = Decimals - left_decimals;
    constexpr std::int64_t left_step = pow10_i64(left_decimals);
    constexpr std::int64_t right_step = pow10_i64(right_decimals);

    Fixtures fixtures;
    fixtures.decimals = Decimals;
    fixtures.scale = scale;
    fixtures.add.reserve(data_size);
    fixtures.mul.reserve(data_size);
    fixtures.div.reserve(data_size);
    fixtures.text.reserve(data_size);

    std::mt19937_64 rng(seed);
    const auto arbitrary = [&] {
        const auto span = static_cast<std::uint64_t>(9 * scale);
        return static_cast<std::uint64_t>(scale) + rng() % span;
    };
    const auto quantized = [&](std::int64_t step, std::uint64_t span_units) {
        const auto span = span_units * static_cast<std::uint64_t>(scale);
        std::uint64_t raw = static_cast<std::uint64_t>(scale) + rng() % span;
        raw -= raw % static_cast<std::uint64_t>(step);
        return raw;
    };

    for (std::size_t i = 0; i < data_size; ++i) {
        const std::int64_t text_raw = signed_value(arbitrary(), i, 3);
        fixtures.text.push_back({text_raw, fixed_text(text_raw, Decimals)});

        const std::int64_t add_lhs = signed_value(arbitrary(), i, 3);
        const std::int64_t add_rhs = signed_value(arbitrary(), i + 1, 5);
        const __int128 add_full = static_cast<__int128>(add_lhs) + add_rhs;
        if (add_full < std::numeric_limits<std::int64_t>::min() ||
            add_full > std::numeric_limits<std::int64_t>::max()) {
            fail("addition fixture overflowed its oracle");
        }
        const auto add_expected = static_cast<std::int64_t>(add_full);
        fixtures.add.push_back({add_lhs,
                                add_rhs,
                                add_expected,
                                fixed_text(add_lhs, Decimals),
                                fixed_text(add_rhs, Decimals),
                                fixed_text(add_expected, Decimals)});

        // Trailing zero counts sum to Decimals, so no rounding policy is used.
        const std::int64_t mul_lhs =
            signed_value(quantized(left_step, 9), i, 3);
        const std::int64_t mul_rhs =
            signed_value(quantized(right_step, 4), i + 1, 5);
        const __int128 product = static_cast<__int128>(mul_lhs) * mul_rhs;
        if (product % scale != 0) fail("multiplication fixture is not exact");
        const __int128 mul_full = product / scale;
        if (mul_full < std::numeric_limits<std::int64_t>::min() ||
            mul_full > std::numeric_limits<std::int64_t>::max()) {
            fail("multiplication fixture overflowed its oracle");
        }
        const auto mul_expected = static_cast<std::int64_t>(mul_full);
        fixtures.mul.push_back({mul_lhs,
                                mul_rhs,
                                mul_expected,
                                fixed_text(mul_lhs, Decimals),
                                fixed_text(mul_rhs, Decimals),
                                fixed_text(mul_expected, Decimals)});

        // Build a dividend from an exact quotient and divisor.
        const std::int64_t div_expected =
            signed_value(quantized(left_step, 9), i + 2, 3);
        const std::int64_t div_rhs =
            signed_value(quantized(right_step, 4), i + 3, 5);
        const __int128 dividend_product =
            static_cast<__int128>(div_expected) * div_rhs;
        if (dividend_product % scale != 0) fail("division fixture is not exact");
        const __int128 div_lhs_full = dividend_product / scale;
        if (div_lhs_full < std::numeric_limits<std::int64_t>::min() ||
            div_lhs_full > std::numeric_limits<std::int64_t>::max()) {
            fail("division fixture overflowed its oracle");
        }
        const auto div_lhs = static_cast<std::int64_t>(div_lhs_full);
        fixtures.div.push_back({div_lhs,
                                div_rhs,
                                div_expected,
                                fixed_text(div_lhs, Decimals),
                                fixed_text(div_rhs, Decimals),
                                fixed_text(div_expected, Decimals)});
    }

    return fixtures;
}

} // namespace

std::string fixed_text(std::int64_t raw, unsigned decimals) {
    const std::int64_t scale = pow10_i64(decimals);
    const std::uint64_t mag = magnitude(raw);
    const std::uint64_t whole = mag / static_cast<std::uint64_t>(scale);
    const std::uint64_t fraction = mag % static_cast<std::uint64_t>(scale);

    std::array<char, 96> out{};
    char* cursor = out.data();
    char* const last = out.data() + out.size();
    if (raw < 0) *cursor++ = '-';

    const auto whole_result = std::to_chars(cursor, last, whole);
    if (whole_result.ec != std::errc{}) fail("oracle integer formatting failed");
    cursor = whole_result.ptr;

    if (decimals != 0) {
        *cursor++ = '.';
        std::array<char, 32> digits{};
        const auto fraction_result =
            std::to_chars(digits.data(), digits.data() + digits.size(), fraction);
        if (fraction_result.ec != std::errc{}) {
            fail("oracle fractional formatting failed");
        }
        const std::size_t count =
            static_cast<std::size_t>(fraction_result.ptr - digits.data());
        if (count > decimals) fail("oracle produced too many fractional digits");
        const std::size_t zeroes = decimals - count;
        if (static_cast<std::size_t>(last - cursor) < zeroes + count) {
            fail("oracle text buffer exhausted");
        }
        std::memset(cursor, '0', zeroes);
        cursor += zeroes;
        std::memcpy(cursor, digits.data(), count);
        cursor += count;
    }

    return std::string(out.data(), cursor);
}

Fixtures make_scale4_fixtures() {
    return make_exact_fixtures<4>(0x5eed);
}

Fixtures make_scale12_fixtures() {
    return make_exact_fixtures<12>(0xc0ffee);
}

} // namespace fixedwide_competitor
