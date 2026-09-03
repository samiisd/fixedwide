#pragma once
#include <cstdint>
#include <compare>
#include <concepts>
#include <bit>
#include <type_traits>

namespace fixedwide::wide {

// std::int64_t is `long` on some targets and `long long` on others (LP64 Linux
// versus AArch64 here), so spelling out both overloads is a redefinition on one
// and a missing conversion on the other. This constraint covers every 64-bit
// integer spelling exactly once.
template<typename T>
concept narrow_int64 = std::integral<T> && sizeof(T) <= 8 && !std::is_same_v<T, bool>;


struct alignas(8) uint128;
struct alignas(8) int128;
struct alignas(8) uint256;
struct alignas(8) int256;

// 128-bit unsigned integer composed of two 64-bit limbs.
struct alignas(8) uint128 {
    std::uint64_t low{0};
    std::uint64_t high{0};

    constexpr uint128() noexcept = default;
    constexpr uint128(std::uint64_t lo, std::uint64_t hi = 0) noexcept : low(lo), high(hi) {}

#if defined(__SIZEOF_INT128__)
    constexpr uint128(unsigned __int128 v) noexcept
        : low(static_cast<std::uint64_t>(v)), high(static_cast<std::uint64_t>(v >> 64)) {}
    constexpr uint128(__int128 v) noexcept
        : low(static_cast<std::uint64_t>(v)), high(static_cast<std::uint64_t>(static_cast<unsigned __int128>(v) >> 64)) {}
    constexpr explicit operator unsigned __int128() const noexcept {
        return (static_cast<unsigned __int128>(high) << 64) | low;
    }
    constexpr explicit operator __int128() const noexcept {
        return static_cast<__int128>((static_cast<unsigned __int128>(high) << 64) | low);
    }
#endif
#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 128
#endif
    template<std::integral T>
    constexpr uint128(T val) noexcept {
        if constexpr (std::is_signed_v<T>) {
            low = static_cast<std::uint64_t>(val);
            high = val < 0 ? ~0ULL : 0ULL;
        } else {
            low = static_cast<std::uint64_t>(val);
            high = 0ULL;
        }
    }

    [[nodiscard]] static constexpr uint128 min() noexcept { return {0ULL, 0ULL}; }
    [[nodiscard]] static constexpr uint128 max() noexcept { return {~0ULL, ~0ULL}; }

    template<narrow_int64 T>
    constexpr explicit operator T() const noexcept { return static_cast<T>(low); }
    constexpr explicit operator int128() const noexcept;
    [[nodiscard]] constexpr bool is_zero() const noexcept { return low == 0 && high == 0; }

    constexpr bool operator==(const uint128&) const noexcept = default;
    constexpr std::strong_ordering operator<=>(const uint128& o) const noexcept {
        if (auto cmp = high <=> o.high; cmp != 0) return cmp;
        return low <=> o.low;
    }

    constexpr uint128 operator~() const noexcept { return {~low, ~high}; }

    constexpr uint128 operator&(const uint128& o) const noexcept { return {low & o.low, high & o.high}; }
    constexpr uint128 operator|(const uint128& o) const noexcept { return {low | o.low, high | o.high}; }
    constexpr uint128 operator^(const uint128& o) const noexcept { return {low ^ o.low, high ^ o.high}; }

    constexpr uint128& operator&=(const uint128& o) noexcept { low &= o.low; high &= o.high; return *this; }
    constexpr uint128& operator|=(const uint128& o) noexcept { low |= o.low; high |= o.high; return *this; }
    constexpr uint128& operator^=(const uint128& o) noexcept { low ^= o.low; high ^= o.high; return *this; }

    constexpr uint128 operator<<(unsigned shift) const noexcept {
        if (shift >= 128) return {0ULL, 0ULL};
        if (shift == 0) return *this;
        if (shift >= 64) return {0ULL, low << (shift - 64)};
        return {low << shift, (high << shift) | (low >> (64 - shift))};
    }

    constexpr uint128 operator>>(unsigned shift) const noexcept {
        if (shift >= 128) return {0ULL, 0ULL};
        if (shift == 0) return *this;
        if (shift >= 64) return {high >> (shift - 64), 0ULL};
        return {(low >> shift) | (high << (64 - shift)), high >> shift};
    }

