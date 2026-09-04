// 07 - Why this library exists: an invoice, in double and in Fixed64<2>.
//
// The same arithmetic, twice. double is a binary float, so 0.01 is not a value
// it has -- errors accumulate and the total drifts. Fixed64<2> is an integer
// count of cents, so the total is the total.
//
// Docs: ../docs/api_reference.md#primary-types

#include <fixedwide/all.hpp>
#include <cstdio>

int main() {
    using namespace fixedwide;
    using Money = Fixed64<2>; // cents
    using Rate = Fixed64<6>;  // a tax rate needs more decimals than money does

    struct Line {
        const char* what;
        const char* unit_price;
        int qty;
    };
    const Line lines[] = {
        {"widget", "0.10", 3},
        {"grommet", "0.20", 4},
        {"flange", "1.15", 7},
        {"sprocket", "19.99", 2},
    };

    double dbl_subtotal = 0.0;
    auto fw_subtotal = Money::from_raw(0);

    for (const auto& l : lines) {
        const auto unit = parse<Money>(l.unit_price).value();
        const auto qty = from_integer<Money>(l.qty).value();

        // Checked, and the overflow would be reported rather than wrapped.
        const auto line_total = mul(unit, qty).value();
        fw_subtotal = add(fw_subtotal, line_total).value();

        dbl_subtotal += std::atof(l.unit_price) * l.qty;
        std::printf("  %-10s %6s x %d = %8s\n", l.what, l.unit_price, l.qty, to_string(line_total).value().c_str());
    }

    // 8.25% tax, applied with a single rounding for the whole expression.
    const auto rate = parse<Rate>("0.082500").value();
    const auto tax = mul_to<Money>(fw_subtotal, rate, Rounding::nearest_away).value();
    const auto total = add(fw_subtotal, tax).value();

    const double dbl_tax = dbl_subtotal * 0.0825;
    const double dbl_total = dbl_subtotal + dbl_tax;

    std::printf("\n              %-18s %s\n", "double", "Fixed64<2>");
    std::printf("subtotal      %-18.17g %s\n", dbl_subtotal, to_string(fw_subtotal).value().c_str());
    std::printf("tax  8.25%%    %-18.17g %s\n", dbl_tax, to_string(tax).value().c_str());
    std::printf("total         %-18.17g %s\n", dbl_total, to_string(total).value().c_str());

    // The exact answers: 0.30 + 0.80 + 8.05 + 39.98 = 49.13; tax 4.053225 -> 4.05.
    if (to_string(fw_subtotal).value() != "49.13") return 1;
    if (to_string(tax).value() != "4.05") return 1;
    if (to_string(total).value() != "53.18") return 1;

    // The double subtotal is not 49.13 -- it is 49.13 plus a little, and the
    // difference is exactly the bug that fixed-point exists to remove.
    if (dbl_subtotal == 49.13)
        std::puts("\n(double happened to land exactly here)");
    else
        std::printf("\ndouble subtotal is off by %.3g\n", dbl_subtotal - 49.13);

    std::puts("OK");
    return 0;
}
