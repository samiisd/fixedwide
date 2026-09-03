#pragma once
#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <array>
#include <span>
#include <expected>
#include <cstring>
#include <bit>

namespace fixedwide {

enum class endian {
    little,
    big,
};

namespace detail {

template<endian Endian, std::size_t N>
void encode_bytes(std::uint8_t* out, const std::uint8_t* in) noexcept {
    if constexpr ((Endian == endian::little && std::endian::native == std::endian::little) ||
                  (Endian == endian::big && std::endian::native == std::endian::big)) {
        std::memcpy(out, in, N);
    } else {
        for (std::size_t i = 0; i < N; ++i) {
            out[i] = in[N - 1 - i];
        }
    }
}

template<endian Endian, std::size_t N>
void decode_bytes(std::uint8_t* out, const std::uint8_t* in) noexcept {
    encode_bytes<Endian, N>(out, in);
}

} // namespace detail

template<endian Endian = endian::little, std::size_t Bits, unsigned D>
[[nodiscard]] inline std::array<std::uint8_t, Bits / 8>
to_bytes(basic_fixed<Bits, D> value) noexcept {
    constexpr std::size_t N = Bits / 8;
    std::array<std::uint8_t, N> res;
    auto r = value.raw();
    detail::encode_bytes<Endian, N>(res.data(), reinterpret_cast<const std::uint8_t*>(&r));
    return res;
}

template<typename Target, endian Endian = endian::little>
[[nodiscard]] inline std::expected<Target, BinaryError>
from_bytes(std::span<const std::uint8_t> bytes) noexcept {
    constexpr std::size_t N = Target::bits / 8;
    if (bytes.size() != N) return std::unexpected(BinaryError::wrong_size);
    typename Target::raw_type raw;
    detail::decode_bytes<Endian, N>(reinterpret_cast<std::uint8_t*>(&raw), bytes.data());
    return Target::from_raw(raw);
}

// Unaligned load/store
template<typename Target, endian Endian = endian::little>
[[nodiscard]] inline Target load_unaligned(const void* ptr) noexcept {
    constexpr std::size_t N = Target::bits / 8;
    typename Target::raw_type raw;
    detail::decode_bytes<Endian, N>(reinterpret_cast<std::uint8_t*>(&raw),
                                    reinterpret_cast<const std::uint8_t*>(ptr));
    return Target::from_raw(raw);
}

template<endian Endian = endian::little, typename T>
inline void store_unaligned(void* ptr, T value) noexcept {
    constexpr std::size_t N = T::bits / 8;
    auto raw = value.raw();
    detail::encode_bytes<Endian, N>(reinterpret_cast<std::uint8_t*>(ptr),
                                    reinterpret_cast<const std::uint8_t*>(&raw));
}

// Wide integers support
template<endian Endian = endian::little>
[[nodiscard]] inline std::array<std::uint8_t, 16> to_bytes(wide::uint128 value) noexcept {
    std::array<std::uint8_t, 16> res;
    detail::encode_bytes<Endian, 16>(res.data(), reinterpret_cast<const std::uint8_t*>(&value));
    return res;
}

template<endian Endian = endian::little>
[[nodiscard]] inline std::array<std::uint8_t, 16> to_bytes(wide::int128 value) noexcept {
    std::array<std::uint8_t, 16> res;
    detail::encode_bytes<Endian, 16>(res.data(), reinterpret_cast<const std::uint8_t*>(&value));
    return res;
}

template<endian Endian = endian::little>
[[nodiscard]] inline std::array<std::uint8_t, 32> to_bytes(wide::uint256 value) noexcept {
    std::array<std::uint8_t, 32> res;
    detail::encode_bytes<Endian, 32>(res.data(), reinterpret_cast<const std::uint8_t*>(&value));
    return res;
}

template<endian Endian = endian::little>
[[nodiscard]] inline std::array<std::uint8_t, 32> to_bytes(wide::int256 value) noexcept {
    std::array<std::uint8_t, 32> res;
    detail::encode_bytes<Endian, 32>(res.data(), reinterpret_cast<const std::uint8_t*>(&value));
    return res;
}

} // namespace fixedwide