    constexpr uint128& operator<<=(unsigned shift) noexcept { *this = *this << shift; return *this; }
    constexpr uint128& operator>>=(unsigned shift) noexcept { *this = *this >> shift; return *this; }

    constexpr uint128 operator+(const uint128& o) const noexcept {
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
        return uint128(static_cast<unsigned __int128>(*this) + static_cast<unsigned __int128>(o));
#else
        std::uint64_t lo = low + o.low;
        std::uint64_t carry = (lo < low) ? 1 : 0;
        std::uint64_t hi = high + o.high + carry;
        return {lo, hi};
#endif
    }

    constexpr uint128 operator*(const uint128& o) const noexcept {
#if defined(__SIZEOF_INT128__)
        return uint128(static_cast<unsigned __int128>(*this) * static_cast<unsigned __int128>(o));
#else
        uint64_t u0 = low & 0xFFFF'FFFFULL;
        uint64_t u1 = low >> 32;
        uint64_t v0 = o.low & 0xFFFF'FFFFULL;
        uint64_t v1 = o.low >> 32;
        uint64_t w0 = u0 * v0;
        uint64_t t = u1 * v0 + (w0 >> 32);
        uint64_t w1 = t & 0xFFFF'FFFFULL;
        uint64_t w2 = t >> 32;
        w1 += u0 * v1;
        w2 += (w1 >> 32);
        w1 &= 0xFFFF'FFFFULL;
        uint64_t hi = u1 * v1 + w2 + low * o.high + high * o.low;
        uint64_t lo = (w1 << 32) | (w0 & 0xFFFF'FFFFULL);
        return uint128(lo, hi);
#endif
    }
    constexpr uint128 operator-(const uint128& o) const noexcept {
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
        return uint128(static_cast<unsigned __int128>(*this) - static_cast<unsigned __int128>(o));
#else
        std::uint64_t lo = low - o.low;
        std::uint64_t borrow = (low < o.low) ? 1 : 0;
        std::uint64_t hi = high - o.high - borrow;
        return {lo, hi};
#endif
    }

    constexpr uint128& operator+=(const uint128& o) noexcept { *this = *this + o; return *this; }
    constexpr uint128& operator-=(const uint128& o) noexcept { *this = *this - o; return *this; }
};

// 128-bit signed integer in two's complement.
struct alignas(8) int128 {
    std::uint64_t low{0};
    std::uint64_t high{0};

    constexpr int128() noexcept = default;
    constexpr int128(std::uint64_t lo, std::uint64_t hi) noexcept : low(lo), high(hi) {}
#if defined(__SIZEOF_INT128__)
    constexpr int128(__int128 val) noexcept
        : low(static_cast<std::uint64_t>(val)),
          high(static_cast<std::uint64_t>(static_cast<unsigned __int128>(val) >> 64)) {}
    constexpr int128(unsigned __int128 val) noexcept
        : low(static_cast<std::uint64_t>(val)),
          high(static_cast<std::uint64_t>(val >> 64)) {}
    constexpr explicit operator __int128() const noexcept {
        return static_cast<__int128>((static_cast<unsigned __int128>(high) << 64) | low);
    }
    constexpr explicit operator unsigned __int128() const noexcept {
        return (static_cast<unsigned __int128>(high) << 64) | low;
    }
#endif
#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 128
#endif
    template<std::integral T>
    constexpr int128(T val) noexcept {
        if constexpr (std::is_signed_v<T>) {
            low = static_cast<std::uint64_t>(val);
            high = val < 0 ? ~0ULL : 0ULL;
        } else {
            low = static_cast<std::uint64_t>(val);
            high = 0ULL;
        }
    }

