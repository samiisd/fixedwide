#include "coverage_support.hpp"
#include <fixedwide/bigint.hpp>
#include "src/division.hpp"
#include "src/native.hpp"
#include "src/limbs.hpp"

using namespace coverage_test;
using namespace fixedwide;

// Bitwise and arithmetic wide operators deliberately wrap modulo their width.
// The checked divmod API has a different contract and is exercised separately.
template<class W>
void operators() {
    constexpr unsigned width = sizeof(W) * 8;
    const cpp_int mask = (cpp_int{1} << width) - 1;
    std::mt19937_64 rng(0x7477696465ULL + width);
    auto values = boundaries(width);
    for (unsigned i = 0; i < 64; ++i) values.push_back(random_bits(rng, width));
    for (const cpp_int& av : values) {
        const W a = from_integer<W>(av);
        const cpp_int aa = bits(a);
        CHECK(bits(~a) == (mask ^ aa));
        CHECK(a.is_zero() == (aa == 0));
        CHECK(a == a);
        CHECK((a <=> a) == 0);
        CHECK(bits(a + W{1}) == ((aa + 1) & mask));
        CHECK(bits(a - W{1}) == ((aa - 1) & mask));
        W v = a;
        v += W{1};
        CHECK(bits(v) == ((aa + 1) & mask));
        v -= W{1};
        CHECK(v == a);
        if constexpr (requires { a.is_negative(); }) {
            CHECK(bits(-a) == ((-aa) & mask));
            CHECK(integer(magnitude(a)) == boost::multiprecision::abs(integer(a)));
        } else
            CHECK(fixedwide::wide::bit_width(a) ==
                  (aa == 0 ? 0 : static_cast<int>(boost::multiprecision::msb(aa) + 1)));
        for (unsigned shift : {0u, 1u, 31u, 32u, 63u, 64u, 65u, 127u, 128u, 129u, 191u, 192u, 255u, 256u, 300u}) {
            CHECK(bits(a << shift) == ((aa << shift) & mask));
            cpp_int expected = integer(a) >> shift;
            expected &= mask;
            CHECK(bits(a >> shift) == expected);
        }
        for (unsigned j = 0; j < 8; ++j) {
            const W b = from_integer<W>(random_bits(rng, width));
            const cpp_int bb = bits(b);
            CHECK(bits(a + b) == ((aa + bb) & mask));
            CHECK(bits(a - b) == ((aa - bb) & mask));
            CHECK(bits(a * b) == ((aa * bb) & mask));
            CHECK(bits(a & b) == (aa & bb));
            CHECK(bits(a | b) == (aa | bb));
            CHECK(bits(a ^ b) == (aa ^ bb));
            CHECK((a < b) == (integer(a) < integer(b)));
            CHECK((a > b) == (integer(a) > integer(b)));
        }
    }
    CHECK(bits(W::min()) == (requires(W x) { x.is_negative(); } ? cpp_int{1} << (width - 1) : cpp_int{0}));
    CHECK(bits(W::max()) == (requires(W x) { x.is_negative(); } ? mask >> 1 : mask));
    CHECK(integer(W{std::uint64_t{42}}) == 42);
    CHECK(integer(W{std::int64_t{42}}) == 42);
}

template<class U>
void unsigned_division() {
    constexpr unsigned width = sizeof(U) * 8;
    std::mt19937_64 rng(0x646976ULL + width);
    std::vector<cpp_int> vals{0, 1, 2, 3, 7, (cpp_int{1} << (width - 1)), (cpp_int{1} << width) - 1};
    for (unsigned i = 0; i < 24; ++i) vals.push_back(random_bits(rng, width));
    for (const auto& a : vals)
        for (const auto& b : vals) {
            const U aa = from_integer<U>(a), bb = from_integer<U>(b);
            CHECK(integer(aa / bb) == (b == 0 ? cpp_int{0} : cpp_int{a / b}));
            CHECK(integer(aa % bb) == (b == 0 ? cpp_int{0} : cpp_int{a % b}));
        }
}

