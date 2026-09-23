#include "check.hpp"

#include <fixedwide/all.hpp>
#include <fixedwide/format.hpp>
#include <fixedwide/iostream.hpp>

#include <cassert>
#include <cstdint>
#include <string>

void test_readme_opening_snippet() {
    // 1. Binary float 0.01 accumulation inaccuracy
    double binary_total = 0.0;
    for (int i = 0; i < 100; ++i) binary_total += 0.01;
    CHECK(binary_total != 1.0);

    // 2. Machine integer overflow
    std::int64_t a = 5'000'000'000'000'000'000LL;
    CHECK(a > 0);

    // 3. Checked decimal Fixed64<2>
    using Money = fixedwide::Fixed64<2>;
    constexpr auto cent = fixedwide::literal<Money>("0.01");
    Money checked_total{};
    for (int i = 0; i < 100; ++i) {
        checked_total = fixedwide::add(checked_total, cent).value();
    }
    CHECK(to_string(checked_total) == "1.00");

    auto overflow_result = add(Money::max(), cent);
    CHECK(!overflow_result.has_value());
    CHECK(overflow_result.error() == fixedwide::ArithmeticError::overflow);
}

void test_readme_constants_snippet() {
    using Money = fixedwide::Fixed64<2>;
    constexpr auto price = fixedwide::literal<Money>("19.99");
    static_assert(price.raw() == 1999);
    static_assert(fixedwide::literal<Money>("19.9900") == price);
    auto read_price = [](std::string_view text) { return fixedwide::parse<Money>(text); };
    const auto parsed = read_price("19.99");
    CHECK(parsed.has_value() && *parsed == price);
    const auto inexact = read_price("19.999");
    CHECK(!inexact && inexact.error() == fixedwide::ParseError::too_precise);
    const auto invalid = read_price("not a price");
    CHECK(!invalid && invalid.error() == fixedwide::ParseError::invalid);
}

void test_readme_scale12_snippet() {
    using FW12 = fixedwide::Fixed64<12>;
    constexpr auto a = fixedwide::literal<FW12>("123.456789012345");
    constexpr auto b = fixedwide::literal<FW12>("2.000000000000");
    const auto product = fixedwide::mul(a, b);
    CHECK(product.has_value());
    CHECK(product->raw() == 246'913578024690LL);
}

void test_readme_types_snippet() {
    using namespace fixedwide;
    constexpr auto price = literal<Fixed64<4>>("19.9900");
    constexpr auto rate = literal<Fixed64<8>>("1.07500000");

    auto product = mul_to<Fixed128<2>>(price, rate);
    CHECK(product.has_value());
    CHECK(to_string(*product) == "21.49");
    CHECK(price == literal<Fixed64<8>>("19.99000000"));

    auto div_zero = div(price, Fixed64<4>{});
    CHECK(!div_zero.has_value());
    CHECK(div_zero.error() == ArithmeticError::division_by_zero);

    auto div_inexact = div(price, literal<Fixed64<4>>("3.0000"), Rounding::exact);
    CHECK(!div_inexact.has_value());
    CHECK(div_inexact.error() == ArithmeticError::inexact);

    // Banker's rounding (Rounding::nearest_even) default on quantize:
    auto q_even = quantize(literal<Fixed64<2>>("2.50"), 0);
    CHECK(q_even.has_value() && to_string(*q_even) == "2.00");
    auto q_odd = quantize(literal<Fixed64<2>>("3.50"), 0);
    CHECK(q_odd.has_value() && to_string(*q_odd) == "4.00");
}

void test_readme_bankers_rounding_snippet() {
    using namespace fixedwide;
    constexpr auto a = literal<Fixed64<2>>("2.50");
    constexpr auto b = literal<Fixed64<2>>("3.50");
    // Explicit cast across scales:
    CHECK(fixed_cast<Fixed64<0>>(a, Rounding::nearest_even)->raw() == 2);
    CHECK(fixed_cast<Fixed64<0>>(b, Rounding::nearest_even)->raw() == 4);
}

int main() {
    test_readme_opening_snippet();
    test_readme_constants_snippet();
    test_readme_scale12_snippet();
    test_readme_types_snippet();
    test_readme_bankers_rounding_snippet();
    return 0;
}
