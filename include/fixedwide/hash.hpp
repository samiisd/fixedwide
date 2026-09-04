#pragma once

/// \file
/// `std::hash` for `basic_fixed` and the wide integers, so they work as keys in
/// the unordered containers. Two values that compare equal hash equal.

#include <fixedwide/fixed.hpp>
#include <functional>

template<std::size_t Bits, unsigned D>
struct std::hash<fixedwide::basic_fixed<Bits, D>> {
    std::size_t operator()(const fixedwide::basic_fixed<Bits, D>& val) const noexcept {
        auto raw = val.raw();
        if constexpr (Bits <= 64) {
            return std::hash<typename fixedwide::basic_fixed<Bits, D>::raw_type>{}(raw);
        } else if constexpr (Bits == 128) {
            std::size_t h1 = std::hash<std::uint64_t>{}(raw.low);
            std::size_t h2 = std::hash<std::uint64_t>{}(raw.high);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        } else {
            std::size_t seed = 0;
            for (int i = 0; i < 4; ++i) {
                seed ^= std::hash<std::uint64_t>{}(raw.limbs[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    }
};

template<>
struct std::hash<fixedwide::wide::uint128> {
    std::size_t operator()(const fixedwide::wide::uint128& val) const noexcept {
        std::size_t h1 = std::hash<std::uint64_t>{}(val.low);
        std::size_t h2 = std::hash<std::uint64_t>{}(val.high);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

template<>
struct std::hash<fixedwide::wide::int128> {
    std::size_t operator()(const fixedwide::wide::int128& val) const noexcept {
        return std::hash<fixedwide::wide::uint128>{}(fixedwide::wide::uint128(val.low, val.high));
    }
};

template<>
struct std::hash<fixedwide::wide::uint256> {
    std::size_t operator()(const fixedwide::wide::uint256& val) const noexcept {
        std::size_t seed = 0;
        for (int i = 0; i < 4; ++i) {
            seed ^= std::hash<std::uint64_t>{}(val.limbs[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template<>
struct std::hash<fixedwide::wide::int256> {
    std::size_t operator()(const fixedwide::wide::int256& val) const noexcept {
        std::size_t seed = 0;
        for (int i = 0; i < 4; ++i) {
            seed ^= std::hash<std::uint64_t>{}(val.limbs[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

namespace fixedwide {
/// Free-function spelling of `std::hash`, for generic code and for Boost-style
/// containers that look for `hash_value` by argument-dependent lookup.
template<std::size_t Bits, unsigned D>
[[nodiscard]] inline std::size_t hash_value(basic_fixed<Bits, D> v) noexcept {
    return std::hash<basic_fixed<Bits, D>>{}(v);
}
} // namespace fixedwide
