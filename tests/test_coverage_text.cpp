#include "coverage_support.hpp"
#include <fixedwide/binary.hpp>
#include <fixedwide/chars.hpp>
#include <fixedwide/hash.hpp>
#include <fixedwide/string.hpp>
#include <algorithm>
#include <span>
#include <string>
#include <unordered_set>

using namespace coverage_test;
using namespace fixedwide;

std::string render(cpp_int raw, unsigned decimals, bool trim = false) {
    const bool negative = raw < 0;
    if (negative) raw = -raw;
    std::string s = raw.convert_to<std::string>();
    if (decimals != 0) {
        if (s.size() <= decimals) s.insert(0, decimals + 1 - s.size(), '0');
        s.insert(s.size() - decimals, 1, '.');
        if (trim) {
            while (s.back() == '0') s.pop_back();
            if (s.back() == '.') s.pop_back();
        }
    }
    if (negative && raw != 0) s.insert(s.begin(), '-');
    return s;
}

template<class T>
void parse_result(const std::string& text, const cpp_int& n, const cpp_int& d, Rounding mode) {
    const auto expected = rounded(n, d, mode, T::bits);
    const auto result = parse<T>(text, mode);
    CHECK(result.has_value() == expected.has_value());
    if (result)
        CHECK(integer(result->raw()) == *expected);
    else
        CHECK(result.error() ==
              (expected.error() == ArithmeticError::overflow ? ParseError::overflow : ParseError::too_precise));
}

