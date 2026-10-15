// 10 - Equity and margin percentage, with one checked publication boundary.
// All intermediate values are ordinary fixed-point values. A failed step
// returns zero; only a successful result() makes the complete formula usable.
// Docs: ../docs/api_reference.md#accumulated-checks

#include <fixedwide/all.hpp>
#include <cstdio>

namespace {
namespace fw = fixedwide;
using Money = fw::Fixed128<10>;
using Percent = fw::Fixed64<6>;
using Integer = fw::Fixed64<0>;
struct Metrics {
    Money equity;
    Percent margin_percent;
};

std::expected<Metrics, fw::ArithmeticError> evaluate(Money balance, Money credit, Money pnl, Money blocked,
                                                     Money used_margin) noexcept {
    fw::Checked c;
    const auto base = c.add(balance, credit);
    const auto equity = c.add(base, pnl);
    const auto available = c.sub(equity, blocked);
    const auto ratio = c.mul_div_to<Percent>(available, Integer::from_raw(100), used_margin);
    return c.result(Metrics{available, ratio});
}
} // namespace

int main() {
    const auto metrics = evaluate(Money{"10000"}, Money{"250"}, Money{"-120.25"}, Money{"300"}, Money{"2000"});
    if (!metrics) return 1;
    if (metrics->equity != Money{"9829.75"} || metrics->margin_percent != Percent{"491.487500"}) return 1;
    // No formatting, storage or external side effects before success.
    std::printf("available equity: %s\nmargin percentage: %s\n", fw::to_string(metrics->equity).value().c_str(),
                fw::to_string(metrics->margin_percent).value().c_str());

    const auto zero_margin = evaluate(Money{"10"}, Money{}, Money{}, Money{}, Money{});
    if (zero_margin || zero_margin.error() != fw::ArithmeticError::division_by_zero) return 1;
    // Overflow comes first; the later zero divisor cannot replace that error.
    const auto overflow = evaluate(Money::max(), Money{"1"}, Money{}, Money{}, Money{});
    if (overflow || overflow.error() != fw::ArithmeticError::overflow) return 1;
    std::puts("OK");
}
