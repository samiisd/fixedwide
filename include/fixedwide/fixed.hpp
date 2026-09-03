#pragma once
#include <fixedwide/wide.hpp>
#include <cstdint>
#include <cstddef>
#include <compare>
#include <type_traits>

#if (defined(__x86_64__) || defined(_M_X64)) && (defined(__GNUC__) || defined(__clang__)) && !defined(FIXEDWIDE_FORCE_PORTABLE)
#  define FIXEDWIDE_HAS_X86_64_ASM 1
#endif

namespace fixedwide {

template<typename T>
constexpr T compute_pow10(unsigned exp) noexcept {
    T res(1);
    for (unsigned i = 0; i < exp; ++i) {
        res = (res << 3) + (res << 1);
    }
    return res;
}

namespace detail {

template<std::size_t Bits>
[[nodiscard]] constexpr wide::uint256 limit_magnitude_u256(bool negative) noexcept {
    static_assert(Bits == 8 || Bits == 16 || Bits == 32 || Bits == 64 || Bits == 128 || Bits == 256,
                  "Invalid bit width for limit_magnitude_u256");
    wide::uint256 lim{};
    if (negative) {
        if constexpr (Bits == 8) {
            lim.limbs[0] = 0x80ULL;
        } else if constexpr (Bits == 16) {
            lim.limbs[0] = 0x8000ULL;
        } else if constexpr (Bits == 32) {
            lim.limbs[0] = 0x8000'0000ULL;
        } else if constexpr (Bits == 64) {
            lim.limbs[0] = 0x8000'0000'0000'0000ULL;
        } else if constexpr (Bits == 128) {
            lim.limbs[0] = 0ULL;
            lim.limbs[1] = 0x8000'0000'0000'0000ULL;
        } else if constexpr (Bits == 256) {
            lim.limbs[0] = 0ULL;
            lim.limbs[1] = 0ULL;
            lim.limbs[2] = 0ULL;
            lim.limbs[3] = 0x8000'0000'0000'0000ULL;
        }
    } else {
        if constexpr (Bits == 8) {
            lim.limbs[0] = 0x7FULL;
        } else if constexpr (Bits == 16) {
            lim.limbs[0] = 0x7FFFULL;
        } else if constexpr (Bits == 32) {
            lim.limbs[0] = 0x7FFF'FFFFULL;
        } else if constexpr (Bits == 64) {
            lim.limbs[0] = 0x7FFF'FFFF'FFFF'FFFFULL;
        } else if constexpr (Bits == 128) {
            lim.limbs[0] = ~0ULL;
            lim.limbs[1] = 0x7FFF'FFFF'FFFF'FFFFULL;
        } else if constexpr (Bits == 256) {
            lim.limbs[0] = ~0ULL;
            lim.limbs[1] = ~0ULL;
            lim.limbs[2] = ~0ULL;
            lim.limbs[3] = 0x7FFF'FFFF'FFFF'FFFFULL;
        }
    }
    return lim;
}

[[nodiscard]] constexpr wide::uint256 limit_for_bits(std::size_t bits, bool negative) noexcept {
    switch (bits) {
    case 8: return limit_magnitude_u256<8>(negative);
    case 16: return limit_magnitude_u256<16>(negative);
    case 32: return limit_magnitude_u256<32>(negative);
    case 64: return limit_magnitude_u256<64>(negative);
    case 128: return limit_magnitude_u256<128>(negative);
    case 256: return limit_magnitude_u256<256>(negative);
    default: return wide::uint256{};
    }
}

} // namespace detail

template<std::size_t Bits>
struct raw_type_helper;

template<> struct raw_type_helper<8>   { using type = std::int8_t; };
template<> struct raw_type_helper<16>  { using type = std::int16_t; };
template<> struct raw_type_helper<32>  { using type = std::int32_t; };
template<> struct raw_type_helper<64>  { using type = std::int64_t; };
template<> struct raw_type_helper<128> { using type = wide::int128; };
template<> struct raw_type_helper<256> { using type = wide::int256; };

template<std::size_t Bits>
using raw_type_t = typename raw_type_helper<Bits>::type;

template<std::size_t Bits>
consteval unsigned max_decimals_for_bits() noexcept {
    if constexpr (Bits == 8)   return 2;
    if constexpr (Bits == 16)  return 4;
    if constexpr (Bits == 32)  return 9;
    if constexpr (Bits == 64)  return 18;
    if constexpr (Bits == 128) return 38;
    if constexpr (Bits == 256) return 76;
    return 0;
}

template<std::size_t Bits, unsigned Decimals>
struct basic_fixed {
    static_assert(Bits == 8 || Bits == 16 || Bits == 32 || Bits == 64 || Bits == 128 || Bits == 256,
                  "Bits must be one of: 8, 16, 32, 64, 128, 256");
    static_assert(Decimals <= max_decimals_for_bits<Bits>(),
                  "Decimals exceeds maximum capacity for given bit width");

