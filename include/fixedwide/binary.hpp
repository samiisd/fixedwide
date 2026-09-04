#pragma once

/// \file
/// Explicit-endian, zero-copy binary encoding. The byte order is named in the
/// call, never taken from the host, so a record written on one machine reads
/// the same on another.

#include <fixedwide/fixed.hpp>
#include <fixedwide/error.hpp>
#include <array>
#include <span>
#include <expected>
#include <cstring>

namespace fixedwide {

/// Byte order for the binary encodings below. Named rather than taken from the
/// host, so a wire format stays the same wherever it is built.
enum class endian {
    /// Least significant byte first.
    little,
    /// Most significant byte first.
    big,
};

namespace detail {

template<endian Endian, std::size_t Bits>
void encode_limbs(std::uint8_t* out, const std::uint64_t* limbs, std::size_t num_limbs) noexcept {
    constexpr std::size_t total_bytes = Bits / 8;
    if constexpr (Bits < 64) {
        std::uint64_t val = limbs[0];
        if constexpr (Endian == endian::little) {
            for (std::size_t i = 0; i < total_bytes; ++i) {
                out[i] = static_cast<std::uint8_t>((val >> (8 * i)) & 0xFF);
            }
        } else {
            for (std::size_t i = 0; i < total_bytes; ++i) {
                out[i] = static_cast<std::uint8_t>((val >> (8 * (total_bytes - 1 - i))) & 0xFF);
            }
        }
        return;
    }

    if constexpr (Endian == endian::little) {
        std::size_t byte_idx = 0;
        for (std::size_t l = 0; l < num_limbs; ++l) {
            std::uint64_t v = limbs[l];
            for (std::size_t b = 0; b < 8; ++b) {
                out[byte_idx++] = static_cast<std::uint8_t>((v >> (8 * b)) & 0xFF);
            }
        }
    } else {
        std::size_t byte_idx = 0;
        for (std::size_t l = num_limbs; l > 0; --l) {
            std::uint64_t v = limbs[l - 1];
            for (std::size_t b = 8; b > 0; --b) {
                out[byte_idx++] = static_cast<std::uint8_t>((v >> (8 * (b - 1))) & 0xFF);
            }
        }
    }
}

template<endian Endian, std::size_t Bits>
void decode_limbs(std::uint64_t* limbs, const std::uint8_t* in, std::size_t num_limbs) noexcept {
    constexpr std::size_t total_bytes = Bits / 8;
    if constexpr (Bits < 64) {
        std::uint64_t val = 0;
        if constexpr (Endian == endian::little) {
            for (std::size_t i = 0; i < total_bytes; ++i) {
                val |= (static_cast<std::uint64_t>(in[i]) << (8 * i));
            }
        } else {
            for (std::size_t i = 0; i < total_bytes; ++i) {
                val = (val << 8) | in[i];
            }
        }
        limbs[0] = val;
        return;
    }

    if constexpr (Endian == endian::little) {
        std::size_t byte_idx = 0;
        for (std::size_t l = 0; l < num_limbs; ++l) {
            std::uint64_t v = 0;
            for (std::size_t b = 0; b < 8; ++b) {
                v |= (static_cast<std::uint64_t>(in[byte_idx++]) << (8 * b));
            }
            limbs[l] = v;
        }
    } else {
        std::size_t byte_idx = 0;
        for (std::size_t l = num_limbs; l > 0; --l) {
            std::uint64_t v = 0;
            for (std::size_t b = 0; b < 8; ++b) {
                v = (v << 8) | in[byte_idx++];
            }
            limbs[l - 1] = v;
        }
    }
}

} // namespace detail

/// Encode the raw scaled integer as exactly `Bits / 8` bytes, two's complement,
/// in the named byte order.
///
/// The scale is not encoded: it is in the type. Both ends must agree on it, the
/// same way they agree on the width.
///
/// \tparam Endian byte order; little by default.
template<endian Endian = endian::little, std::size_t Bits, unsigned D>
[[nodiscard]] inline std::array<std::uint8_t, Bits / 8> to_bytes(basic_fixed<Bits, D> value) noexcept {
    constexpr std::size_t N = Bits / 8;
    std::array<std::uint8_t, N> res;
    auto r = value.raw();
    if constexpr (Bits <= 64) {
        std::uint64_t l = static_cast<std::uint64_t>(r);
        detail::encode_limbs<Endian, Bits>(res.data(), &l, 1);
    } else if constexpr (Bits == 128) {
        std::uint64_t limbs[2] = {r.low, r.high};
        detail::encode_limbs<Endian, 128>(res.data(), limbs, 2);
    } else {
        detail::encode_limbs<Endian, 256>(res.data(), r.limbs, 4);
    }
    return res;
}

/// Decode bytes written by `to_bytes`.
///
/// \tparam Target the fixed-point type the bytes encode.
/// \tparam Endian the byte order they were written in.
/// \return the value, or `BinaryError::wrong_size` when the span is not
///         exactly `Target::bits / 8` bytes.
template<typename Target, endian Endian = endian::little>
[[nodiscard]] inline std::expected<Target, BinaryError> from_bytes(std::span<const std::uint8_t> bytes) noexcept {
    constexpr std::size_t N = Target::bits / 8;
    if (bytes.size() != N) return std::unexpected(BinaryError::wrong_size);
    if constexpr (Target::bits <= 64) {
        std::uint64_t l = 0;
        detail::decode_limbs<Endian, Target::bits>(&l, bytes.data(), 1);
        return Target::from_raw(static_cast<typename Target::raw_type>(l));
    } else if constexpr (Target::bits == 128) {
        std::uint64_t limbs[2]{};
        detail::decode_limbs<Endian, 128>(limbs, bytes.data(), 2);
        return Target::from_raw(wide::int128(limbs[0], limbs[1]));
    } else {
        std::uint64_t limbs[4]{};
        detail::decode_limbs<Endian, 256>(limbs, bytes.data(), 4);
        return Target::from_raw(wide::int256(limbs[0], limbs[1], limbs[2], limbs[3]));
    }
}

// Unaligned load/store
/// Read a value straight out of a packet or a memory-mapped record at any
/// alignment. Unchecked: the caller guarantees `Target::bits / 8` readable
/// bytes at `ptr`. Use `from_bytes` when the size should be validated.
template<typename Target, endian Endian = endian::little>
[[nodiscard]] inline Target load_unaligned(const void* ptr) noexcept {
    constexpr std::size_t N = Target::bits / 8;
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(ptr);
    if constexpr (Target::bits <= 64) {
        std::uint64_t l = 0;
        detail::decode_limbs<Endian, Target::bits>(&l, bytes, 1);
        return Target::from_raw(static_cast<typename Target::raw_type>(l));
    } else if constexpr (Target::bits == 128) {
        std::uint64_t limbs[2]{};
        detail::decode_limbs<Endian, 128>(limbs, bytes, 2);
        return Target::from_raw(wide::int128(limbs[0], limbs[1]));
    } else {
        std::uint64_t limbs[4]{};
        detail::decode_limbs<Endian, 256>(limbs, bytes, 4);
        return Target::from_raw(wide::int256(limbs[0], limbs[1], limbs[2], limbs[3]));
    }
}

/// Write a value straight into a packet or record at any alignment. Unchecked:
/// the caller guarantees enough writable bytes at `ptr`.
template<endian Endian = endian::little, typename T>
inline void store_unaligned(void* ptr, T value) noexcept {
    auto b = to_bytes<Endian>(value);
    std::memcpy(ptr, b.data(), b.size());
}

// Wide integers support
/// Encode a wide integer as bytes, two's complement, in the named byte order.
template<endian Endian = endian::little>
[[nodiscard]] inline std::array<std::uint8_t, 16> to_bytes(wide::uint128 value) noexcept {
    std::array<std::uint8_t, 16> res;
    std::uint64_t limbs[2] = {value.low, value.high};
    detail::encode_limbs<Endian, 128>(res.data(), limbs, 2);
    return res;
}

template<endian Endian = endian::little>
[[nodiscard]] inline std::array<std::uint8_t, 16> to_bytes(wide::int128 value) noexcept {
    std::array<std::uint8_t, 16> res;
    std::uint64_t limbs[2] = {value.low, value.high};
    detail::encode_limbs<Endian, 128>(res.data(), limbs, 2);
    return res;
}

template<endian Endian = endian::little>
[[nodiscard]] inline std::array<std::uint8_t, 32> to_bytes(wide::uint256 value) noexcept {
    std::array<std::uint8_t, 32> res;
    detail::encode_limbs<Endian, 256>(res.data(), value.limbs, 4);
    return res;
}

template<endian Endian = endian::little>
[[nodiscard]] inline std::array<std::uint8_t, 32> to_bytes(wide::int256 value) noexcept {
    std::array<std::uint8_t, 32> res;
    detail::encode_limbs<Endian, 256>(res.data(), value.limbs, 4);
    return res;
}

} // namespace fixedwide
