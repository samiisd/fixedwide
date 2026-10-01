#include <fixedwide/all.hpp>
#include <array>
#include <cstdio>
#include <span>
#include <string_view>

namespace fw = fixedwide;
using Price = fw::Fixed64<8>;

enum class InputError { invalid_number, outside_domain };

std::expected<Price, InputError> read_bounded(std::string_view text, Price low, Price high) {
    auto value = fw::parse<Price>(text);
    if (!value) return std::unexpected(InputError::invalid_number);
    if (*value < low || *value > high) return std::unexpected(InputError::outside_domain);
    return *value;
}

// Preconditions: abs(price) <= 1,000,000 and abs(delta) <= 10,000.
// One shift stays within +/-1,010,000. Repeated shifts need a new bound.
void translate(std::span<Price> prices, Price delta) noexcept {
    for (auto& price : prices) price += delta;
}

int main() {
    constexpr Price price_limit = "1000000";
    constexpr Price delta_limit = "10000";
    static_assert(*fw::add(price_limit, delta_limit) < Price::max());
    static_assert(*fw::sub(-price_limit, delta_limit) > Price::min());

    const std::array<std::string_view, 3> input{"125.50", "125.49", "125.48"};
    std::array<Price, 3> prices{};
    for (std::size_t i = 0; i < input.size(); ++i) {
        auto price = read_bounded(input[i], -price_limit, price_limit);
        if (!price) return 1;
        prices[i] = *price;
    }
    const auto delta = read_bounded("0.0025", -delta_limit, delta_limit);
    if (!delta) return 1;

    translate(prices, *delta);
    constexpr Price first = "125.5025";
    static_assert(Price{"125.50"} + "0.0025" == first);
    constexpr Price second = "125.4925";
    constexpr Price third = "125.4825";
    constexpr std::array<Price, 3> expected{first, second, third};
    if (prices != expected) return 1;

    if (read_bounded("1000001", -price_limit, price_limit)) return 1;
    if (read_bounded("not a price", -price_limit, price_limit)) return 1;
    const auto overflow = fw::add(Price::max(), Price{"0.01"});
    if (overflow || overflow.error() != fw::ArithmeticError::overflow) return 1;

    std::puts("OK: checked input, bounded ordinary arithmetic");
}
