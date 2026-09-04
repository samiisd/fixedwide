// 04 - Mixing widths and scales: name the destination, round exactly once.
//
// add/sub/mul/div take two operands of the SAME type. Mixing has no single
// obvious result type, so it is spelled with a destination: mul_to<Dest>,
// add_to<Dest>, fixed_cast<Dest>. The exact rational result is formed at full
// width and rounded once, straight into Dest -- never rounded twice.
//
// Docs: ../docs/api_reference.md#mixed-scale-operations

#include <fixedwide/all.hpp>
#include <cstdio>

int main() {
    using namespace fixedwide;
    using Price = Fixed64<4>;  // 4 decimals
    using Rate = Fixed64<8>;   // 8 decimals
    using Money = Fixed128<2>; // 2 decimals, wider

    const auto price = parse<Price>("19.9900").value();
    const auto rate = parse<Rate>("1.07500000").value();

    // This would not compile: two different types, no destination named.
    //   auto bad = mul(price, rate);
    // The error names the deleted overload and tells you to use mul_to<Dest>.

    const auto gross = mul_to<Money>(price, rate).value();                    // 21.49
    const auto net = div_to<Money>(gross, Rate::from_raw(100000000)).value(); // / 1.0
    const auto sum = add_to<Money>(price, gross).value();                     // 19.99 + 21.49
    std::printf("gross  %s\n", to_string(gross).value().c_str());
    std::printf("sum    %s\n", to_string(sum).value().c_str());

    if (to_string(gross).value() != "21.49") return 1;
    if (to_string(net).value() != "21.49") return 1;
    if (to_string(sum).value() != "41.48") return 1;

    // Comparison across scales needs no destination and no rounding: both
    // sides are widened to a common exact form, so the answer is never a
    // rounding artefact. 19.99 at 4 decimals equals 19.99 at 8.
    if (!(price == parse<Rate>("19.99000000").value())) return 1;
    if (!(price < parse<Rate>("19.99000001").value())) return 1;

    // fixed_cast defaults to exact, so a narrowing that would lose a digit is
    // an error rather than a silent truncation. Ask for it and you get it.
    const auto exact_ok = fixed_cast<Fixed64<2>>(parse<Price>("19.9900").value());
    if (!exact_ok) return 1;
    const auto refused = fixed_cast<Fixed64<2>>(parse<Price>("19.9999").value());
    if (refused || refused.error() != ArithmeticError::inexact) return 1;
    const auto rounded = fixed_cast<Fixed64<2>>(parse<Price>("19.9999").value(), Rounding::nearest_even);
    if (to_string(rounded.value()).value() != "20.00") return 1;

    // mul_div rounds once for the whole expression: a*b/c, not round(round(a*b)/c).
    const auto fee = mul_div_to<Money>(price, Price::from_raw(150), Price::from_raw(10000)).value();
    std::printf("fee    %s\n", to_string(fee).value().c_str()); // 19.99 * 0.015

    std::puts("OK");
    return 0;
}
