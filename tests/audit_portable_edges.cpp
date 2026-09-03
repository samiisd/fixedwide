#define FIXEDWIDE_FORCE_PORTABLE 1
#include "detail.hpp"
#include <cstdint>
#include <limits>
#include <iostream>

int main() {
    using namespace fixedwide::detail;
    std::int64_t hi = 0;
    std::uint64_t lo = 0;
    imul64x64(std::numeric_limits<std::int64_t>::min(), 1, hi, lo);
    std::cout << "imul hi=" << hi << " lo=" << lo << '\n';

    auto q = div_signed64(-1, 0, std::numeric_limits<std::int64_t>::min());
    std::cout << "div q=" << q.quotient << " r=" << q.remainder << '\n';

    fixedwide::wide::uint128 n(~0ULL, 0x8000000000000000ULL);
    fixedwide::wide::uint128 d(0ULL, 0x8000000000000000ULL);
    auto qr = divide128(n, d, true);
    std::cout << "divide128 q=" << qr.quotient.low << ":" << qr.quotient.high
              << " r=" << qr.remainder.low << ":" << qr.remainder.high << '\n';
    return 0;
}
