// 01 - Quick start: parse text, multiply across scales, print the result.
//
// The three steps almost every use of this library goes through. Note that
// every fallible step returns std::expected, so nothing throws and nothing
// silently produces a wrong number.
//
// Docs: ../docs/api_reference.md#primary-types

#include <fixedwide/all.hpp>
#include <cstdio>

int main() {
    using namespace fixedwide;

    // Distinct domain types. A price is not a quantity, and the compiler knows.
    using Price    = Fixed64<4>;    // 4 decimals, 64-bit
    using Quantity = Fixed32<2>;    // 2 decimals, 32-bit
    using Notional = Fixed128<6>;   // 6 decimals, 128-bit

    // parse defaults to Rounding::exact: text that does not land on the type's
    // decimal grid is rejected rather than quietly rounded.
    const auto price = parse<Price>("123.4567");
    const auto qty   = parse<Quantity>("10.50");
    if (!price || !qty) { std::puts("parse failed"); return 1; }

    // Different widths and scales, so the destination must be named. The exact
    // rational product is formed first and rounded once, straight to Notional.
    const auto notional = mul_to<Notional>(*price, *qty, Rounding::nearest_even);
    if (!notional) { std::puts("multiply overflowed"); return 1; }

    std::printf("price     %s\n", to_string(*price).value().c_str());
    std::printf("quantity  %s\n", to_string(*qty).value().c_str());
    std::printf("notional  %s\n", to_string(*notional).value().c_str());

    // Comparison across scales is exact and needs no destination: nothing is
    // rounded to make the two sides comparable.
    if (*price > Quantity::from_raw(10000)) {   // 100.00
        std::puts("price is above 100.00");
    }

    if (to_string(*notional).value() != "1296.295350") { return 1; }
    std::puts("OK");
    return 0;
}
