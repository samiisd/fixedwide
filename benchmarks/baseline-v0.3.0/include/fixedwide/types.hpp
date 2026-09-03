#pragma once
#include <cstdint>

#if !defined(__x86_64__) || !defined(__SIZEOF_INT128__)
#error "fixedwide requires an x86-64 compiler with __int128 support"
#endif

namespace fixedwide {
// Compiler integers, not arbitrary-precision objects. No allocations or wrappers.
using i128 = __int128;
using u128 = unsigned __int128;
inline constexpr u128 u128_max = ~u128{0};
inline constexpr i128 i128_max = static_cast<i128>(u128_max >> 1);
inline constexpr i128 i128_min = -i128_max - 1;

// Unlike signed abs(), magnitude() is defined even for the signed minimum.
[[nodiscard]] constexpr u128 magnitude(i128 value) noexcept {
    return value < 0 ? u128{0} - static_cast<u128>(value) : static_cast<u128>(value);
}
[[nodiscard]] constexpr int bit_width(u128 value) noexcept {
    const auto high = static_cast<std::uint64_t>(value >> 64);
    if (high != 0) return 128 - __builtin_clzll(high);
    const auto low = static_cast<std::uint64_t>(value);
    return low == 0 ? 0 : 64 - __builtin_clzll(low);
}
} // namespace fixedwide
