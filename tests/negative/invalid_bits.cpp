#include <fixedwide/fixed.hpp>

int main() {
    using namespace fixedwide;
    // Expected to fail: 48 bits is not supported
    basic_fixed<48, 4> bad;
    return 0;
}
