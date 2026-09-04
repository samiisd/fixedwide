#pragma once
#include <cstdint>
#include <cstddef>
#include <compare>
#include <bit>
#include <algorithm>
#include "detail.hpp"

namespace fixedwide::detail {

template<std::size_t L>
struct uint_limbs {
    std::uint64_t limbs[L]{};

    constexpr uint_limbs() noexcept = default;

    template<std::integral T>
    constexpr uint_limbs(T val) noexcept {
        limbs[0] = static_cast<std::uint64_t>(val);
    }

    constexpr uint_limbs(wide::uint128 v) noexcept {
        limbs[0] = v.low;
        if constexpr (L > 1) limbs[1] = v.high;
    }

    constexpr uint_limbs(wide::uint256 v) noexcept {
        for (std::size_t i = 0; i < std::min(L, std::size_t{4}); ++i) {
            limbs[i] = v.limbs[i];
        }
    }

    template<std::size_t L2>
    constexpr explicit uint_limbs(const uint_limbs<L2>& o) noexcept {
        for (std::size_t i = 0; i < std::min(L, L2); ++i) {
            limbs[i] = o.limbs[i];
        }
    }

    [[nodiscard]] constexpr bool is_zero() const noexcept {
        for (std::size_t i = 0; i < L; ++i) {
            if (limbs[i] != 0) return false;
        }
        return true;
    }

    [[nodiscard]] constexpr std::size_t active_limbs() const noexcept {
        for (std::size_t i = L; i > 0; --i) {
            if (limbs[i - 1] != 0) return i;
        }
        return 0;
    }

    [[nodiscard]] constexpr unsigned bit_width() const noexcept {
        for (std::size_t i = L; i > 0; --i) {
            if (limbs[i - 1] != 0) {
                return static_cast<unsigned>(i * 64 - std::countl_zero(limbs[i - 1]));
            }
        }
        return 0;
    }

    [[nodiscard]] constexpr unsigned clz() const noexcept { return static_cast<unsigned>(L * 64) - bit_width(); }

    constexpr bool operator==(const uint_limbs&) const noexcept = default;
    constexpr std::strong_ordering operator<=>(const uint_limbs& o) const noexcept {
        for (std::size_t i = L; i > 0; --i) {
            if (auto cmp = limbs[i - 1] <=> o.limbs[i - 1]; cmp != 0) return cmp;
        }
        return std::strong_ordering::equal;
    }

    constexpr uint_limbs operator~() const noexcept {
        uint_limbs res;
        for (std::size_t i = 0; i < L; ++i) res.limbs[i] = ~limbs[i];
        return res;
    }

    constexpr uint_limbs operator&(const uint_limbs& o) const noexcept {
        uint_limbs res;
        for (std::size_t i = 0; i < L; ++i) res.limbs[i] = limbs[i] & o.limbs[i];
        return res;
    }

    constexpr uint_limbs operator|(const uint_limbs& o) const noexcept {
        uint_limbs res;
        for (std::size_t i = 0; i < L; ++i) res.limbs[i] = limbs[i] | o.limbs[i];
        return res;
    }

    constexpr uint_limbs operator^(const uint_limbs& o) const noexcept {
        uint_limbs res;
        for (std::size_t i = 0; i < L; ++i) res.limbs[i] = limbs[i] ^ o.limbs[i];
        return res;
    }

    constexpr uint_limbs operator<<(unsigned shift) const noexcept {
        if (shift >= L * 64) return uint_limbs{};
        if (shift == 0) return *this;
        unsigned limb_shift = shift / 64;
        unsigned bit_shift = shift % 64;
        uint_limbs res{};
        for (std::size_t i = limb_shift; i < L; ++i) {
            res.limbs[i] |= limbs[i - limb_shift] << bit_shift;
            if (bit_shift != 0 && i + 1 < L) {
                res.limbs[i + 1] |= limbs[i - limb_shift] >> (64 - bit_shift);
            }
        }
        return res;
    }