    [[nodiscard]] static constexpr int128 min() noexcept { return {0ULL, 0x8000'0000'0000'0000ULL}; }
    [[nodiscard]] static constexpr int128 max() noexcept { return {~0ULL, 0x7FFF'FFFF'FFFF'FFFFULL}; }

    [[nodiscard]] constexpr bool is_negative() const noexcept {
        return static_cast<std::int64_t>(high) < 0;
    }
    template<narrow_int64 T>
    constexpr explicit operator T() const noexcept { return static_cast<T>(low); }
    constexpr explicit operator uint128() const noexcept { return uint128(low, high); }
    [[nodiscard]] constexpr bool is_zero() const noexcept { return low == 0 && high == 0; }

    constexpr bool operator==(const int128&) const noexcept = default;
    constexpr std::strong_ordering operator<=>(const int128& o) const noexcept {
        auto h1 = static_cast<std::int64_t>(high);
        auto h2 = static_cast<std::int64_t>(o.high);
        if (auto cmp = h1 <=> h2; cmp != 0) return cmp;
        return low <=> o.low;
    }

    constexpr int128 operator~() const noexcept { return {~low, ~high}; }
    constexpr int128 operator-() const noexcept {
        uint128 u = ~uint128(low, high) + uint128(1ULL, 0ULL);
        return {u.low, u.high};
    }

    constexpr int128 operator&(const int128& o) const noexcept { return {low & o.low, high & o.high}; }
    constexpr int128 operator|(const int128& o) const noexcept { return {low | o.low, high | o.high}; }
    constexpr int128 operator^(const int128& o) const noexcept { return {low ^ o.low, high ^ o.high}; }

    constexpr int128 operator+(const int128& o) const noexcept {
        uint128 res = uint128(low, high) + uint128(o.low, o.high);
        return {res.low, res.high};
    }
    constexpr int128 operator*(const int128& o) const noexcept {
        uint128 u = uint128(low, high) * uint128(o.low, o.high);
        return int128(u.low, u.high);
    }
    constexpr int128 operator-(const int128& o) const noexcept {
        uint128 res = uint128(low, high) - uint128(o.low, o.high);
        return {res.low, res.high};
    }
    constexpr int128& operator+=(const int128& o) noexcept { *this = *this + o; return *this; }
    constexpr int128& operator-=(const int128& o) noexcept { *this = *this - o; return *this; }

    constexpr int128 operator<<(unsigned shift) const noexcept {
        uint128 u = uint128(low, high) << shift;
        return {u.low, u.high};
    }
    constexpr int128 operator>>(unsigned shift) const noexcept {
        if (shift >= 128) return is_negative() ? int128(~0ULL, ~0ULL) : int128(0ULL, 0ULL);
        if (shift == 0) return *this;
        auto sign_ext = is_negative() ? ~0ULL : 0ULL;
        if (shift >= 64) {
            auto h = static_cast<std::int64_t>(high) >> (shift - 64);
            return {static_cast<std::uint64_t>(h), sign_ext};
        }
        return {(low >> shift) | (high << (64 - shift)),
                static_cast<std::uint64_t>(static_cast<std::int64_t>(high) >> shift)};
    }
};

// 256-bit unsigned integer composed of four 64-bit limbs (least-significant first).
struct alignas(8) uint256 {
    std::uint64_t limbs[4]{0, 0, 0, 0};

    constexpr uint256() noexcept = default;
    constexpr uint256(std::uint64_t l0, std::uint64_t l1, std::uint64_t l2, std::uint64_t l3) noexcept
        : limbs{l0, l1, l2, l3} {}
    constexpr uint256(uint128 v) noexcept : limbs{v.low, v.high, 0, 0} {}
#if defined(__SIZEOF_INT128__)
    constexpr uint256(unsigned __int128 v) noexcept
        : limbs{static_cast<std::uint64_t>(v), static_cast<std::uint64_t>(v >> 64), 0, 0} {}
    constexpr uint256(__int128 v) noexcept {
        unsigned __int128 uv = static_cast<unsigned __int128>(v);
        std::uint64_t sign = v < 0 ? ~0ULL : 0ULL;
        limbs[0] = static_cast<std::uint64_t>(uv);
        limbs[1] = static_cast<std::uint64_t>(uv >> 64);
        limbs[2] = sign;
        limbs[3] = sign;
    }
#endif
#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 256
#endif
    template<std::integral T>
    constexpr uint256(T val) noexcept {
        if constexpr (std::is_signed_v<T>) {
            std::uint64_t u = static_cast<std::uint64_t>(val);
            std::uint64_t sign = val < 0 ? ~0ULL : 0ULL;
            limbs[0] = u; limbs[1] = sign; limbs[2] = sign; limbs[3] = sign;
        } else {
            limbs[0] = static_cast<std::uint64_t>(val);
            limbs[1] = 0; limbs[2] = 0; limbs[3] = 0;
        }
    }

    [[nodiscard]] static constexpr uint256 min() noexcept { return {0ULL, 0ULL, 0ULL, 0ULL}; }
    [[nodiscard]] static constexpr uint256 max() noexcept { return {~0ULL, ~0ULL, ~0ULL, ~0ULL}; }

    [[nodiscard]] constexpr bool is_zero() const noexcept {
        return limbs[0] == 0 && limbs[1] == 0 && limbs[2] == 0 && limbs[3] == 0;
    }

    constexpr bool operator==(const uint256&) const noexcept = default;
    constexpr std::strong_ordering operator<=>(const uint256& o) const noexcept {
        for (int i = 3; i >= 0; --i) {
            if (auto cmp = limbs[i] <=> o.limbs[i]; cmp != 0) return cmp;
        }
        return std::strong_ordering::equal;
    }

    constexpr explicit operator uint128() const noexcept {
        return uint128(limbs[0], limbs[1]);
    }

    constexpr uint256 operator~() const noexcept {
        return {~limbs[0], ~limbs[1], ~limbs[2], ~limbs[3]};
    }

    constexpr uint256 operator&(const uint256& o) const noexcept {
        return {limbs[0] & o.limbs[0], limbs[1] & o.limbs[1], limbs[2] & o.limbs[2], limbs[3] & o.limbs[3]};
    }
    constexpr uint256 operator|(const uint256& o) const noexcept {
        return {limbs[0] | o.limbs[0], limbs[1] | o.limbs[1], limbs[2] | o.limbs[2], limbs[3] | o.limbs[3]};
    }
    constexpr uint256 operator^(const uint256& o) const noexcept {
        return {limbs[0] ^ o.limbs[0], limbs[1] ^ o.limbs[1], limbs[2] ^ o.limbs[2], limbs[3] ^ o.limbs[3]};
    }

    constexpr uint256 operator<<(unsigned shift) const noexcept {
        if (shift >= 256) return {0ULL, 0ULL, 0ULL, 0ULL};
        if (shift == 0) return *this;
        unsigned limb_shift = shift / 64;
        unsigned bit_shift = shift % 64;
        uint256 result{};
        for (unsigned i = limb_shift; i < 4; ++i) {
            result.limbs[i] |= limbs[i - limb_shift] << bit_shift;
            if (bit_shift != 0 && i + 1 < 4) {
                result.limbs[i + 1] |= limbs[i - limb_shift] >> (64 - bit_shift);
            }
        }
        return result;
    }

    constexpr uint256 operator>>(unsigned shift) const noexcept {
        if (shift >= 256) return {0ULL, 0ULL, 0ULL, 0ULL};
        if (shift == 0) return *this;
        unsigned limb_shift = shift / 64;
        unsigned bit_shift = shift % 64;
        uint256 result{};
        for (unsigned i = 0; i + limb_shift < 4; ++i) {
            result.limbs[i] |= limbs[i + limb_shift] >> bit_shift;
            if (bit_shift != 0 && i + limb_shift + 1 < 4) {
                result.limbs[i] |= limbs[i + limb_shift + 1] << (64 - bit_shift);
            }
        }
        return result;
    }

    constexpr uint256& operator+=(const uint256& o) noexcept {
        *this = *this + o;
        return *this;
    }
    constexpr uint256& operator-=(const uint256& o) noexcept {
        *this = *this - o;
        return *this;
    }
    constexpr uint256 operator+(const uint256& o) const noexcept {
        uint256 res{};
        std::uint64_t carry = 0;
        for (int i = 0; i < 4; ++i) {
            std::uint64_t a = limbs[i];
            std::uint64_t b = o.limbs[i];
            std::uint64_t s = a + b + carry;
            carry = (s < a) || (carry && s == a) ? 1 : 0;
            res.limbs[i] = s;
        }
        return res;
    }

    constexpr uint256 operator*(const uint256& o) const noexcept {
        uint256 res{};
        for (int i = 0; i < 4; ++i) {
            uint64_t carry = 0;
            for (int j = 0; i + j < 4; ++j) {
                uint64_t u = limbs[i];
                uint64_t v = o.limbs[j];
                uint64_t u0 = u & 0xFFFF'FFFFULL;
                uint64_t u1 = u >> 32;
                uint64_t v0 = v & 0xFFFF'FFFFULL;
                uint64_t v1 = v >> 32;
                uint64_t w0 = u0 * v0;
                uint64_t t = u1 * v0 + (w0 >> 32);
                uint64_t w1 = t & 0xFFFF'FFFFULL;
                uint64_t w2 = t >> 32;
                w1 += u0 * v1;
                w2 += (w1 >> 32);
                w1 &= 0xFFFF'FFFFULL;
                uint64_t hi = u1 * v1 + w2;
                uint64_t lo = (w1 << 32) | (w0 & 0xFFFF'FFFFULL);

                uint64_t s1 = res.limbs[i + j] + lo;
                uint64_t c1 = (s1 < lo) ? 1 : 0;
                uint64_t s2 = s1 + carry;
                uint64_t c2 = (s2 < s1) ? 1 : 0;
                res.limbs[i + j] = s2;
                carry = hi + c1 + c2;
            }
        }
        return res;
    }
    constexpr uint256 operator-(const uint256& o) const noexcept {
        uint256 res{};
        std::uint64_t borrow = 0;
        for (int i = 0; i < 4; ++i) {
            std::uint64_t a = limbs[i];
            std::uint64_t b = o.limbs[i] + borrow;
            borrow = (b < o.limbs[i]) || (a < b) ? 1 : 0;
            res.limbs[i] = a - b;
        }
        return res;
    }
};

// 256-bit signed integer in two's complement.
struct alignas(8) int256 {
    std::uint64_t limbs[4]{0, 0, 0, 0};

    constexpr int256() noexcept = default;
    constexpr int256(std::uint64_t l0, std::uint64_t l1, std::uint64_t l2, std::uint64_t l3) noexcept
        : limbs{l0, l1, l2, l3} {}
    constexpr int256(uint128 v) noexcept : limbs{v.low, v.high, 0, 0} {}
#if defined(__SIZEOF_INT128__)
    constexpr int256(unsigned __int128 v) noexcept
        : limbs{static_cast<std::uint64_t>(v), static_cast<std::uint64_t>(v >> 64), 0, 0} {}
    constexpr int256(__int128 v) noexcept {
        unsigned __int128 uv = static_cast<unsigned __int128>(v);
        std::uint64_t sign = v < 0 ? ~0ULL : 0ULL;
        limbs[0] = static_cast<std::uint64_t>(uv);
        limbs[1] = static_cast<std::uint64_t>(uv >> 64);
        limbs[2] = sign;
        limbs[3] = sign;
    }
#endif
#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 256
#endif
    template<std::integral T>
    constexpr int256(T val) noexcept {
        if constexpr (std::is_signed_v<T>) {
            std::uint64_t u = static_cast<std::uint64_t>(val);
            std::uint64_t sign = val < 0 ? ~0ULL : 0ULL;
            limbs[0] = u; limbs[1] = sign; limbs[2] = sign; limbs[3] = sign;
        } else {
            limbs[0] = static_cast<std::uint64_t>(val);
            limbs[1] = 0; limbs[2] = 0; limbs[3] = 0;
        }
    }
    constexpr int256(int128 v) noexcept
        : limbs{v.low, v.high,
                v.is_negative() ? ~0ULL : 0ULL,
                v.is_negative() ? ~0ULL : 0ULL} {}

    [[nodiscard]] static constexpr int256 min() noexcept {
        return {0ULL, 0ULL, 0ULL, 0x8000'0000'0000'0000ULL};
    }
    [[nodiscard]] static constexpr int256 max() noexcept {
        return {~0ULL, ~0ULL, ~0ULL, 0x7FFF'FFFF'FFFF'FFFFULL};
    }

    [[nodiscard]] constexpr bool is_negative() const noexcept {
        return static_cast<std::int64_t>(limbs[3]) < 0;
    }
    [[nodiscard]] constexpr bool is_zero() const noexcept {
        return limbs[0] == 0 && limbs[1] == 0 && limbs[2] == 0 && limbs[3] == 0;
    }

    constexpr bool operator==(const int256&) const noexcept = default;
    constexpr std::strong_ordering operator<=>(const int256& o) const noexcept {
        auto h1 = static_cast<std::int64_t>(limbs[3]);
        auto h2 = static_cast<std::int64_t>(o.limbs[3]);
        if (auto cmp = h1 <=> h2; cmp != 0) return cmp;
        for (int i = 2; i >= 0; --i) {
            if (auto cmp = limbs[i] <=> o.limbs[i]; cmp != 0) return cmp;
        }
        return std::strong_ordering::equal;
    }

    constexpr int256 operator~() const noexcept {
        return {~limbs[0], ~limbs[1], ~limbs[2], ~limbs[3]};
    }
    constexpr int256 operator-() const noexcept {
        uint256 u = ~uint256(limbs[0], limbs[1], limbs[2], limbs[3]) + uint256{1ULL, 0ULL, 0ULL, 0ULL};
        return {u.limbs[0], u.limbs[1], u.limbs[2], u.limbs[3]};
    }

    constexpr int256 operator+(const int256& o) const noexcept {
        uint256 res = uint256(limbs[0], limbs[1], limbs[2], limbs[3]) +
                      uint256(o.limbs[0], o.limbs[1], o.limbs[2], o.limbs[3]);
        return {res.limbs[0], res.limbs[1], res.limbs[2], res.limbs[3]};
    }
    constexpr int256 operator*(const int256& o) const noexcept {
        uint256 u = uint256(limbs[0], limbs[1], limbs[2], limbs[3]) *
                    uint256(o.limbs[0], o.limbs[1], o.limbs[2], o.limbs[3]);
        return int256(u.limbs[0], u.limbs[1], u.limbs[2], u.limbs[3]);
    }
    constexpr int256 operator-(const int256& o) const noexcept {
        uint256 res = uint256(limbs[0], limbs[1], limbs[2], limbs[3]) -
                      uint256(o.limbs[0], o.limbs[1], o.limbs[2], o.limbs[3]);
        return {res.limbs[0], res.limbs[1], res.limbs[2], res.limbs[3]};
    }
    constexpr int256& operator+=(const int256& o) noexcept { *this = *this + o; return *this; }
    constexpr int256& operator-=(const int256& o) noexcept { *this = *this - o; return *this; }

    constexpr int256 operator&(const int256& o) const noexcept {
        return {limbs[0] & o.limbs[0], limbs[1] & o.limbs[1], limbs[2] & o.limbs[2], limbs[3] & o.limbs[3]};
    }
    constexpr int256 operator|(const int256& o) const noexcept {
        return {limbs[0] | o.limbs[0], limbs[1] | o.limbs[1], limbs[2] | o.limbs[2], limbs[3] | o.limbs[3]};
    }
    constexpr int256 operator^(const int256& o) const noexcept {
        return {limbs[0] ^ o.limbs[0], limbs[1] ^ o.limbs[1], limbs[2] ^ o.limbs[2], limbs[3] ^ o.limbs[3]};
    }

    constexpr int256 operator<<(unsigned shift) const noexcept {
        uint256 u = uint256(limbs[0], limbs[1], limbs[2], limbs[3]) << shift;
        return {u.limbs[0], u.limbs[1], u.limbs[2], u.limbs[3]};
    }
    constexpr int256 operator>>(unsigned shift) const noexcept {
        if (shift >= 256) return is_negative() ? int256(~0ULL, ~0ULL, ~0ULL, ~0ULL) : int256();
        if (shift == 0) return *this;
        bool neg = is_negative();
        std::uint64_t sign_ext = neg ? ~0ULL : 0ULL;
        unsigned limb_shift = shift / 64;
        unsigned bit_shift = shift % 64;
        int256 result{};
        for (unsigned i = 0; i < 4; ++i) {
            result.limbs[i] = sign_ext;
        }
        for (unsigned i = 0; i + limb_shift < 4; ++i) {
            unsigned src = i + limb_shift;
            std::uint64_t curr = limbs[src];
            std::uint64_t next = (src + 1 < 4) ? limbs[src + 1] : sign_ext;
            if (bit_shift == 0) {
                result.limbs[i] = curr;
            } else {
                result.limbs[i] = (curr >> bit_shift) | (next << (64 - bit_shift));
            }
        }
        return result;
    }
};

[[nodiscard]] constexpr uint128 magnitude(int128 v) noexcept {
    if (v.is_negative()) {
        return ~uint128(v.low, v.high) + uint128(1ULL, 0ULL);
    }
    return {v.low, v.high};
}

[[nodiscard]] constexpr uint256 magnitude(int256 v) noexcept {
    if (v.is_negative()) {
        return ~uint256(v.limbs[0], v.limbs[1], v.limbs[2], v.limbs[3]) + uint256(1ULL, 0ULL, 0ULL, 0ULL);
    }
    return {v.limbs[0], v.limbs[1], v.limbs[2], v.limbs[3]};
}

[[nodiscard]] constexpr int bit_width(uint128 v) noexcept {
    if (v.high != 0) return 128 - std::countl_zero(v.high);
    if (v.low != 0) return 64 - std::countl_zero(v.low);
    return 0;
}

[[nodiscard]] constexpr int bit_width(uint256 v) noexcept {
    for (int i = 3; i >= 0; --i) {
        if (v.limbs[i] != 0) return (i + 1) * 64 - std::countl_zero(v.limbs[i]);
    }
    return 0;
}

static_assert(sizeof(uint128) == 16 && alignof(uint128) <= 8);
static_assert(sizeof(int128) == 16 && alignof(int128) <= 8);
static_assert(sizeof(uint256) == 32 && alignof(uint256) <= 8);
static_assert(sizeof(int256) == 32 && alignof(int256) <= 8);
static_assert(std::is_standard_layout_v<uint128> && std::is_trivially_copyable_v<uint128>);
static_assert(std::is_standard_layout_v<int128> && std::is_trivially_copyable_v<int128>);
static_assert(std::is_standard_layout_v<uint256> && std::is_trivially_copyable_v<uint256>);
static_assert(std::is_standard_layout_v<int256> && std::is_trivially_copyable_v<int256>);


constexpr uint128::operator int128() const noexcept {
    return int128(low, high);
}

// Mixed operators with integers
template<std::integral T> constexpr int128 operator*(T a, int128 b) noexcept { return int128(a) * b; }
template<std::integral T> constexpr int128 operator*(int128 a, T b) noexcept { return a * int128(b); }
template<std::integral T> constexpr int128 operator+(T a, int128 b) noexcept { return int128(a) + b; }
template<std::integral T> constexpr int128 operator+(int128 a, T b) noexcept { return a + int128(b); }
template<std::integral T> constexpr int128 operator-(int128 a, T b) noexcept { return a - int128(b); }
template<std::integral T> constexpr int128 operator-(T a, int128 b) noexcept { return int128(a) - b; }

template<std::integral T> constexpr int128 operator/(int128 a, T b) noexcept {
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    return int128(static_cast<__int128>(a) / static_cast<__int128>(b));
#else
    bool neg = a.is_negative() != (b < 0);
    uint128 ma = magnitude(a);
    uint64_t mb = static_cast<uint64_t>(b < 0 ? 0ULL - static_cast<uint64_t>(b) : static_cast<uint64_t>(b));
    uint64_t qhi = ma.high / mb;
    uint64_t rem = ma.high % mb;
    uint64_t qlo = 0;
    for (int i = 63; i >= 0; --i) {
        uint64_t carry = (rem >> 63) & 1;
        rem = (rem << 1) | ((ma.low >> i) & 1);
        if (carry || rem >= mb) {
            rem -= mb;
            qlo |= (1ULL << i);
        }
    }
    uint128 q(qlo, qhi);
    return neg ? -int128(q.low, q.high) : int128(q.low, q.high);
#endif
}

constexpr uint128 operator/(uint128 a, uint128 b) noexcept {
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    if (b.is_zero()) return uint128{};
    return uint128(static_cast<unsigned __int128>(a) / static_cast<unsigned __int128>(b));
#else
    if (b.is_zero() || a < b) return uint128{};
    uint128 q{};
    uint128 rem{};
    for (int i = 127; i >= 0; --i) {
        rem = rem << 1;
        unsigned limb_idx = static_cast<unsigned>(i / 64);
        unsigned bit_idx = static_cast<unsigned>(i % 64);
        std::uint64_t limb = (limb_idx == 1) ? a.high : a.low;
        rem.low |= (limb >> bit_idx) & 1ULL;
        if (rem >= b) {
            rem = rem - b;
            if (limb_idx == 1) q.high |= (1ULL << bit_idx);
            else q.low |= (1ULL << bit_idx);
        }
    }
    return q;
#endif
}

constexpr uint128 operator%(uint128 a, uint128 b) noexcept {
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    if (b.is_zero()) return uint128{};
    return uint128(static_cast<unsigned __int128>(a) % static_cast<unsigned __int128>(b));
#else
    if (b.is_zero()) return uint128{};
    if (a < b) return a;
    uint128 rem{};
    for (int i = 127; i >= 0; --i) {
        rem = rem << 1;
        unsigned limb_idx = static_cast<unsigned>(i / 64);
        unsigned bit_idx = static_cast<unsigned>(i % 64);
        std::uint64_t limb = (limb_idx == 1) ? a.high : a.low;
        rem.low |= (limb >> bit_idx) & 1ULL;
        if (rem >= b) {
            rem = rem - b;
        }
    }
    return rem;
#endif
}

constexpr uint256 operator/(uint256 a, uint256 b) noexcept {
    if (b.is_zero() || a < b) return uint256{};
    uint256 q{};
    uint256 rem{};
    for (int i = 255; i >= 0; --i) {
        rem = rem << 1;
        unsigned limb_idx = static_cast<unsigned>(i / 64);
        unsigned bit_idx = static_cast<unsigned>(i % 64);
        rem.limbs[0] |= (a.limbs[limb_idx] >> bit_idx) & 1ULL;
        if (rem >= b) {
            rem = rem - b;
            q.limbs[limb_idx] |= (1ULL << bit_idx);
        }
    }
    return q;
}

constexpr uint256 operator%(uint256 a, uint256 b) noexcept {
    if (b.is_zero()) return uint256{};
    if (a < b) return a;
    uint256 rem{};
    for (int i = 255; i >= 0; --i) {
        rem = rem << 1;
        unsigned limb_idx = static_cast<unsigned>(i / 64);
        unsigned bit_idx = static_cast<unsigned>(i % 64);
        rem.limbs[0] |= (a.limbs[limb_idx] >> bit_idx) & 1ULL;
        if (rem >= b) {
            rem = rem - b;
        }
    }
    return rem;
}

} // namespace fixedwide::wide

namespace fixedwide {
using u128 = wide::uint128;
using i128 = wide::int128;
using u256 = wide::uint256;
using i256 = wide::int256;

[[nodiscard]] constexpr unsigned bit_width(wide::uint128 v) noexcept {
    if (v.high != 0) return 128 - std::countl_zero(v.high);
    if (v.low != 0) return 64 - std::countl_zero(v.low);
    return 0;
}
[[nodiscard]] constexpr unsigned bit_width(wide::uint256 v) noexcept {
    for (int i = 3; i >= 0; --i) {
        if (v.limbs[i] != 0) return (i + 1) * 64 - std::countl_zero(v.limbs[i]);
    }
    return 0;
}

inline constexpr i128 i128_min = wide::int128::min();
inline constexpr i128 i128_max = wide::int128::max();
inline constexpr u128 u128_max = wide::uint128::max();
inline constexpr i256 i256_min = wide::int256::min();
inline constexpr i256 i256_max = wide::int256::max();
inline constexpr u256 u256_max = wide::uint256::max();
}
