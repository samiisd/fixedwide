#include <fixedwide/literal.hpp>

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
    return minimum == F::min() && constructed == minimum && first == 125 && second == 125 ? 0 : 1;
}