    constexpr uint_limbs operator>>(unsigned shift) const noexcept {
        if (shift >= L * 64) return uint_limbs{};
        if (shift == 0) return *this;
        unsigned limb_shift = shift / 64;
        unsigned bit_shift = shift % 64;
        uint_limbs res{};
        for (std::size_t i = 0; i + limb_shift < L; ++i) {
            res.limbs[i] |= limbs[i + limb_shift] >> bit_shift;
            if (bit_shift != 0 && i + limb_shift + 1 < L) {
                res.limbs[i] |= limbs[i + limb_shift + 1] << (64 - bit_shift);
            }
        }
        return res;
    }

    constexpr uint_limbs operator+(const uint_limbs& o) const noexcept {
        uint_limbs res{};
        std::uint64_t carry = 0;
        for (std::size_t i = 0; i < L; ++i) {
            std::uint64_t a = limbs[i];
            std::uint64_t b = o.limbs[i];
            std::uint64_t s = a + b + carry;
            carry = (s < a) || (carry && s == a) ? 1 : 0;
            res.limbs[i] = s;
        }
        return res;
    }

    constexpr uint_limbs operator-(const uint_limbs& o) const noexcept {
        uint_limbs res{};
        std::uint64_t borrow = 0;
        for (std::size_t i = 0; i < L; ++i) {
            std::uint64_t a = limbs[i];
            std::uint64_t b = o.limbs[i] + borrow;
            borrow = (b < o.limbs[i]) || (a < b) ? 1 : 0;
            res.limbs[i] = a - b;
        }
        return res;
    }

    constexpr bool operator==(std::uint64_t val) const noexcept { return limbs[0] == val && active_limbs() <= 1; }
    constexpr bool operator!=(std::uint64_t val) const noexcept { return !(*this == val); }

    constexpr uint_limbs operator+(std::uint64_t val) const noexcept { return *this + uint_limbs(val); }
    constexpr uint_limbs operator-(std::uint64_t val) const noexcept { return *this - uint_limbs(val); }
    constexpr uint_limbs operator/(std::uint64_t val) const noexcept { return divmod64(*this, val).quotient; }
    constexpr uint_limbs operator/(int val) const noexcept {
        return divmod64(*this, static_cast<std::uint64_t>(val)).quotient;
    }
    constexpr uint_limbs operator%(std::uint64_t val) const noexcept {
        return uint_limbs(divmod64(*this, val).remainder);
    }

    constexpr uint_limbs& operator+=(const uint_limbs& o) noexcept {
        *this = *this + o;
        return *this;
    }
    constexpr uint_limbs& operator+=(std::uint64_t val) noexcept {
        *this = *this + val;
        return *this;
    }
    constexpr uint_limbs& operator-=(const uint_limbs& o) noexcept {
        *this = *this - o;
        return *this;
    }
    constexpr uint_limbs& operator-=(std::uint64_t val) noexcept {
        *this = *this - val;
        return *this;
    }

    constexpr uint_limbs operator&(std::uint64_t val) const noexcept {
        uint_limbs res{};
        res.limbs[0] = limbs[0] & val;
        return res;
    }
    constexpr uint_limbs operator^(std::uint64_t val) const noexcept {
        uint_limbs res = *this;
        res.limbs[0] ^= val;
        return res;
    }
    constexpr uint_limbs operator|(std::uint64_t val) const noexcept {
        uint_limbs res = *this;
        res.limbs[0] |= val;
        return res;
    }

    [[nodiscard]] constexpr wide::uint128 to_uint128() const noexcept {
        return wide::uint128(limbs[0], L > 1 ? limbs[1] : 0ULL);
    }

