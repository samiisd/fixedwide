// 02 - The six rounding modes, side by side on the same division.
//
// Every operation that can lose information takes a Rounding. There is no
// implicit default hidden inside the library: arithmetic defaults to
// nearest_even, parsing and fixed_cast default to exact.
//
// Docs: ../docs/api_reference.md#rounding-modes

#include <fixedwide/all.hpp>
#include <cstdio>

int main() {
    using namespace fixedwide;
    using Money = Fixed64<2>;   // cents

    struct { const char* name; Rounding mode; } modes[] = {
        {"toward_zero ", Rounding::toward_zero},
        {"floor       ", Rounding::floor},
        {"ceil        ", Rounding::ceil},
        {"nearest_even", Rounding::nearest_even},
        {"nearest_away", Rounding::nearest_away},
        {"exact       ", Rounding::exact},
    };

    // Two exact ties, one positive and one negative, so the modes that differ
    // only on ties are actually distinguished. 0.05 / 2 == 0.025, a half-cent.
    const auto tie_pos = parse<Money>("0.05").value();
    const auto tie_neg = parse<Money>("-0.05").value();
    const auto two     = parse<Money>("2.00").value();

    std::puts("  mode          0.05/2    -0.05/2");
    for (const auto& m : modes) {
        const auto pos = div(tie_pos, two, m.mode);
        const auto neg = div(tie_neg, two, m.mode);
        std::printf("  %s  %8s   %8s\n", m.name,
                    pos ? to_string(*pos).value().c_str() : "<inexact>",
                    neg ? to_string(*neg).value().c_str() : "<inexact>");
    }

    // exact is the one that refuses: it reports the loss instead of choosing.
    const auto refused = div(tie_pos, two, Rounding::exact);
    if (refused || refused.error() != ArithmeticError::inexact) return 1;

    // nearest_even is the arithmetic default because it does not drift over a
    // long sum: half the ties go up, half go down.
    if (div(tie_pos, two, Rounding::nearest_even).value() != parse<Money>("0.02").value()) return 1;
    if (div(tie_pos, two, Rounding::nearest_away).value() != parse<Money>("0.03").value()) return 1;
    if (div(tie_neg, two, Rounding::floor).value()        != parse<Money>("-0.03").value()) return 1;
    if (div(tie_neg, two, Rounding::toward_zero).value()  != parse<Money>("-0.02").value()) return 1;

    std::puts("OK");
    return 0;
}
