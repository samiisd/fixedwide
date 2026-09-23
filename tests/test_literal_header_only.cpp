#include <fixedwide/literal.hpp>

// No link to fixedwide, no exceptions, and no constexpr on the local variable:
// the public API must still do all decimal conversion during compilation.
int main() {
    using F = fixedwide::Fixed256<0>;
    auto minimum = fixedwide::literal<F>("-57896044618658097711785492504343953926634992332820282019728792003956564819968");
    constexpr auto cents = fixedwide::literal<fixedwide::Fixed64<2>>("19.99");
    static_assert(cents.raw() == 1999);
    return minimum == F::min() ? 0 : 1;
}
