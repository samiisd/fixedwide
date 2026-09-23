// 01 - Quick start: runtime input, a source constant, mixed multiply, output.
// Run without arguments for the built-in example, or pass a decimal price.
#include <fixedwide/all.hpp>
#include <cstdio>

int main(int argc, char* argv[]) {
    using namespace fixedwide;
    if (argc > 2) {
        std::puts("usage: example_01_quick_start [price]");
        return 1;
    }

    // Different widths/scales are distinct types; aliases alone are not units.
    using Price = Fixed64<4>;
    using Quantity = Fixed32<2>;
    using Notional = Fixed128<6>;

    // Source constants are checked during compilation and return the value.
    constexpr auto qty = literal<Quantity>("10.50");

    // External text remains fallible. Parsing is exact unless told otherwise.
    const auto price = parse<Price>(argc == 2 ? argv[1] : "123.4567");
    if (!price) {
        std::puts("parse failed");
        return 1;
    }

    // Name the destination scale and round once into it. Arithmetic remains
    // checked even when one operand came from a compile-time literal.
    const auto notional = mul_to<Notional>(*price, qty, Rounding::nearest_even);
    if (!notional) {
        std::puts("multiply overflowed");
        return 1;
    }

    std::printf("price:    %s\n", to_string(*price).value().c_str());
    std::printf("quantity: %s\n", to_string(qty).value().c_str());
    std::printf("notional: %s\n", to_string(*notional).value().c_str());

    // Comparisons across scales are exact too.
    if (*price > literal<Fixed32<2>>("100")) {
        std::puts("price is above 100.00");
    }
    if (argc == 1 && to_string(*notional).value() != "1296.295350") return 1;
    std::puts("OK");
    return 0;
}