template<class T>
void decimals() {
    constexpr unsigned D = T::fractional_digits;
    auto vals = boundaries(T::bits);
    const cpp_int limit = cpp_int{1} << (T::bits - 1);
    std::mt19937_64 rng(0x74657874ULL + D + T::bits);
    for (unsigned i = 0; i < 30; ++i) vals.push_back(random_bits(rng, T::bits) - limit);
    for (const cpp_int& raw : vals) {
        const auto value = T::from_raw(from_integer<typename T::raw_type>(raw));
        const auto canonical = render(raw, D);
        CHECK(parse<T>(canonical) == value);
        CHECK(parse<T>(canonical + "e0") == value);
        const auto scientific = raw.convert_to<std::string>() + "e-" + std::to_string(D);
        CHECK(parse<T>(scientific) == value);
        const auto plus = raw >= 0 ? "+" + canonical : canonical;
        CHECK(parse<T>(plus) == value);
        CHECK(to_string(value).value() == canonical);
        for (unsigned d : std::array<unsigned, 3>{0, D / 2, D})
            for (auto mode : modes)
                for (bool trim : {false, true}) {
                    FormatOptions opts{
                        .digits = d, .trim_trailing_zeros = trim, .rounding = mode, .explicit_digits = true};
                    const auto q = rounded(raw, power10(D - d), mode, 512);
                    char buffer[text_capacity];
                    std::fill_n(buffer, text_capacity, '!');
                    const auto result = to_chars(buffer, sizeof buffer, value, opts);
                    if (!q) {
                        CHECK(!result && result.error() == FormatError::inexact);
                        continue;
                    }
                    const auto expected = render(*q, d, trim);
                    CHECK(result.has_value());
                    if (std::string_view(buffer, *result) != expected)
                        std::fprintf(stderr, "bits=%zu D=%u d=%u mode=%d trim=%d raw=%s expected=%s actual=%.*s\n",
                                     T::bits, D, d, static_cast<int>(mode), trim, raw.convert_to<std::string>().c_str(),
                                     expected.c_str(), static_cast<int>(*result), buffer);
                    CHECK(std::string_view(buffer, *result) == expected);
                    CHECK(buffer[*result] == '!');
                    const auto end = to_chars(buffer, buffer + expected.size(), value, opts);
                    CHECK(end && *end == buffer + expected.size());
                    std::fill_n(buffer, text_capacity, '!');
                    const auto small = to_chars(buffer, expected.size() - 1, value, opts);
                    CHECK(!small && small.error() == FormatError::buffer_too_small);
                    CHECK(buffer[0] == '!');
                    CHECK(to_string(value, opts).value() == expected);
                }
        std::array<std::uint8_t, T::bits / 8 + 17> buffer{};
        const cpp_int raw_bits = raw & ((cpp_int{1} << T::bits) - 1);
        const auto le = to_bytes<endian::little>(value), be = to_bytes<endian::big>(value);
        for (std::size_t i = 0; i < le.size(); ++i) {
            CHECK(le[i] == ((raw_bits >> (8 * i)) & 255).convert_to<unsigned>());
            CHECK(be[be.size() - 1 - i] == le[i]);
        }
        for (std::size_t offset = 0; offset < 17; ++offset) {
            store_unaligned<endian::little>(buffer.data() + offset, value);
            CHECK((load_unaligned<T, endian::little>(buffer.data() + offset) == value));
            store_unaligned<endian::big>(buffer.data() + offset, value);
            CHECK((load_unaligned<T, endian::big>(buffer.data() + offset) == value));
        }
        CHECK((from_bytes<T, endian::little>(le) == value));
        CHECK((from_bytes<T, endian::big>(be) == value));
        CHECK(!from_bytes<T>(std::span<const std::uint8_t>(le.data(), le.size() - 1)));
        std::unordered_set<T> set{value};
        CHECK(set.contains(value));
        CHECK(hash_value(value) == std::hash<T>{}(value));
    }
    for (std::string invalid :
         {"", "+", "-", ".", "1..2", "1e", "1E-", "1E+", "1e1.2", "e0", "x", " 1", "1 ", "--1", "1e++1"}) {
        const auto r = parse<T>(invalid);
        CHECK(!r);
        CHECK(r.error() == (invalid.empty() ? ParseError::empty : ParseError::invalid));
    }
    CHECK(!parse<T>(std::string(4097, '0')));
    CHECK(parse<T>(std::string(4096, '0')) == T{});
    CHECK(parse<T>("0e999999999999999999999999999") == T{});
    CHECK(parse<T>("-0.000e-9999999999999999999999999") == T{});
    CHECK(!parse<T>("1e999999999999999999999999999"));
    for (auto mode : modes)
        for (int sign : {-1, 1}) {
            const std::string prefix = sign < 0 ? "-" : "";
            parse_result<T>(prefix + "1e-1000", sign, power10(1000 - D), mode);
            // Both the plain-decimal fast parser and the exponent/general parser
            // must resolve ties, sticky tails, and rounding-induced overflow alike.
            for (const cpp_int n :
                 std::vector<cpp_int>{1, 4, 5, 6, 9, 10, 15, 25, 49, 50, 51, 59, 60, (limit - 1) * 10 + 5}) {
                const cpp_int signed_n = sign * n;
                const auto s = render(signed_n, D + 1);
                parse_result<T>(s, signed_n, 10, mode);
                parse_result<T>(s + "e0", signed_n, 10, mode);
            }
            parse_result<T>(prefix + "0000000.50000001e-" + std::to_string(D), sign * cpp_int{50000001}, power10(8),
                            mode);
        }
    // Exercise iterator-boundary parsing, exponent padding and sticky tails.
    for (const std::string text : {"0", "1", "-1", "2e0", "1e-1", "7e3", "invalid"}) {
        const auto a = parse<T>(text), b = from_chars<T>(text.data(), text.data() + text.size());
        CHECK(a.has_value() == b.has_value());
        if (a)
            CHECK(*a == *b);
        else
            CHECK(a.error() == b.error());
    }
    for (auto mode : modes) {
        parse_result<T>(render(limit + 1, D) + "e0", limit + 1, 1, mode);
        parse_result<T>(render(-limit - 1, D) + "e0", -limit - 1, 1, mode);
        for (int tail : {101, 149, 150, 151, 199}) parse_result<T>(render(cpp_int{tail}, D + 2), tail, 100, mode);
    }
    const auto zero = T{};
    CHECK(!to_string(zero, {.digits = D + 1, .explicit_digits = true}));
    char b[16]{};
    CHECK(!from_chars<T>(nullptr, b));
    CHECK(!from_chars<T>(b, nullptr));
    CHECK(!from_chars<T>(b + 1, b));
    CHECK(!to_chars(nullptr, b, zero));
    CHECK(!to_chars(b, static_cast<char*>(nullptr), zero));
    CHECK(!to_chars(b + 1, b, zero));
    CHECK(!to_chars(b, b, zero));
}

