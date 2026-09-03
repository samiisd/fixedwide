#include "check.hpp"
#include <fixedwide/all.hpp>

using namespace fixedwide;

void test_fixed8() {
    using F = Fixed8<1>;
    auto a = F::from_raw(15);
    auto b = F::from_raw(20);
    auto s = add(a, b);
    CHECK(s.has_value() && s->raw() == 35);
    auto diff = sub(b, a);
    CHECK(diff.has_value() && diff->raw() == 5);
    auto p = mul(a, b);
    CHECK(p.has_value() && p->raw() == 30);
    auto q = div(*p, a);
    CHECK(q.has_value() && q->raw() == 20);

    // Overflow
    CHECK(!add(F::max(), F::from_raw(1)));
    CHECK(!sub(F::min(), F::from_raw(1)));
    CHECK(!negate(F::min()));
    CHECK(!abs(F::min()));
    CHECK(!div(a, F{})); // div by zero
}

void test_fixed16() {
    using F = Fixed16<3>;
    auto a = *from_integer<F>(5);
    auto b = *from_integer<F>(2);
    auto s = *add(a, b);
    CHECK(s.raw() == 7000);
    auto p = *mul(a, b);
    CHECK(p.raw() == 10000);
    auto q = *div(a, b);
    CHECK(q.raw() == 2500);
}

void test_fixed32() {
    using F = Fixed32<4>;
    auto a = *from_integer<F>(100);
    auto b = *from_integer<F>(4);
    CHECK(mul(a, b)->raw() == 4000000);
    CHECK(div(a, b)->raw() == 250000);
    CHECK(remainder(a, b)->raw() == 0);
}

void test_fixed64() {
    using F = Fixed64<12>;
    auto a = *from_integer<F>(15);
    auto b = *from_integer<F>(2);
    auto p = *mul(a, b);
    CHECK(p == *from_integer<F>(30));
    auto q = *div(p, b);
    CHECK(q == a);

    // mul_div
    auto md = *mul_div(a, b, *from_integer<F>(3));
    CHECK(md == *from_integer<F>(10));

    // Quantize
    auto qz = *quantize(F::from_raw(1234567890123LL), 6);
    CHECK(qz.raw() == 1234568000000LL);

    // Backward compat aliases
    CHECK(sizeof(FP64) == 8);
    CHECK(FP64::max() == fp64_max);
    CHECK(FP64::min() == fp64_min);
}

void test_fixed128() {
    using F = Fixed128<12>;
    auto a = *from_integer<F>(1000000);
    auto b = *from_integer<F>(2000000);
    auto p = *mul(a, b);
    auto q = *div(p, a);
    CHECK(q == b);

    // Backward compat aliases
    CHECK(sizeof(FP128) == 16);
    CHECK(FP128::max() == fp128_max);
}

void test_fixed256() {
    using F = Fixed256<18>;
    auto a = *from_integer<F>(123456789);
    auto b = *from_integer<F>(987654321);
    auto p = *mul(a, b);
    auto q = *div(p, a);
    CHECK(q == b);
}

int main() {
    test_fixed8();
    test_fixed16();
    test_fixed32();
    test_fixed64();
    test_fixed128();
    test_fixed256();
    std::printf("test_core passed (%lu checks)\n", checks);
    return 0;
}
