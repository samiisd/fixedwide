#define FIXEDWIDE_FORCE_PORTABLE 1
#include "detail.hpp"
#include <iostream>
int main() {
  using fixedwide::wide::uint128;
  using fixedwide::detail::divide128;
  const uint128 d(0ULL, 0x8000000000000000ULL);
  const uint128 n(~0ULL, 0x8000000000000000ULL);
  for (int i=0;i<10;++i) {
    auto qr=divide128(n,d,true);
    std::cout << qr.quotient.high << ':' << qr.quotient.low << ' '
              << qr.remainder.high << ':' << qr.remainder.low << '\n';
  }
}