    using raw_type = raw_type_t<Bits>;
    static constexpr std::size_t bits = Bits;
    static constexpr unsigned fractional_digits = Decimals;

    static constexpr raw_type scale() noexcept {
        return compute_pow10<raw_type>(Decimals);
    }

    constexpr basic_fixed() noexcept : m_raw(0) {}

    template<std::size_t OtherBits>
        requires (OtherBits < Bits)
    explicit constexpr basic_fixed(basic_fixed<OtherBits, Decimals> other) noexcept
        : m_raw(0) {
        if constexpr (Bits <= 64) {
            m_raw = static_cast<raw_type>(other.raw());
        } else if constexpr (Bits == 128) {
            if constexpr (OtherBits <= 64) {
                m_raw = wide::int128(static_cast<std::int64_t>(other.raw()));
            }
        } else {
            if constexpr (OtherBits <= 64) {
                m_raw = wide::int256(static_cast<std::int64_t>(other.raw()));
            } else if constexpr (OtherBits == 128) {
                m_raw = wide::int256(other.raw());
            }
        }
    }

    [[nodiscard]] static constexpr basic_fixed from_raw(raw_type r) noexcept {
        basic_fixed f;
        f.m_raw = r;
        return f;
    }

    [[nodiscard]] constexpr raw_type raw() const noexcept {
        return m_raw;
    }

    [[nodiscard]] static constexpr basic_fixed min() noexcept {
        if constexpr (Bits == 8)   return from_raw(INT8_MIN);
        if constexpr (Bits == 16)  return from_raw(INT16_MIN);
        if constexpr (Bits == 32)  return from_raw(INT32_MIN);
        if constexpr (Bits == 64)  return from_raw(INT64_MIN);
        if constexpr (Bits == 128) return from_raw(wide::int128::min());
        if constexpr (Bits == 256) return from_raw(wide::int256::min());
    }

    [[nodiscard]] static constexpr basic_fixed max() noexcept {
        if constexpr (Bits == 8)   return from_raw(INT8_MAX);
        if constexpr (Bits == 16)  return from_raw(INT16_MAX);
        if constexpr (Bits == 32)  return from_raw(INT32_MAX);
        if constexpr (Bits == 64)  return from_raw(INT64_MAX);
        if constexpr (Bits == 128) return from_raw(wide::int128::max());
        if constexpr (Bits == 256) return from_raw(wide::int256::max());
    }

