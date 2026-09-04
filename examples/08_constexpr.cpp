// 08 - All of it at compile time.
//
// mul, div, mul_div, quantize, remainder, add, sub and the comparisons are
// constexpr, so a rate table or a set of conversion constants can be computed
// during compilation and checked by the compiler rather than by a test.
//
// Docs: ../docs/api_reference.md#arithmetic-functions

#include <fixedwide/all.hpp>
#include <cstdio>

using namespace fixedwide;
using Money = Fixed64<4>;

// A whole expression evaluated by the compiler. If any step overflowed, this
// would not compile -- the error is a diagnostic, not a runtime surprise.
constexpr Money apply_rate(Money amount, Money rate) {
    return mul(amount, rate, Rounding::nearest_even).value();
}

constexpr Money hundred = Money::from_raw(1'000'000);   // 100.0000
constexpr Money rate    = Money::from_raw(1'075);       //   0.1075

static_assert(apply_rate(hundred, rate) == Money::from_raw(107'500));   // 10.7500
static_assert(add(hundred, hundred).value() == Money::from_raw(2'000'000));
static_assert(div(hundred, Money::from_raw(30'000), Rounding::toward_zero).value()
              == Money::from_raw(333'333));                              // 33.3333
static_assert(quantize(Money::from_raw(12'345), 2, Rounding::nearest_even).value()
              == Money::from_raw(12'300));                               // 1.2345 -> 1.23
static_assert(remainder(Money::from_raw(100'000), Money::from_raw(30'000)).value()
              == Money::from_raw(10'000));

// Errors are constexpr too: an overflow at compile time is a value you can
// static_assert on, not a hard error, so a table can be validated in place.
static_assert(!add(Money::max(), Money::from_raw(1)).has_value());
static_assert(add(Money::max(), Money::from_raw(1)).error() == ArithmeticError::overflow);
static_assert(!div(hundred, Money::from_raw(0)).has_value());

// Cross-scale comparison, also at compile time and also exact.
static_assert(Money::from_raw(1'000'000) == Fixed128<8>::from_raw(10'000'000'000LL));

int main() {
    // Nothing above ran at runtime; this line is the only work the program does.
    std::printf("100.0000 * 0.1075 = %s (computed at compile time)\n",
                to_string(apply_rate(hundred, rate)).value().c_str());
    std::puts("OK");
    return 0;
}