void primitives() {
    std::mt19937_64 rng(0x7072696d69746976ULL);
    for (unsigned i = 0; i < 4000; ++i) {
        const unsigned w = (i % 4 + 1) * 64;
        const cpp_int a = random_bits(rng, w), b = random_bits(rng, i % 2 == 0 ? 64 : 128);
        const u256 num = from_integer<u256>(a);
        const u128 den = from_integer<u128>(b);
        const auto full = fixedwide::divmod(num, den);
        const auto narrow = detail::divide_narrow(num, den);
        if (b == 0) {
            CHECK(!full && !narrow);
            continue;
        }
        CHECK(full.has_value());
        CHECK(integer(full->quotient) == a / b);
        CHECK(integer(full->remainder) == a % b);
        if (a / b >= (cpp_int{1} << 128))
            CHECK(!narrow && narrow.error() == ArithmeticError::overflow);
        else {
            CHECK(narrow.has_value());
            CHECK(integer(narrow->quotient) == a / b);
            CHECK(integer(narrow->remainder) == a % b);
        }
        const u128 x = from_integer<u128>(random_bits(rng, i % 2 == 0 ? 64 : 128));
        const u128 y = from_integer<u128>(random_bits(rng, i % 3 == 0 ? 64 : 128));
        CHECK(integer(detail::multiply128(x, y)) == integer(x) * integer(y));
    }
    CHECK(!detail::divide_narrow(u256{}, u128{}));
    CHECK(!fixedwide::divmod(u256{}, u128{}));
    CHECK(!fixedwide::divmod(i256{}, i128{}));
}

void narrow_division_edges() {
    const cpp_int word = cpp_int{1} << 64;
    for (std::uint64_t d :
         {UINT64_C(1), UINT64_C(3), (UINT64_C(1) << 32) - 1, (UINT64_C(1) << 32) + 1, UINT64_C(1) << 63, UINT64_MAX}) {
        for (std::uint64_t h : {UINT64_C(0), d / 2, d - 1})
            for (std::uint64_t l : {UINT64_C(0), UINT64_C(1), UINT64_MAX}) {
                std::uint64_t r = 0;
                const auto q = detail::div128by64(h, l, d, r);
                const cpp_int n = cpp_int{h} * word + l;
                CHECK(cpp_int{q} == n / d && cpp_int{r} == n % d);
            }
    }
}

void normalized_division_edges() {
    const cpp_int base = cpp_int{1} << 64;
    // Algorithm D's maximum-digit estimate, carry out of rhat, and both
    // one-limb/two-limb quotient paths. The invariant is remainder < divisor.
    for (std::uint64_t v1 : {UINT64_C(1) << 63, UINT64_MAX})
        for (std::uint64_t v0 : {UINT64_C(1), UINT64_MAX}) {
            const cpp_int den = (cpp_int{v1} << 64) + v0;
            for (const cpp_int rem : std::vector<cpp_int>{0, 1, den - 1, den / 2})
                for (std::uint64_t next : {UINT64_C(0), UINT64_C(1), UINT64_MAX}) {
                    const cpp_int n = rem * base + next;
                    auto rw = from_integer<u128>(rem);
                    const auto q = detail::divide_digit(rw, next, v0, v1);
                    CHECK(cpp_int{q} == n / den);
                    CHECK(integer(rw) == n % den);
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
                    auto rn = detail::nat::load(from_integer<u128>(rem));
                    const auto qn = detail::nat::divide_digit(rn, next, v0, v1);
                    CHECK(cpp_int{qn} == n / den);
                    CHECK(integer(detail::nat::store(rn)) == n % den);
#endif
                }
            for (cpp_int q : std::vector<cpp_int>{1, 7, base - 1, base, base + 1}) {
                const cpp_int n = den * q + den - 1;
                const auto got = detail::divide_narrow(from_integer<u256>(n), from_integer<u128>(den));
                CHECK(got && integer(got->quotient) == q && integer(got->remainder) == den - 1);
            }
        }
    // A previous quotient correction reused qhat*v2 after decrementing qhat.
    // That made an exactly divisible multi-limb numerator return one unit short.
    const cpp_int d("33377827660633392054828866544284941527744376448277243731116410742665482086019");
    const cpp_int q = ((cpp_int{1} << 127) - 1) * power10(38), n = d * q;
    detail::u1024_limbs nn{}, dd{};
    for (unsigned i = 0; i < 16; ++i) {
        nn.limbs[i] = static_cast<std::uint64_t>((n >> (i * 64)) & ((cpp_int{1} << 64) - 1));
        dd.limbs[i] = static_cast<std::uint64_t>((d >> (i * 64)) & ((cpp_int{1} << 64) - 1));
    }
    const auto got = detail::divmod_knuth(nn, dd);
    cpp_int actual = 0;
    for (unsigned i = 16; i > 0; --i) actual = (actual << 64) + got.quotient.limbs[i - 1];
    CHECK(actual == q && got.remainder.is_zero());
}