template<class W>
void integers() {
    constexpr unsigned width = sizeof(W) * 8;
    const bool signed_type = requires(W v) { v.is_negative(); };
    const cpp_int low = signed_type ? -(cpp_int{1} << (width - 1)) : cpp_int{0};
    const cpp_int high = (cpp_int{1} << (signed_type ? width - 1 : width)) - 1;
    auto parse_wide = [](std::string_view s) {
        if constexpr (std::is_same_v<W, i128>)
            return parse_i128(s);
        else if constexpr (std::is_same_v<W, u128>)
            return parse_u128(s);
        else if constexpr (std::is_same_v<W, i256>)
            return parse_i256(s);
        else
            return parse_u256(s);
    };
    std::vector<cpp_int> vals{low, high, 0, 1, 123456789, power10(18) - 1, power10(18), power10(19), power10(20) + 1};
    if (signed_type) vals.push_back(-123456789);
    for (const auto& raw : vals) {
        const W value = from_integer<W>(raw);
        const auto text = raw.convert_to<std::string>();
        CHECK(parse_wide(text) == value);
        CHECK(to_string(value).value() == text);
        if (raw >= 0) CHECK(parse_wide("+" + text) == value);
        CHECK(parse_wide(raw < 0 ? "-000000" + text.substr(1) : "000000" + text) == value);
        char buf[128]{};
        const auto size = to_chars(buf, sizeof buf, value);
        CHECK(size && std::string_view(buf, *size) == text);
        CHECK(!to_chars(buf, text.size() - 1, value));
        std::unordered_set<W> set{value};
        CHECK(set.contains(value));
        const auto le = to_bytes<endian::little>(value), be = to_bytes<endian::big>(value);
        for (std::size_t j = 0; j < le.size(); ++j) CHECK(le[j] == be[le.size() - 1 - j]);
    }
    CHECK(!parse_wide((high + 1).convert_to<std::string>()));
    CHECK(!parse_wide((low - 1).convert_to<std::string>()));
    CHECK(!parse_wide(std::string(180, '9')));
    for (const char* s : {"", "+", "-", "1x", "1.0", "1e0", "--1", " 1", "1 "}) CHECK(!parse_wide(s));
}

void generic_text_entries() {
    for (std::size_t bits : {64u, 128u, 256u})
        for (std::int64_t r : {-12345, 0, 12345}) {
            std::array<char, 128> buf{};
            const auto got = detail::format_fixed_kernel(buf.data(), buf.size(), i256{r}, 4, {.digits = 4}, bits);
            const std::string expected = render(cpp_int{r}, 4);
            CHECK(got && std::string_view(buf.data(), *got) == expected);
        }
}

void run_coverage_text() {
    decimals<Fixed8<0>>();
    decimals<Fixed8<2>>();
    decimals<Fixed16<4>>();
    decimals<Fixed32<9>>();
    decimals<Fixed64<0>>();
    decimals<Fixed64<12>>();
    decimals<Fixed64<18>>();
    decimals<Fixed128<0>>();
    decimals<Fixed128<12>>();
    decimals<Fixed128<38>>();
    decimals<Fixed256<0>>();
    decimals<Fixed256<18>>();
    decimals<Fixed256<76>>();
    generic_text_entries();
    integers<u128>();
    integers<i128>();
    integers<u256>();
    integers<i256>();
    CHECK(parse64("1") == FP64::from_raw(1'000'000'000'000LL));
    CHECK(parse128("-1") == FP128::from_raw(i128{-1'000'000'000'000LL}));
}
