#include <fixedwide/fixed.hpp>

int main() {
    using namespace fixedwide;
    Fixed64<4> a;
    // Expected to fail: no runtime scale member
    auto s = a.scale_member;
    return 0;
}
