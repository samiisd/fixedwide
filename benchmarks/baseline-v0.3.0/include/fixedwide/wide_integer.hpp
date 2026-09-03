#pragma once
#include <fixedwide/types.hpp>

#if !defined(__BITINT_MAXWIDTH__) || __BITINT_MAXWIDTH__ < 256
#error "fixedwide wide integers require C++ _BitInt(256) support (tested with Clang)"
#endif

namespace fixedwide {
using i256 = _BitInt(256);
using u256 = unsigned _BitInt(256);
inline constexpr u256 u256_max = ~u256{0};
inline constexpr i256 i256_max = static_cast<i256>(u256_max >> 1);
inline constexpr i256 i256_min = -i256_max - 1;
[[nodiscard]] constexpr u256 magnitude(i256 value) noexcept {
    return value < 0 ? u256{0} - static_cast<u256>(value) : static_cast<u256>(value);
}
[[nodiscard]] constexpr int bit_width(u256 value) noexcept {
    const u128 high = static_cast<u128>(value >> 128);
    return high == 0 ? bit_width(static_cast<u128>(value)) : 128 + bit_width(high);
}
} // namespace fixedwide
