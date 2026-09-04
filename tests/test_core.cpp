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

// Regression: from_integer must be callable with a RUNTIME integer.
//
// detail::max_integer_allowed was `consteval` and took the sign as an argument.
// Because callers pass a runtime bool, C++23 immediate escalation (P2564)
// promoted from_integer itself into an immediate function, so this whole
// function failed to compile under GCC. Every call below is deliberately opaque
// to the optimiser's constant folding.
void test_from_integer_runtime() {
    volatile int runtime_amount = 250;
    volatile long long runtime_big = -1'000'000LL;

    const auto a = from_integer<Fixed64<4>>(static_cast<int>(runtime_amount));
    CHECK(a.has_value());
    CHECK(a->raw() == 2'500'000);

    const auto b = from_integer<Fixed128<12>>(static_cast<long long>(runtime_big));
    CHECK(b.has_value());

    const auto c = from_integer<Fixed32<2>>(static_cast<int>(runtime_amount));
    CHECK(c.has_value());
    CHECK(c->raw() == 25'000);

    // Scale 0 and the overflow edge, still with runtime operands.
    const auto d = from_integer<Fixed16<0>>(static_cast<int>(runtime_amount));
    CHECK(d.has_value());
    volatile int too_big = 400'000;
    const auto e = from_integer<Fixed32<4>>(static_cast<int>(too_big));
    CHECK(!e.has_value());
    CHECK(e.error() == ArithmeticError::overflow);

    // The negative limit is one larger in magnitude than the positive one, and
    // the two are selected by a runtime bool.
    CHECK(from_integer<Fixed8<0>>(static_cast<int>(-128)).has_value());
    CHECK(!from_integer<Fixed8<0>>(static_cast<int>(128)).has_value());
}

int main() {
    test_fixed8();
    test_fixed16();
    test_fixed32();
    test_fixed64();
    test_fixed128();
    test_fixed256();
    test_from_integer_runtime();
    std::printf("test_core passed (%lu checks)\n", checks);
    return 0;
}
