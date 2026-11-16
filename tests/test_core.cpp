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

template<typename F>
consteval bool check_zero_compare_constexpr() {
    constexpr F zero{};
    constexpr F pos = F::from_raw(1);
    constexpr F neg = F::from_raw(-1);
    constexpr F hi = F::max();
    constexpr F lo = F::min();

    static_assert(noexcept(zero == 0) && noexcept(0 == zero));
    static_assert(noexcept(zero <=> 0) && noexcept(0 <=> zero));
    static_assert(!std::is_constructible_v<F, int>);
    static_assert(!std::is_convertible_v<int, F>);

    bool ok = true;
    ok = ok && (zero == 0) && !(zero != 0) && !(zero < 0) && (zero <= 0) && !(zero > 0) && (zero >= 0);
    ok = ok && (0 == zero) && !(0 != zero) && !(0 < zero) && (0 <= zero) && !(0 > zero) && (0 >= zero);
    ok = ok && ((zero <=> 0) == std::strong_ordering::equal) && ((0 <=> zero) == std::strong_ordering::equal);

    ok = ok && !(pos == 0) && (pos != 0) && !(pos < 0) && !(pos <= 0) && (pos > 0) && (pos >= 0);
    ok = ok && !(0 == pos) && (0 != pos) && (0 < pos) && (0 <= pos) && !(0 > pos) && !(0 >= pos);
    ok = ok && ((pos <=> 0) == std::strong_ordering::greater) && ((0 <=> pos) == std::strong_ordering::less);

    ok = ok && !(neg == 0) && (neg != 0) && (neg < 0) && (neg <= 0) && !(neg > 0) && !(neg >= 0);
    ok = ok && !(0 == neg) && (0 != neg) && !(0 < neg) && !(0 <= neg) && (0 > neg) && (0 >= neg);
    ok = ok && ((neg <=> 0) == std::strong_ordering::less) && ((0 <=> neg) == std::strong_ordering::greater);

    ok = ok && (hi > 0) && (0 < hi) && (hi != 0) && (lo < 0) && (0 > lo) && (lo != 0);
    return ok;
}

template<typename F>
void check_zero_compare_runtime() {
    static_assert(check_zero_compare_constexpr<F>());

    volatile int z_raw = 0;
    volatile int p_raw = 1;
    volatile int n_raw = -1;
    const F zero = F::from_raw(static_cast<typename F::raw_type>(static_cast<int>(z_raw)));
    const F pos = F::from_raw(static_cast<typename F::raw_type>(static_cast<int>(p_raw)));
    const F neg = F::from_raw(static_cast<typename F::raw_type>(static_cast<int>(n_raw)));
    const F hi = F::max();
    const F lo = F::min();

    CHECK(zero == 0);
    CHECK(!(zero != 0));
    CHECK(!(zero < 0));
    CHECK(zero <= 0);
    CHECK(!(zero > 0));
    CHECK(zero >= 0);
    CHECK(0 == zero);
    CHECK(!(0 != zero));
    CHECK(!(0 < zero));
    CHECK(0 <= zero);
    CHECK(!(0 > zero));
    CHECK(0 >= zero);
    CHECK((zero <=> 0) == 0);
    CHECK((0 <=> zero) == 0);

    CHECK(!(pos == 0));
    CHECK(pos != 0);
    CHECK(!(pos < 0));
    CHECK(!(pos <= 0));
    CHECK(pos > 0);
    CHECK(pos >= 0);
    CHECK(!(0 == pos));
    CHECK(0 != pos);
    CHECK(0 < pos);
    CHECK(0 <= pos);
    CHECK(!(0 > pos));
    CHECK(!(0 >= pos));
    CHECK((pos <=> 0) > 0);
    CHECK((0 <=> pos) < 0);

    CHECK(!(neg == 0));
    CHECK(neg != 0);
    CHECK(neg < 0);
    CHECK(neg <= 0);
    CHECK(!(neg > 0));
    CHECK(!(neg >= 0));
    CHECK(!(0 == neg));
    CHECK(0 != neg);
    CHECK(!(0 < neg));
    CHECK(!(0 <= neg));
    CHECK(0 > neg);
    CHECK(0 >= neg);
    CHECK((neg <=> 0) < 0);
    CHECK((0 <=> neg) > 0);

    CHECK(hi > 0 && 0 < hi && hi != 0);
    CHECK(lo < 0 && 0 > lo && lo != 0);
}

void test_zero_comparisons() {
    check_zero_compare_runtime<Fixed8<0>>();
    check_zero_compare_runtime<Fixed8<2>>();
    check_zero_compare_runtime<Fixed16<0>>();
    check_zero_compare_runtime<Fixed16<4>>();
    check_zero_compare_runtime<Fixed32<0>>();
    check_zero_compare_runtime<Fixed32<4>>();
    check_zero_compare_runtime<Fixed64<0>>();
    check_zero_compare_runtime<Fixed64<2>>();
    check_zero_compare_runtime<Fixed64<18>>();
    check_zero_compare_runtime<Fixed128<0>>();
    check_zero_compare_runtime<Fixed128<12>>();
    check_zero_compare_runtime<Fixed128<38>>();
    check_zero_compare_runtime<Fixed256<0>>();
    check_zero_compare_runtime<Fixed256<18>>();
    check_zero_compare_runtime<Fixed256<76>>();
}

int main() {
    test_fixed8();
    test_fixed16();
    test_fixed32();
    test_fixed64();
    test_fixed128();
    test_fixed256();
    test_from_integer_runtime();
    test_zero_comparisons();
    std::printf("test_core passed (%lu checks)\n", checks);
    return 0;
}
