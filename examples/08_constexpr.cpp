// 08 - Exact source constants composed with checked compile-time arithmetic.
#include <fixedwide/all.hpp>
#include <cstdio>

using namespace fixedwide;
using Money = Fixed64<4>;

// Arithmetic still returns expected. A failed .value() in a constant expression
// is a compile error; at runtime callers must check results before using them.
constexpr Money apply_rate(Money amount, Money rate) {
    return mul(amount, rate, Rounding::nearest_even).value();
}

constexpr auto hundred = literal<Money>("100");
constexpr auto rate = literal<Money>("0.1075");
constexpr auto amount = apply_rate(hundred, rate);
static_assert(amount == literal<Money>("10.75"));
static_assert(add(hundred, hundred).value() == literal<Money>("200"));
static_assert(div(hundred, literal<Money>("3"), Rounding::toward_zero).value() == literal<Money>("33.3333"));
static_assert(quantize(literal<Money>("1.2345"), 2, Rounding::nearest_even).value() == literal<Money>("1.23"));
static_assert(remainder(literal<Money>("10"), literal<Money>("3")).value() == literal<Money>("1"));

// A constant is exact or rejected; there is deliberately no rounding argument.
// constexpr auto bad = literal<Money>("0.00001"); // does not compile
constexpr auto unit = literal<Money>("0.0001");
static_assert(!add(Money::max(), unit).has_value());
static_assert(add(Money::max(), unit).error() == ArithmeticError::overflow);
static_assert(!div(hundred, Money{}).has_value());
static_assert(hundred == literal<Fixed128<8>>("100"));

int main() {
    std::printf("100.0000 * 0.1075 = %s (computed at compile time)\n", to_string(amount).value().c_str());
    std::puts("OK");
    return 0;
}
