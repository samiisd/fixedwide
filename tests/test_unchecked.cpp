#include <fixedwide/unchecked.hpp>
#include "check.hpp"
#include <array>
#include <type_traits>
#include <utility>

namespace fw = fixedwide;
namespace fast = fixedwide::unchecked;

template<class A, class B>
concept CanAdd = requires(A a, B b) { a + b; };
template<class A, class B>
concept CanMultiply = requires(A a, B b) { a * b; };
template<class A, class B>
concept CanAddAssign = requires(A a, B b) { a += b; };

using Price = fw::Fixed64<8>;
static_assert(std::is_same_v<decltype(Price{} + Price{}), Price>);
static_assert(std::is_same_v<decltype(std::declval<Price&>() += Price{}), Price&>);
static_assert(std::is_same_v<decltype(fw::add(Price{}, Price{})), std::expected<Price, fw::ArithmeticError>>);
static_assert(noexcept(Price{} + Price{}));
static_assert(noexcept(Price{} * Price{}));
static_assert(noexcept(std::declval<Price&>() /= Price{}));
static_assert(!CanAdd<Price, fw::Fixed64<4>>);
static_assert(!CanAdd<Price, fw::Fixed128<8>>);
static_assert(!CanMultiply<Price, fw::Fixed64<4>>);
static_assert(!CanAddAssign<Price, fw::Fixed128<8>>);
static_assert(!CanAdd<Price, int>);
static_assert(!CanMultiply<Price, double>);
static_assert(sizeof(Price) == sizeof(std::int64_t));
static_assert(std::is_trivially_copyable_v<Price>);

constexpr bool constant_arithmetic() {
    using F = fw::Fixed64<2>;
    auto a = F::from_raw(150);
    const auto b = F::from_raw(200);
    a += b;
    a -= b;
    a *= b;
    a /= b;
    a %= b;
    return a.raw() == 150 && (-a).raw() == -150 && (+a) == a && fast::abs(-a) == a && fast::mul_div(a, b, b) == a &&
           fast::midpoint(a, b).raw() == 175 && fast::from_integer<F>(1).raw() == 100;
}
static_assert(constant_arithmetic());
static_assert((fw::Fixed256<0>::max() + fw::Fixed256<0>::min()).raw() == fw::wide::int256(-1));

constexpr std::array modes{fw::Rounding::nearest_even, fw::Rounding::nearest_away, fw::Rounding::toward_zero,
                           fw::Rounding::floor,        fw::Rounding::ceil,         fw::Rounding::exact};

template<class F>
void pair(F a, F b) {
    if (auto value = fw::add(a, b)) {
        CHECK(a + b == *value);
        auto copy = a;
        CHECK(&(copy += b) == &copy);
        CHECK(copy == *value);
    }
    if (auto value = fw::sub(a, b)) {
        CHECK(a - b == *value);
        auto copy = a;
        CHECK(&(copy -= b) == &copy);
        CHECK(copy == *value);
    }
    for (auto mode : modes) {
        if (auto value = fw::mul(a, b, mode)) CHECK(fast::mul(a, b, mode) == *value);
        if (auto value = fw::div(a, b, mode)) CHECK(fast::div(a, b, mode) == *value);
        if (auto value = fw::mul_div(a, b, b, mode)) CHECK(fast::mul_div(a, b, b, mode) == *value);
        if (auto value = fw::midpoint(a, b, mode)) CHECK(fast::midpoint(a, b, mode) == *value);
    }
    if (auto value = fw::mul(a, b)) {
        CHECK(a * b == *value);
        auto copy = a;
        CHECK(&(copy *= b) == &copy);
        CHECK(copy == *value);
    }
    if (auto value = fw::div(a, b)) {
        CHECK(a / b == *value);
        auto copy = a;
        CHECK(&(copy /= b) == &copy);
        CHECK(copy == *value);
    }
    if (auto value = fw::remainder(a, b)) {
        CHECK(a % b == *value);
        auto copy = a;
        CHECK(&(copy %= b) == &copy);
        CHECK(copy == *value);
    }
}

