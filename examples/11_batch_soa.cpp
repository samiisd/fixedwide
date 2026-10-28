// Independent products, ordered balance updates, and per-entry rounding.
#include <fixedwide/batch.hpp>
#include <fixedwide/literal.hpp>
#include <array>
#include <cstdio>
#include <span>

namespace fw = fixedwide;
using Money = fw::Fixed128<12>;
using Price = fw::Fixed64<10>;
using WideMoney = fw::Fixed256<12>;
using Status = std::expected<void, fw::ArithmeticError>;

static std::expected<Money, fw::ArithmeticError> replace(Money total, Money previous, Money next) noexcept {
    const auto removed = fw::sub_to<WideMoney>(total, previous, fw::Rounding::exact);
    if (!removed) return std::unexpected(removed.error());
    const auto updated = fw::add_to<WideMoney>(*removed, next, fw::Rounding::exact);
    if (!updated) return std::unexpected(updated.error());
    return fw::fixed_cast<Money>(*updated, fw::Rounding::exact);
}

int main() {
    constexpr std::array<Money, 5> slopes{Money{"1"}, Money{"100"}, Money{"-0.01"}, Money{"0"}, Money{"2"}};
    std::array<Money, 5> values{};
    std::array<Money, 3> totals{};
    const std::array<std::size_t, 5> balance{0, 1, 0, 2, 1}; // duplicate destinations; row 3 is a zero hole
    const auto product = fw::batch::prepare_mul_to<Money>(Price{"0.01"}, fw::Rounding::exact);

    const auto result = fw::batch::for_each(
        slopes.size(), product,
        [&](std::size_t row, Money delta) noexcept -> Status {
            const auto after = fw::add(values[row], delta);
            if (!after) return std::unexpected(after.error());
            const auto total = fw::add(totals[balance[row]], delta);
            if (!total) return std::unexpected(total.error());
            // Check both results before committing either. Duplicate balance
            // destinations retain precisely this physical row order.
            values[row] = *after;
            totals[balance[row]] = *total;
            return {};
        },
        std::span{slopes});
    if (!result || totals[0] != Money{"0.0099"} || totals[1] != Money{"1.02"} || totals[2] != Money{}) return 1;

    // Every boundary is checked separately in a fused row kernel. There is no
    // array-sized intermediate and no change to the two rounding boundaries.
    constexpr std::array<Money, 2> slope{Money{"1"}, Money{"2"}};
    constexpr std::array<Money, 2> residual{Money{"0.1"}, Money{"-0.1"}};
    constexpr std::array<Price, 2> anchors{Price{"99"}, Price{"101"}};
    std::array<Money, 2> next{};
    const Price mark{"100"}; // prices already validated to make subtraction safe
    const auto multiply = fw::batch::mul_to<Money>(fw::Rounding::nearest_even);
    const auto fx = fw::batch::prepare_mul_to<Money>(Money{"1.25"}, fw::Rounding::nearest_even);
    const auto anchored = fw::batch::transform(
        std::span{next},
        [&](Money s, Money c, Price anchor) noexcept -> std::expected<Money, fw::ArithmeticError> {
            const auto amount = multiply(s, Price::from_raw(mark.raw() - anchor.raw()));
            if (!amount) return std::unexpected(amount.error());
            const auto subtotal = fw::add(*amount, c);
            if (!subtotal) return std::unexpected(subtotal.error());
            return fx(*subtotal);
        },
        std::span{slope}, std::span{residual}, std::span{anchors});
    if (!anchored || next[0] != Money{"1.375"} || next[1] != Money{"-2.625"}) return 2;

    // A wide temporary permits the required cancellation beyond Money's range.
    const Money h{"100000000000000000000000000"};
    const auto replacement = replace(h, Money::from_raw(-h.raw()), Money::from_raw(-h.raw()));
    if (!replacement || *replacement != h) return 3;

    // Publication: round each entry, sum in entry order, then apply position FX.
    constexpr std::array<Money, 2> tiny{Money{"0.005"}, Money{"0.005"}};
    const auto sum = fw::batch::transform_reduce(
        tiny.size(), Money{}, fw::batch::prepare_mul_to<Money>(Price{"0.0000000001"}, fw::Rounding::nearest_even),
        std::span{tiny});
    if (!sum || *sum != Money{}) return 4;
    const auto converted = fx(*sum);
    if (!converted || *converted != Money{}) return 5;

    // The same transform also handles comparisons, masks and conversions.
    std::array<bool, 2> positive{};
    if (!fw::batch::transform(std::span{positive}, [](Money x) noexcept { return x > Money{}; }, std::span{next}))
        return 6;
    if (!positive[0] || positive[1]) return 7;
    std::puts("OK: batched SoA arithmetic preserves ordered checked boundaries");
}