    constexpr bool operator==(const basic_fixed&) const noexcept = default;
    constexpr auto operator<=>(const basic_fixed&) const noexcept = default;

private:
    raw_type m_raw{0};
};

// Aliases
template<unsigned D> using Fixed8   = basic_fixed<8, D>;
template<unsigned D> using Fixed16  = basic_fixed<16, D>;
template<unsigned D> using Fixed32  = basic_fixed<32, D>;
template<unsigned D> using Fixed64  = basic_fixed<64, D>;
template<unsigned D> using Fixed128 = basic_fixed<128, D>;
template<unsigned D> using Fixed256 = basic_fixed<256, D>;

// Size and alignment invariants
static_assert(sizeof(Fixed8<2>) == 1 && alignof(Fixed8<2>) <= 8);
static_assert(sizeof(Fixed16<4>) == 2 && alignof(Fixed16<4>) <= 8);
static_assert(sizeof(Fixed32<9>) == 4 && alignof(Fixed32<9>) <= 8);
static_assert(sizeof(Fixed64<18>) == 8 && alignof(Fixed64<18>) <= 8);
static_assert(sizeof(Fixed128<38>) == 16 && alignof(Fixed128<38>) <= 8);
static_assert(sizeof(Fixed256<76>) == 32 && alignof(Fixed256<76>) <= 8);

namespace detail {

template<class T>
constexpr wide::int256 to_int256_raw(T val) noexcept {
    if constexpr (std::is_same_v<T, wide::int256>) {
        return val;
    } else if constexpr (std::is_same_v<T, wide::int128>) {
        return wide::int256(val);
    } else {
        return wide::int256(static_cast<std::int64_t>(val));
    }
}

template<class T>
constexpr wide::uint256 to_uint256_raw(T val) noexcept {
    if constexpr (std::is_same_v<T, wide::uint256>) {
        return val;
    } else if constexpr (std::is_same_v<T, wide::int256>) {
        return wide::uint256(val.limbs[0], val.limbs[1], val.limbs[2], val.limbs[3]);
    } else if constexpr (std::is_same_v<T, wide::uint128>) {
        return wide::uint256(val.low, val.high, 0, 0);
    } else if constexpr (std::is_same_v<T, wide::int128>) {
        return wide::uint256(val.low, val.high, 0, 0);
    } else {
        return wide::uint256(static_cast<std::uint64_t>(val));
    }
}

template<std::size_t Bits, unsigned Decimals>
consteval wide::uint256 max_integer_allowed(bool negative) noexcept {
    auto lim = limit_magnitude_u256<Bits>(negative);
    if constexpr (Decimals == 0) {
        return lim;
    } else {
        auto sc = to_uint256_raw(basic_fixed<Bits, Decimals>::scale());
        wide::uint256 q{};
        wide::uint256 rem{};
        for (int i = 255; i >= 0; --i) {
            rem = rem << 1;
            unsigned limb_idx = static_cast<unsigned>(i / 64);
            unsigned bit_idx = static_cast<unsigned>(i % 64);
            rem.limbs[0] |= (lim.limbs[limb_idx] >> bit_idx) & 1ULL;
            if (rem >= sc) {
                rem = rem - sc;
                q.limbs[limb_idx] |= (1ULL << bit_idx);
            }
        }
        return q;
    }
}

template<class Dest>
constexpr Dest from_int256_raw(wide::int256 val) noexcept {
    if constexpr (Dest::bits == 8) {
        return Dest::from_raw(static_cast<std::int8_t>(val.limbs[0]));
    } else if constexpr (Dest::bits == 16) {
        return Dest::from_raw(static_cast<std::int16_t>(val.limbs[0]));
    } else if constexpr (Dest::bits == 32) {
        return Dest::from_raw(static_cast<std::int32_t>(val.limbs[0]));
    } else if constexpr (Dest::bits == 64) {
        return Dest::from_raw(static_cast<std::int64_t>(val.limbs[0]));
    } else if constexpr (Dest::bits == 128) {
        return Dest::from_raw(wide::int128(val.limbs[0], val.limbs[1]));
    } else {
        return Dest::from_raw(val);
    }
}

} // namespace detail


// Backward compatibility aliases for 0.4
using FP64 = Fixed64<12>;
using FP128 = Fixed128<12>;
inline constexpr auto fp64_min = FP64::min();
inline constexpr auto fp64_max = FP64::max();
inline constexpr auto fp128_min = FP128::min();
inline constexpr auto fp128_max = FP128::max();

// Backward compatibility constants for 0.4
inline constexpr unsigned fractional_digits = 12;
inline constexpr std::int64_t scale = 1'000'000'000'000LL;

template<class T>
[[nodiscard]] constexpr T from_raw(typename T::raw_type raw) noexcept {
    return T::from_raw(raw);
}
} // namespace fixedwide