template<class F>
void type_cases() {
    using Raw = typename F::raw_type;
    const auto one = F::from_raw(Raw{1});
    const auto minus_one = F::from_raw(Raw{-1});
    const auto a = F::from_raw(Raw{3});
    const auto b = F::from_raw(Raw{7});
    const std::array values{F::min(), F::min() + one, minus_one, F{}, one, a, b, F::max() - one, F::max()};
    for (auto x : values) {
        for (auto y : values) pair(x, y);
        if (x != F::min()) {
            CHECK(-x == *fw::negate(x));
            CHECK(fast::abs(x) == *fw::abs(x));
        }
        CHECK(+x == x);
        CHECK(fast::quantize(x, F::fractional_digits) == x);
    }
    CHECK(fast::midpoint(a, b, F::fractional_digits) == F::from_raw(Raw{5}));
    CHECK(fast::from_integer<F>(0) == F{});
    CHECK(F::min() % minus_one == F{});
    auto self = a;
    self += self;
    CHECK(self == F::from_raw(Raw{6}));
    self -= self;
    CHECK(self == F{});

    // Checked errors remain errors. Never execute invalid unchecked arithmetic.
    CHECK(fw::add(F::max(), one).error() == fw::ArithmeticError::overflow);
    CHECK(fw::sub(F::min(), one).error() == fw::ArithmeticError::overflow);
    CHECK(fw::negate(F::min()).error() == fw::ArithmeticError::overflow);
    CHECK(fw::div(a, F{}).error() == fw::ArithmeticError::division_by_zero);
}

void mixed_cases() {
    using A = fw::Fixed64<2>;
    using B = fw::Fixed32<4>;
    using Dest = fw::Fixed128<6>;
    const auto a = A::from_raw(125);
    const auto b = B::from_raw(30000);
    CHECK(fast::fixed_cast<Dest>(a) == *fw::fixed_cast<Dest>(a));
    for (auto mode : modes) {
        CHECK(fast::add_to<Dest>(a, b, mode) == *fw::add_to<Dest>(a, b, mode));
        CHECK(fast::sub_to<Dest>(a, b, mode) == *fw::sub_to<Dest>(a, b, mode));
        CHECK(fast::mul_to<Dest>(a, b, mode) == *fw::mul_to<Dest>(a, b, mode));
        if (auto expected = fw::div_to<Dest>(a, b, mode)) CHECK(fast::div_to<Dest>(a, b, mode) == *expected);
        CHECK(fast::mul_div_to<Dest>(a, b, b, mode) == *fw::mul_div_to<Dest>(a, b, b, mode));
    }
    using F = fw::Fixed64<12>;
    const auto x = F::from_raw(123456789012345LL);
    const auto twice = F::from_raw(2000000000000LL);
    CHECK((x * twice).raw() == 246913578024690LL);
    CHECK(fast::mul_div(F::max(), F::max(), F::max()) == F::max());
    CHECK(fast::mul_div(F::min(), F::min(), F::min()) == F::min());
    CHECK(fast::div(A::from_raw(-1), A::from_raw(300), fw::Rounding::floor).raw() == -1);
    CHECK(fast::div(A::from_raw(-1), A::from_raw(300), fw::Rounding::ceil).raw() == 0);
    CHECK(fast::quantize(A::from_raw(150), 0).raw() == 200);
}

int main() {
    type_cases<fw::Fixed8<0>>();
    type_cases<fw::Fixed8<2>>();
    type_cases<fw::Fixed16<0>>();
    type_cases<fw::Fixed16<4>>();
    type_cases<fw::Fixed32<0>>();
    type_cases<fw::Fixed32<9>>();
    type_cases<fw::Fixed64<0>>();
    type_cases<fw::Fixed64<8>>();
    type_cases<fw::Fixed64<18>>();
    type_cases<fw::Fixed128<0>>();
    type_cases<fw::Fixed128<19>>();
    type_cases<fw::Fixed128<38>>();
    type_cases<fw::Fixed256<0>>();
    type_cases<fw::Fixed256<76>>();
    mixed_cases();
    std::printf("unchecked: %llu checks OK\n", static_cast<unsigned long long>(checks));
}
