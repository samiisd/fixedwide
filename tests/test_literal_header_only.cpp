#include <fixedwide/literal.hpp>

namespace {
using Price = fixedwide::Fixed64<10>;
using Amount = fixedwide::Fixed128<12>;

struct Money {
private:
    struct LiteralConstructorTag {};

public:
    Amount value{};
    template<std::size_t N>
    consteval Money(const char (&text)[N], LiteralConstructorTag = {}) : value(text) {}
};

// Ordinary expressions in instantiated templates must remain immediate too:
// a constexpr local alone does not catch accidental runtime literal calls.
template<class Tag>
Price template_price() {
    return Price{"100"};
}

template<class Tag>
Money template_money() {
    return Money{"100"};
}

template<class Tag>
bool template_aggregate() {
    struct Quote {
        Price bid{}, ask{};
    };
    Quote quote{.bid = Price{"100"}, .ask = Price{"101"}};
    return quote.bid == "100" && quote.ask == "101";
}
} // namespace

// No link to fixedwide, no exceptions, and no constexpr on the local variable:
// the public API must still do all decimal conversion during compilation.
int main() {
    using F = fixedwide::Fixed256<0>;
    auto minimum =
        fixedwide::literal<F>("-57896044618658097711785492504343953926634992332820282019728792003956564819968");
    constexpr auto cents = fixedwide::literal<fixedwide::Fixed64<2>>("19.99");
    static_assert(cents.raw() == 1999);
    F constructed = "-57896044618658097711785492504343953926634992332820282019728792003956564819968";
    constexpr fixedwide::Fixed64<2> amount = "19.99";
    static_assert(amount == cents);
    // Typed initialization remains immediate in generic-lambda instantiations.
    auto accumulate = [](auto& total) {
        fixedwide::Fixed64<2> value = "1.25";
        total += value.raw();
    };
    int first = 0;
    long second = 0;
    accumulate(first);
    accumulate(second);
    auto price = template_price<int>();
    auto money = template_money<int>();
    auto generic_price = [](auto) { return Price{"100"}; };
    return minimum == F::min() && constructed == minimum && first == 125 && second == 125 &&
                   price.raw() == Price::scale() * 100 && money.value.raw() == Amount::scale() * 100 &&
                   template_aggregate<int>() && generic_price(0) == price
               ? 0
               : 1;
}
