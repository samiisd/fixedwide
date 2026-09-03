#include "check.hpp"
#include <fixedwide/all.hpp>

using namespace fixedwide;

void test_mixed_compare() {
    auto a = Fixed32<4>::from_raw(15'000); // 1.5
    auto b = Fixed128<12>::from_raw(wide::int128(1'500'000'000'000LL)); // 1.5
    CHECK(a == b);
    CHECK(!(a < b));
    CHECK(!(a > b));
    CHECK(a <= b);
    CHECK(a >= b);

    auto c = Fixed64<8>::from_raw(150'000'001LL); // 1.50000001
    CHECK(a < c);
    CHECK(c > a);
    CHECK(b < c);

    auto neg = Fixed8<1>::from_raw(-15); // -1.5
    CHECK(neg < a);
    CHECK(a > neg);
    CHECK(neg < c);
}

void test_mixed_cast() {
    auto a = Fixed32<4>::from_raw(12'345); // 1.2345
    auto b = fixed_cast<Fixed128<12>>(a);
    CHECK(b.has_value());
    CHECK(b->raw() == wide::int128(1'234'500'000'000LL));

    // Lossy cast with exact default fails
    auto lossy_exact = fixed_cast<Fixed64<2>>(a);
    CHECK(!lossy_exact.has_value() && lossy_exact.error() == ArithmeticError::inexact);

    // Lossy cast with nearest_even succeeds
    auto lossy_round = fixed_cast<Fixed64<2>>(a, Rounding::nearest_even);
    CHECK(lossy_round.has_value() && lossy_round->raw() == 123);
}

void test_mixed_arithmetic() {
    using Price = Fixed64<8>;
    using FXRate = Fixed64<12>;
    using Money = Fixed128<12>;

    auto price = Price::from_raw(100'00000000LL);
    auto rate = FXRate::from_raw(1'050000000000LL);

    auto res = mul_to<Money>(price, rate);
    CHECK(res.has_value());
    CHECK(res->raw() == wide::int128(105'000000000000LL));

    auto div_res = div_to<Price>(*res, rate);
    CHECK(div_res.has_value() && div_res->raw() == 100'00000000LL);

    auto sum_res = add_to<Money>(price, rate);
    CHECK(sum_res.has_value() && sum_res->raw() == wide::int128(101'050000000000LL));

    auto sub_res = sub_to<Money>(price, rate);
    CHECK(sub_res.has_value() && sub_res->raw() == wide::int128(98'950000000000LL));

    auto md_res = mul_div_to<Money>(price, rate, FXRate::from_raw(2'000000000000LL));
    CHECK(md_res.has_value() && md_res->raw() == wide::int128(52'500000000000LL));
}

int main() {
    test_mixed_compare();
    test_mixed_cast();
    test_mixed_arithmetic();
    std::printf("test_mixed passed (%lu checks)\n", checks);
    return 0;
}
