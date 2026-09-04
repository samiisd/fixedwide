// 03 - Errors are values: overflow, division by zero, and refusing to round.
//
// No operation throws, sets errno, or returns a wrong answer in place of an
// error. Every failure is a std::expected you have to look at, and C++23's
// monadic and_then / transform / or_else chain them without a pyramid of ifs.
//
// Docs: ../docs/api_reference.md#error-types

#include <fixedwide/all.hpp>
#include <cstdio>

using namespace fixedwide;
using Money = Fixed32<2>; // 32-bit on purpose: it overflows where you can see it

const char* name(ArithmeticError e) {
    switch (e) {
    case ArithmeticError::overflow: return "overflow";
    case ArithmeticError::division_by_zero: return "division_by_zero";
    case ArithmeticError::inexact: return "inexact";
    case ArithmeticError::invalid_precision: return "invalid_precision";
    case ArithmeticError::invalid_value: return "invalid_value";
    }
    return "?";
}

int main() {
    // 1. Overflow. Fixed32<2> tops out near 21,474,836.47.
    const auto big = Money::max();
    const auto over = add(big, parse<Money>("0.01").value());
    if (over || over.error() != ArithmeticError::overflow) return 1;
    std::printf("max + 0.01        -> %s\n", name(over.error()));

    // 2. Division by zero is an error, not undefined behaviour and not a NaN.
    const auto dz = div(parse<Money>("1.00").value(), Money::from_raw(0));
    if (dz || dz.error() != ArithmeticError::division_by_zero) return 1;
    std::printf("1.00 / 0          -> %s\n", name(dz.error()));

    // 3. Rounding::exact refuses to lose a digit rather than choosing for you.
    const auto ex = div(parse<Money>("1.00").value(), parse<Money>("3.00").value(), Rounding::exact);
    if (ex || ex.error() != ArithmeticError::inexact) return 1;
    std::printf("1.00 / 3.00 exact -> %s\n", name(ex.error()));

    // 4. Chaining. Each step runs only if the previous one produced a value,
    //    and the first error falls straight through to the end.
    const auto chained = parse<Money>("100.00")
                             .transform_error([](ParseError) { return ArithmeticError::invalid_value; })
                             .and_then([](Money m) { return mul(m, parse<Money>("1.20").value()); })
                             .and_then([](Money m) { return div(m, parse<Money>("4.00").value()); });
    if (!chained || to_string(*chained).value() != "30.00") return 1;
    std::printf("100 * 1.20 / 4    -> %s\n", to_string(*chained).value().c_str());

    // The same chain, where the middle step overflows: nothing after it runs.
    const auto failed = std::expected<Money, ArithmeticError>{Money::max()}
                            .and_then([](Money m) { return mul(m, parse<Money>("2.00").value()); })
                            .and_then([](Money m) { return div(m, parse<Money>("4.00").value()); });
    if (failed || failed.error() != ArithmeticError::overflow) return 1;
    std::printf("max * 2 / 4       -> %s\n", name(failed.error()));

    // value_or, when a default really is the right answer.
    const auto fallback = div(parse<Money>("1.00").value(), Money::from_raw(0)).value_or(Money::from_raw(0));
    if (fallback != Money::from_raw(0)) return 1;

    std::puts("OK");
    return 0;
}