    [[nodiscard]] constexpr wide::uint256 to_uint256() const noexcept {
        return wide::uint256(limbs[0], L > 1 ? limbs[1] : 0ULL, L > 2 ? limbs[2] : 0ULL, L > 3 ? limbs[3] : 0ULL);
    }
};

// Full product of L1 limbs by L2 limbs -> L1 + L2 limbs
template<std::size_t L1, std::size_t L2>
[[nodiscard]] inline uint_limbs<L1 + L2> mul_full(const uint_limbs<L1>& a, const uint_limbs<L2>& b) noexcept {
    uint_limbs<L1 + L2> res{};
    for (std::size_t i = 0; i < L1; ++i) {
        std::uint64_t carry = 0;
        for (std::size_t j = 0; j < L2; ++j) {
            std::uint64_t hi, lo;
            mul64x64(a.limbs[i], b.limbs[j], hi, lo);
            // add lo and carry to res.limbs[i + j]
            std::uint64_t sum1 = res.limbs[i + j] + lo;
            std::uint64_t c1 = (sum1 < lo) ? 1 : 0;
            std::uint64_t sum2 = sum1 + carry;
            std::uint64_t c2 = (sum2 < sum1) ? 1 : 0;
            res.limbs[i + j] = sum2;
            carry = hi + c1 + c2;
        }
        // propagate carry to next limbs
        std::size_t k = i + L2;
        while (carry != 0 && k < L1 + L2) {
            std::uint64_t sum = res.limbs[k] + carry;
            carry = (sum < carry) ? 1 : 0;
            res.limbs[k] = sum;
            ++k;
        }
    }
    return res;
}

template<std::size_t L>
constexpr uint_limbs<L> operator*(const uint_limbs<L>& a, const uint_limbs<L>& b) noexcept {
    uint_limbs<L> res{};
    for (std::size_t i = 0; i < L; ++i) {
        std::uint64_t carry = 0;
        for (std::size_t j = 0; i + j < L; ++j) {
            std::uint64_t hi, lo;
            mul64x64(a.limbs[i], b.limbs[j], hi, lo);
            std::uint64_t s1 = res.limbs[i + j] + lo;
            std::uint64_t c1 = (s1 < lo) ? 1 : 0;
            std::uint64_t s2 = s1 + carry;
            std::uint64_t c2 = (s2 < s1) ? 1 : 0;
            res.limbs[i + j] = s2;
            carry = hi + c1 + c2;
        }
    }
    return res;
}

// Multi-limb division by a 64-bit divisor
template<std::size_t L>
struct DivMod64Result {
    uint_limbs<L> quotient{};
    std::uint64_t remainder{0};
};

// always_inline: the caller keeps the limbs in its own frame instead of handing
// a 72-byte result back through memory. The profile of a Fixed256 multiply was
// dominated by exactly those stack round-trips, not by the divisions.
template<std::size_t L>
[[nodiscard, gnu::always_inline]] inline DivMod64Result<L> divmod64(const uint_limbs<L>& num,
                                                                    std::uint64_t divisor) noexcept {
    DivMod64Result<L> res{};
    std::uint64_t rem = 0;
    // Skip leading zero limbs. While the remainder is still zero, a zero limb
    // yields a zero quotient limb and leaves the remainder zero, so the divide
    // is pure cost -- and these divides are serially dependent, each waiting on
    // the previous remainder. A Fixed256 product usually occupies three of the
    // eight limbs of its 512-bit intermediate, so five of the eight hardware
    // divisions were being executed for nothing.
    std::size_t i = L;
    while (i > 0 && num.limbs[i - 1] == 0) --i;
    for (; i > 0; --i) {
        res.quotient.limbs[i - 1] = div128by64(rem, num.limbs[i - 1], divisor, rem);
    }
    res.remainder = rem;
    return res;
}

// General Knuth Algorithm D multi-limb division
template<std::size_t Lnum, std::size_t Lden>
struct DivModResult {
    uint_limbs<Lnum> quotient{};
    uint_limbs<Lden> remainder{};
};

template<std::size_t Lnum, std::size_t Lden>
[[nodiscard]] inline DivModResult<Lnum, Lden> divmod_knuth(const uint_limbs<Lnum>& num,
                                                           const uint_limbs<Lden>& den) noexcept {
    DivModResult<Lnum, Lden> res{};
    std::size_t n = den.active_limbs();
    if (n == 0) return res; // Division by zero
    std::size_t m_plus_n = num.active_limbs();
    if (m_plus_n < n) {
        for (std::size_t i = 0; i < std::min(Lnum, Lden); ++i) {
            res.remainder.limbs[i] = num.limbs[i];
        }
        return res;
    }
    if (n == 1) {
        auto r64 = divmod64(num, den.limbs[0]);
        res.quotient = r64.quotient;
        res.remainder.limbs[0] = r64.remainder;
        return res;
    }

    std::size_t m = m_plus_n - n;
    const unsigned shift = static_cast<unsigned>(std::countl_zero(den.limbs[n - 1]));

    // Normalized divisor
    std::uint64_t v[Lden + 1]{};
    if (shift == 0) {
        for (std::size_t i = 0; i < n; ++i) v[i] = den.limbs[i];
    } else {
        for (std::size_t i = 0; i < n; ++i) {
            v[i] |= den.limbs[i] << shift;
            if (i + 1 < n) v[i + 1] |= den.limbs[i] >> (64 - shift);
        }
    }

    // Normalized dividend
    std::uint64_t u[Lnum + 2]{};
    if (shift == 0) {
        for (std::size_t i = 0; i < m_plus_n; ++i) u[i] = num.limbs[i];
    } else {
        for (std::size_t i = 0; i < m_plus_n; ++i) {
            u[i] |= num.limbs[i] << shift;
            u[i + 1] |= num.limbs[i] >> (64 - shift);
        }
    }

    std::uint64_t v1 = v[n - 1];
    std::uint64_t v2 = v[n - 2];

    // Counts down from m to 0 inclusive. `j` is std::size_t rather than int so
    // that every u[j + i] indexes without a signed-to-unsigned conversion; the
    // `j-- > 0` idiom is what makes an unsigned countdown terminate.
    for (std::size_t j = m + 1; j-- > 0;) {
        std::uint64_t u_top = u[j + n];
        std::uint64_t u_next = u[j + n - 1];
        std::uint64_t u_next2 = u[j + n - 2];

        std::uint64_t qhat, rhat;
        bool carry = false;
        if (u_top == v1) {
            qhat = ~0ULL;
            rhat = u_next + v1;
            carry = (rhat < u_next);
        } else {
            qhat = div128by64(u_top, u_next, v1, rhat);
        }

        std::uint64_t ph, pl;
        mul64x64(qhat, v2, ph, pl);
        while (!carry && (ph > rhat || (ph == rhat && pl > u_next2))) {
            --qhat;
            std::uint64_t prev = rhat;
            rhat += v1;
            carry = (rhat < prev);
        }

        // Multiply and subtract: u[j .. j+n] -= qhat * v[0 .. n-1]
        std::uint64_t borrow = 0;
        for (std::size_t i = 0; i < n; ++i) {
            std::uint64_t p_hi, p_lo;
            mul64x64(qhat, v[i], p_hi, p_lo);
            std::uint64_t sub = p_lo + borrow;
            borrow = (sub < p_lo) ? 1 : 0;
            borrow += p_hi;
            if (u[j + i] < sub) ++borrow;
            u[j + i] -= sub;
        }
        if (u[j + n] < borrow) {
            // Add back
            --qhat;
            std::uint64_t add_carry = 0;
            for (std::size_t i = 0; i < n; ++i) {
                std::uint64_t sum = u[j + i] + v[i] + add_carry;
                add_carry = (sum < u[j + i]) || (add_carry && sum == u[j + i]) ? 1 : 0;
                u[j + i] = sum;
            }
            u[j + n] = 0;
        } else {
            u[j + n] -= borrow;
        }

        if (j < Lnum) {
            res.quotient.limbs[j] = qhat;
        }
    }

    // Unnormalize remainder
    if (shift == 0) {
        for (std::size_t i = 0; i < n && i < Lden; ++i) res.remainder.limbs[i] = u[i];
    } else {
        for (std::size_t i = 0; i < n && i < Lden; ++i) {
            res.remainder.limbs[i] |= u[i] >> shift;
            if (i + 1 < Lden + 1) res.remainder.limbs[i] |= u[i + 1] << (64 - shift);
        }
    }

    return res;
}

// Common aliases for internal wide computation
using u128_limbs = uint_limbs<2>;
using u256_limbs = uint_limbs<4>;
using u512_limbs = uint_limbs<8>;
using u768_limbs = uint_limbs<12>;
using u1024_limbs = uint_limbs<16>;

inline u1024_limbs pow10_limbs(unsigned exp) noexcept {
    u1024_limbs v(1ULL);
    for (unsigned i = 0; i < exp; ++i) {
        v = (v << 3) + (v << 1);
    }
    return v;
}

} // namespace fixedwide::detail
