#pragma once

#include <fixedwide/wide.hpp>
#include <array>
#include <bit>
#include <cstring>
#include <limits>

// Test-only encoder, independent of the production decoder. Callers supply the
// complete significand and exponent fields; the oracle uses those integers,
// never a value converted through double or the production decode function.
namespace floating_test {
using fixedwide::wide::uint128;

template<typename Float>
inline constexpr int precision = std::numeric_limits<Float>::digits;

template<typename Float>
inline constexpr bool supported =
    std::numeric_limits<Float>::is_iec559 && std::numeric_limits<Float>::radix == 2 &&
    ((precision<Float> == 24 && sizeof(Float) == 4) || (precision<Float> == 53 && sizeof(Float) == 8) ||
     (precision<Float> == 113 && sizeof(Float) == 16 &&
      (std::endian::native == std::endian::little || std::endian::native == std::endian::big))
#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
     || (precision<Float> == 64 && sizeof(Float) >= 10 && std::endian::native == std::endian::little)
#endif
    );

template<typename Float>
void store(Float& value, bool negative, unsigned exponent, uint128 significand, unsigned char padding = 0) {
    static_assert(supported<Float>);
    std::array<unsigned char, sizeof(Float)> bytes{};
    bytes.fill(padding);
    if constexpr (precision<Float> == 24) {
        const auto raw = (std::uint32_t{negative} << 31) | (exponent << 23) |
                         (static_cast<std::uint32_t>(significand.low) & 0x007f'ffffu);
        std::memcpy(bytes.data(), &raw, sizeof(raw));
    } else if constexpr (precision<Float> == 53) {
        const auto raw = (std::uint64_t{negative} << 63) | (std::uint64_t{exponent} << 52) |
                         (significand.low & 0x000f'ffff'ffff'ffffULL);
        std::memcpy(bytes.data(), &raw, sizeof(raw));
    } else if constexpr (precision<Float> == 64) {
        const auto sign_exponent = static_cast<std::uint16_t>((negative ? 0x8000u : 0) | exponent);
        std::memcpy(bytes.data(), &significand.low, 8);
        std::memcpy(bytes.data() + 8, &sign_exponent, 2);
    } else {
        const std::uint64_t high = (std::uint64_t{negative} << 63) | (std::uint64_t{exponent} << 48) |
                                   (significand.high & 0x0000'ffff'ffff'ffffULL);
        constexpr unsigned low_offset = std::endian::native == std::endian::little ? 0 : 8;
        std::memcpy(bytes.data() + low_offset, &significand.low, 8);
        std::memcpy(bytes.data() + (8 - low_offset), &high, 8);
    }
    std::memcpy(&value, bytes.data(), sizeof(value));
}
} // namespace floating_test
