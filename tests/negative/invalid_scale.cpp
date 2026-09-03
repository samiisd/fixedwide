#include <fixedwide/fixed.hpp>

int main() {
    using namespace fixedwide;
    // Expected to fail: Fixed8 can only hold up to 2 decimal places
    Fixed8<3> bad;
    return 0;
}