void signed_division() {
    const auto nums = boundaries(256), dens = boundaries(128);
    for (const auto& n : nums)
        for (const auto& d : dens) {
            const auto actual = fixedwide::divmod(from_integer<i256>(n), from_integer<i128>(d));
            const bool overflow = n == -(cpp_int{1} << 255) && d == -1;
            if (d == 0)
                CHECK(!actual && actual.error() == ArithmeticError::division_by_zero);
            else if (overflow)
                CHECK(!actual && actual.error() == ArithmeticError::overflow);
            else {
                CHECK(actual.has_value());
                CHECK(integer(actual->quotient) == n / d);
                CHECK(integer(actual->remainder) == n % d);
            }
            for (const auto mode : modes) {
                agrees(divide_to_i128(from_integer<i256>(n), from_integer<i128>(d), mode), rounded(n, d, mode, 128));
            }
        }
    for (std::int64_t n = -12; n <= 12; ++n)
        for (std::int64_t d = -5; d <= 5; ++d)
            for (auto mode : modes)
                agrees(mul_div(i128{n}, i128{7}, i128{d}, mode), rounded(cpp_int{n} * 7, d, mode, 128));
    for (auto mode : modes) {
        agrees(mul_div(i128::min(), i128::max(), i128{1}, mode),
               rounded(-(cpp_int{1} << 127) * ((cpp_int{1} << 127) - 1), 1, mode, 128));
        agrees(mul_div(i128::max(), i128{2}, i128{2}, mode), rounded(((cpp_int{1} << 127) - 1) * 2, 2, mode, 128));
    }
}

void conversions() {
    const i128 a{-42};
    const u128 u{23ULL};
    CHECK(integer(static_cast<i128>(u)) == 23);
    CHECK(bits(static_cast<u128>(a)) == ((cpp_int{1} << 128) - 42));
    CHECK(static_cast<std::int64_t>(a) == -42);
    CHECK(static_cast<std::uint64_t>(u) == 23);
    CHECK(integer(i256{a}) == -42);
    CHECK(integer(i256{u}) == 23);
    CHECK(integer(u256{u}) == 23);
    CHECK(integer(static_cast<u128>(u256{u})) == 23);
    CHECK(integer(a + 4) == -38);
    CHECK(integer(4 + a) == -38);
    CHECK(integer(a - 4) == -46);
    CHECK(integer(4 - a) == 46);
    CHECK(integer(a * 4) == -168);
    CHECK(integer(4 * a) == -168);
    CHECK(integer(a / 5) == -8);
    CHECK(integer(a / -5) == 8);
    CHECK(integer(i128{42} / 5) == 8);
    u128 v{0xFFULL, 0xBBULL};
    v &= u128{0x0FULL, 0x0FULL};
    CHECK(v == u128(15ULL, 11ULL));
    v |= u128{0xF0ULL, 0xF0ULL};
    CHECK(v == u128(255ULL, 251ULL));
    v ^= u128{0xFFULL, 0xFFULL};
    CHECK(v == u128(0ULL, 4ULL));
    v >>= 64;
    CHECK(v == u128{4});
    v <<= 3;
    CHECK(v == u128{32});
#if defined(__SIZEOF_INT128__)
    const __int128 neg = -static_cast<__int128>(42);
    const unsigned __int128 pos = static_cast<unsigned __int128>(1) << 80;
    CHECK(integer(i128{neg}) == -42);
    CHECK(integer(i128{pos}) == (cpp_int{1} << 80));
    CHECK(bits(u128{neg}) == (cpp_int{1} << 128) - 42);
    CHECK(bits(u128{pos}) == (cpp_int{1} << 80));
    CHECK(static_cast<__int128>(i128{neg}) == neg);
    CHECK(static_cast<unsigned __int128>(i128{pos}) == pos);
    CHECK(static_cast<__int128>(u128{pos}) == static_cast<__int128>(pos));
    CHECK(static_cast<unsigned __int128>(u128{pos}) == pos);
    CHECK(integer(i256{neg}) == -42);
    CHECK(integer(i256{pos}) == (cpp_int{1} << 80));
    CHECK(bits(u256{neg}) == (cpp_int{1} << 256) - 42);
    CHECK(bits(u256{pos}) == (cpp_int{1} << 80));
#endif
}

void run_coverage_wide() {
    operators<u128>();
    operators<i128>();
    operators<u256>();
    operators<i256>();
    unsigned_division<u128>();
    unsigned_division<u256>();
    conversions();
    primitives();
    narrow_division_edges();
    normalized_division_edges();
    signed_division();
}
