#include "check.hpp"
#include <fixedwide/all.hpp>
#include <string_view>

using namespace fixedwide;

void test_chars_parsing() {
    auto r1 = parse<Fixed64<4>>("123.4567");
    CHECK(r1.has_value() && r1->raw() == 1234567LL);

    auto r2 = parse<Fixed64<4>>("-123.4567");
    CHECK(r2.has_value() && r2->raw() == -1234567LL);

    auto r3 = parse<Fixed64<4>>(".5");
    CHECK(r3.has_value() && r3->raw() == 5000LL);

    auto r4 = parse<Fixed64<4>>("100.");
    CHECK(r4.has_value() && r4->raw() == 1000000LL);

    auto r5 = parse<Fixed64<4>>("1.25e2");
    CHECK(r5.has_value() && r5->raw() == 1250000LL);

    // Invalid inputs
    CHECK(!parse<Fixed64<4>>(""));
    CHECK(!parse<Fixed64<4>>("+"));
    CHECK(!parse<Fixed64<4>>("-"));
    CHECK(!parse<Fixed64<4>>("."));
    CHECK(!parse<Fixed64<4>>("abc"));
    CHECK(!parse<Fixed64<4>>("1.2.3"));

    // Huge exponent doesn't loop forever or crash
    CHECK(!parse<Fixed64<4>>("1e999999999999999999"));

    // Exact parsing
    auto rexact = parse<Fixed64<2>>("12.345", Rounding::exact);
    CHECK(!rexact.has_value() && rexact.error() == ParseError::too_precise);

    auto rround = parse<Fixed64<2>>("12.345", Rounding::nearest_even);
    CHECK(rround.has_value() && rround->raw() == 1234LL || rround->raw() == 1235LL);
}

void test_chars_formatting() {
    char buf[64];
    auto val = Fixed64<4>::from_raw(1234567LL);
    auto res = to_chars(buf, sizeof(buf), val);
    CHECK(res.has_value());
    CHECK(std::string_view(buf, *res) == "123.4567");

    // Zero
    auto res0 = to_chars(buf, sizeof(buf), Fixed64<2>{});
    CHECK(res0.has_value() && std::string_view(buf, *res0) == "0.00");

    // Negative
    auto res_neg = to_chars(buf, sizeof(buf), Fixed64<2>::from_raw(-150));
    CHECK(res_neg.has_value() && std::string_view(buf, *res_neg) == "-1.50");

    // Trim trailing zeros
    auto res_trim =
        to_chars(buf, sizeof(buf), Fixed64<4>::from_raw(1234500LL), FormatOptions{.trim_trailing_zeros = true});
    CHECK(res_trim.has_value() && std::string_view(buf, *res_trim) == "123.45");

    // Buffer too small does not modify buffer
    buf[0] = 'Z';
    auto res_small = to_chars(buf, 2, val);
    CHECK(!res_small.has_value() && res_small.error() == FormatError::buffer_too_small);
    CHECK(buf[0] == 'Z');
}

void test_roundtrip() {
    using F = Fixed128<18>;
    std::string original = "987654321012345678.123456789012345678";
    auto parsed = parse<F>(original);
    CHECK(parsed.has_value());

    char buf[128];
    auto formatted = to_chars(buf, sizeof(buf), *parsed);
    CHECK(formatted.has_value());
    CHECK(std::string_view(buf, *formatted) == original);
}

int main() {
    test_chars_parsing();
    test_chars_formatting();
    test_roundtrip();
    std::printf("test_chars passed (%lu checks)\n", checks);
    return 0;
}
