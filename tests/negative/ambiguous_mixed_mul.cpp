#include <fixedwide/fixed.hpp>
#include <fixedwide/arithmetic.hpp>

int main() {
    using namespace fixedwide;
    Fixed64<8> a;
    Fixed64<12> b;
    // Expected to fail at compile time: ambiguous mixed same-name arithmetic
    auto res = mul(a, b);
    return 0;
}
