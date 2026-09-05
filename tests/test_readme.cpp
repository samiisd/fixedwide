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
    auto checked_total = Money::from_raw(0);
    for (int i = 0; i < 100; ++i) {
        checked_total = add(checked_total, fixedwide::parse<Money>("0.01").value()).value();
    }
    CHECK(to_string(checked_total) == "1.00");

    auto overflow_result = add(Money::max(), fixedwide::parse<Money>("0.01").value());
    CHECK(!overflow_result.has_value());
    CHECK(overflow_result.error() == fixedwide::ArithmeticError::overflow);
}

void test_readme_scale12_snippet() {
    using FW12 = fixedwide::Fixed64<12>;
    const auto a = FW12::from_raw(123'456789012345LL);
    const auto b = FW12::from_raw(2'000000000000LL);
    const auto product = fixedwide::mul(a, b);
    CHECK(product.has_value());
    CHECK(product->raw() == 246'913578024690LL);
}

void test_readme_types_snippet() {
    using namespace fixedwide;
    auto price = parse<Fixed64<4>>("19.9900").value();
    auto rate = parse<Fixed64<8>>("1.07500000").value();

    auto product = mul_to<Fixed128<2>>(price, rate);
    CHECK(product.has_value());
    CHECK(to_string(*product) == "21.49");
    CHECK(price == parse<Fixed64<8>>("19.99000000").value());

    auto div_zero = div(price, Fixed64<4>::from_raw(0));
    CHECK(!div_zero.has_value());
    CHECK(div_zero.error() == ArithmeticError::division_by_zero);

    auto div_inexact = div(price, parse<Fixed64<4>>("3.0000").value(), Rounding::exact);
    CHECK(!div_inexact.has_value());
    CHECK(div_inexact.error() == ArithmeticError::inexact);
}

int main() {
    test_readme_opening_snippet();
    test_readme_scale12_snippet();
    test_readme_types_snippet();
    return 0;
}
