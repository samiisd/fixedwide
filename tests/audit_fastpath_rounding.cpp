// Boundary regressions for the inline IDIV paths. Do not replace runtime inputs
// with constexpr fixtures: the constexpr implementation never executes IDIV.
#include <fixedwide/arithmetic.hpp>
#include "check.hpp"
#include <cstdint>
#include <cstdio>
#include <initializer_list>

namespace {
using namespace fixedwide;
using R = Rounding;
using E = ArithmeticError;
constexpr R modes[] = {R::toward_zero, R::floor, R::ceil,
                       R::nearest_even, R::nearest_away, R::exact};
enum class Op { mul, div, mul_div };

template<class F>
constexpr F raw(std::int64_t n) { return F::from_raw(typename F::raw_type(n)); }

template<class F>
constexpr auto calculate(Op op, std::int64_t a, std::int64_t b, std::int64_t c, R mode) {
    if (op == Op::mul) return mul(raw<F>(a), raw<F>(b), mode);
    if (op == Op::div) return div(raw<F>(a), raw<F>(b), mode);
    return mul_div(raw<F>(a), raw<F>(b), raw<F>(c), mode);
}

std::int64_t runtime(std::int64_t n) {
    volatile std::int64_t value = n;
    return value;
}

// Independent, exact integer facts for the three regression vectors:
// a*b / 10^8 = INT64_MAX + 74677520/100000000
// a*10^8 / b = INT64_MAX + 57309684/99999988
// a*b / c    = INT64_MAX + 1/2 (an odd tie, so nearest-even increments).
constexpr std::int64_t mul_a = 9'223'371'852'387'338'760LL;
constexpr std::int64_t mul_b = 100'000'002LL;
constexpr std::int64_t div_a = 9'223'370'930'050'131'385LL;
constexpr std::int64_t div_b = 99'999'988LL;
constexpr std::int64_t md_a = 6'148'914'691'236'517'205LL;

constexpr auto ce_mul = calculate<Fixed64<8>>(Op::mul, mul_a, mul_b, 1, R::nearest_even);
constexpr auto ce_div = calculate<Fixed64<8>>(Op::div, div_a, div_b, 1, R::nearest_even);
constexpr auto ce_md = calculate<Fixed64<8>>(Op::mul_div, md_a, 3, 2, R::nearest_even);
static_assert(!ce_mul && ce_mul.error() == E::overflow);
static_assert(!ce_div && ce_div.error() == E::overflow);
static_assert(!ce_md && ce_md.error() == E::overflow);
constexpr auto ce_wide = calculate<Fixed128<8>>(Op::mul, mul_a, mul_b, 1, R::nearest_even);
static_assert(ce_wide && ce_wide->raw() == wide::int128(0x8000'0000'0000'0000ULL, 0ULL));

// Each input has magnitude INT64_MAX + f, where 1/2 <= f < 1.
// For 64 bits, the positive rounded value overflows; the negative one is MIN.
// For 128 bits, both results are valid, including positive 2^63.
template<class F, class Call>
void check_boundary(Call call, bool negative) {
    for (R mode : modes) {
        const auto result = call(mode);
        if (mode == R::exact) {
            CHECK(!result && result.error() == E::inexact);
            continue;
        }
        const bool increment = mode == R::nearest_even || mode == R::nearest_away ||
                               (mode == R::floor && negative) || (mode == R::ceil && !negative);
        if constexpr (F::bits == 64) {
            if (increment && !negative) {
                CHECK(!result && result.error() == E::overflow);
            } else {
                const auto expected = increment ? INT64_MIN : (negative ? -INT64_MAX : INT64_MAX);
                CHECK(result && result->raw() == expected);
            }
        } else {
            auto expected = wide::int128(negative ? -INT64_MAX : INT64_MAX);
            if (increment) expected = expected + wide::int128(negative ? -1 : 1);
            CHECK(result && result->raw() == expected);
        }
    }
}

template<class F>
void boundary(Op op, std::int64_t a, std::int64_t b, std::int64_t c, bool negative) {
    a = runtime(a); b = runtime(b); c = runtime(c);
    check_boundary<F>([&](R mode) { return calculate<F>(op, a, b, c, mode); }, negative);
}

// Independent oracle for the runtime-divisor rounding comparison. It covers
// every remainder/parity at small divisors plus the largest signed magnitude.
void rounding_comparison() {
    auto check_increment = [](std::uint64_t q, std::uint64_t r, std::uint64_t d) {
        const bool expected = r > d - r || (r == d - r && (q & 1) != 0);
        CHECK(detail_arith::nearest_even_inc(q, r, d) == expected);
    };
    for (std::uint64_t d = 1; d <= 512; ++d)
        for (std::uint64_t r = 0; r < d; ++r)
            for (std::uint64_t q = 0; q < 2; ++q) check_increment(q, r, d);
    const std::uint64_t divisors[] = {2, 3, 100'000'000, 1'000'000'000'000ULL,
        1'000'000'000'000'000'000ULL, 0x7fff'ffff'ffff'ffffULL, 0x8000'0000'0000'0000ULL};
    for (auto d : divisors) {
        const std::uint64_t remainders[] = {0, 1, d / 2 - 1, d / 2, d / 2 + 1, d - 1};
        for (auto r : remainders) if (r < d) {
            check_increment(0, r, d);
            check_increment(1, r, d);
            check_increment(UINT64_MAX, r, d);
        }
    }
    // Positive decimal divisor: IDIV's remainder has the numerator's sign.
    for (auto q : {INT64_MIN, std::int64_t{-1}, std::int64_t{0}, std::int64_t{1}, INT64_MAX})
        for (auto r : {std::int64_t{0}, std::int64_t{1}, std::int64_t{49'999'999},
                      std::int64_t{50'000'000}, std::int64_t{50'000'001}, std::int64_t{99'999'999}})
            for (bool neg : {false, true}) {
                const auto actual = detail_arith::nearest_scaled_adj<100'000'000>(q, neg ? -r : r, neg);
                const bool inc = r > 100'000'000 - r || (r == 100'000'000 - r && (q & 1) != 0);
                CHECK(actual == (inc ? (neg ? -1 : 1) : 0));
            }
}

void regressions() {
    const std::int64_t quotients[] = {INT64_MIN, INT64_MIN + 1, -2, -1, 0, 1, 2, INT64_MAX - 1, INT64_MAX};
    for (auto q : quotients) for (std::int64_t adj : {std::int64_t{-1}, std::int64_t{0}, std::int64_t{1}}) {
        if ((q < 0 && adj > 0) || (q > 0 && adj < 0)) continue;
        CHECK(detail_arith::round_wide_quotient(q, adj) == wide::int128(q) + wide::int128(adj));
    }
    for (std::int64_t sign : {std::int64_t{-1}, std::int64_t{1}}) {
        boundary<Fixed64<8>>(Op::mul, sign * mul_a, mul_b, 1, sign < 0);
        boundary<Fixed64<8>>(Op::div, sign * div_a, div_b, 1, sign < 0);
        boundary<Fixed64<8>>(Op::mul_div, sign * md_a, 3, 2, sign < 0);
        boundary<Fixed64<8>>(Op::mul_div, sign * md_a, 3, -2, sign > 0);
        boundary<Fixed128<8>>(Op::mul, sign * mul_a, mul_b, 1, sign < 0);
        boundary<Fixed128<8>>(Op::div, sign * div_a, div_b, 1, sign < 0);
        boundary<Fixed128<8>>(Op::mul_div, sign * md_a, 3, 2, sign < 0);
        boundary<Fixed128<8>>(Op::mul_div, sign * md_a, 3, -2, sign > 0);

        // The compatibility adapter has its own rounding tail, including ceil
        // and nearest-away. Its fractional remainder is 514159080386 / 10^12.
        auto a = raw<Fixed64<12>>(runtime(sign * 9'223'372'036'725'648'599LL));
        auto b = raw<Fixed64<12>>(runtime(1'000'000'000'014LL));
        check_boundary<Fixed128<12>>([&](R mode) { return mul_wide(a, b, mode); }, sign < 0);

        // 10^19 is positive but is NOT a signed-64 divisor. Both operands still
        // fit int64_t, which used to select the invalid signed-IDIV path.
        auto x = raw<Fixed128<19>>(runtime(sign * 1'000'000'000'000'000'000LL));
        auto y = raw<Fixed128<19>>(runtime(1'000'000'000'000'000'000LL));
        for (R mode : modes) {
            auto result = mul(x, y, mode);
            CHECK(result && result->raw() == wide::int128(sign * 100'000'000'000'000'000LL));
        }
    }

    // No spurious failure at exact limits, zero, or a negative result rounding
    // to zero. Keep negative divisor and signed-min handling covered too.
    auto min = raw<Fixed64<8>>(runtime(INT64_MIN));
    auto max = raw<Fixed64<8>>(runtime(INT64_MAX));
    auto one = raw<Fixed64<8>>(runtime(100'000'000));
    for (R mode : modes) {
        CHECK(mul(min, one, mode) == min);
        CHECK(mul(max, one, mode) == max);
        CHECK(div(min, one, mode) == min);
        CHECK(div(max, one, mode) == max);
        auto zero_divisor = div(one, raw<Fixed64<8>>(runtime(0)), mode);
        CHECK(!zero_divisor && zero_divisor.error() == E::division_by_zero);
    }
    auto tiny = raw<Fixed128<8>>(runtime(-1));
    CHECK(mul(tiny, raw<Fixed128<8>>(runtime(1)), R::nearest_even) == Fixed128<8>{});
    CHECK(mul(tiny, raw<Fixed128<8>>(runtime(1)), R::toward_zero) == Fixed128<8>{});
}

#if defined(__SIZEOF_INT128__) && !defined(_WIN32)
// Independent rational oracle for RAW 64-bit operands: all products fit signed
// 128 bits. It does not use fixedwide's arithmetic, rounding, or division helpers.
// Windows (including clang-cl, which can expose __int128 without the division
// runtime) and the no-int128 job still execute all literal regressions above.
using I = __int128;
using U = unsigned __int128;

template<std::size_t Bits>
std::expected<I, E> rational(I numerator, I denominator, R mode) {
    if (denominator == 0) return std::unexpected(E::division_by_zero);
    I q = numerator / denominator;
    I r = numerator % denominator;
    auto fits = [](I value) {
        if constexpr (Bits == 64) return value >= I{INT64_MIN} && value <= I{INT64_MAX};
        return true; // The oracle's input domain cannot overflow signed 128 bits.
    };
    if (!fits(q)) return std::unexpected(E::overflow);
    if (r != 0) {
        if (mode == R::exact) return std::unexpected(E::inexact);
        const bool negative = (numerator < 0) != (denominator < 0);
        const U rem = static_cast<U>(r < 0 ? -r : r);
        const U den = static_cast<U>(denominator < 0 ? -denominator : denominator);
        // den-rem avoids overflowing when testing a half-way case.
        const bool above_half = rem > den - rem;
        const bool tie = rem == den - rem;
        bool increment = mode == R::floor ? negative : mode == R::ceil ? !negative : false;
        if (mode == R::nearest_even) increment = above_half || (tie && q % 2 != 0);
        if (mode == R::nearest_away) increment = above_half || tie;
        if (increment) q += negative ? -1 : 1;
    }
    if (!fits(q)) return std::unexpected(E::overflow);
    return q;
}

template<class F>
void compare(const std::expected<F, E>& actual, const std::expected<I, E>& expected) {
    CHECK(actual.has_value() == expected.has_value());
    if (!expected) {
        CHECK(actual.error() == expected.error());
    } else if constexpr (F::bits == 64) {
        CHECK(I{actual->raw()} == *expected);
    } else {
        const U bits = static_cast<U>(*expected);
        CHECK(actual->raw().low == static_cast<std::uint64_t>(bits));
        CHECK(actual->raw().high == static_cast<std::uint64_t>(bits >> 64));
    }
}

std::uint64_t next(std::uint64_t& state) {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

template<class F>
void differential() {
    I scale_value = 1;
    for (unsigned d = 0; d < F::fractional_digits; ++d) scale_value *= 10;
    const std::int64_t edges[] = {INT64_MIN, INT64_MIN + 1, -INT64_MAX, -3, -2, -1, 0, 1, 2, 3,
                                  INT64_MAX - 1, INT64_MAX, mul_a - 1, mul_a, mul_a + 1,
                                  mul_b, div_a, div_b, md_a};
    auto exercise = [&](std::int64_t a, std::int64_t b, std::int64_t c) {
        for (R mode : modes) {
            const auto x = raw<F>(a), y = raw<F>(b), z = raw<F>(c);
            compare(mul(x, y, mode), rational<F::bits>(I{a} * I{b}, scale_value, mode));
            compare(mul_div(x, y, z, mode), rational<F::bits>(I{a} * I{b}, I{c}, mode));
            if constexpr (F::fractional_digits <= 18) {
                compare(div(x, y, mode), rational<F::bits>(I{a} * scale_value, I{b}, mode));
            }
        }
    };
    for (auto a : edges) for (auto b : edges) {
        // Small/even/odd/negative divisors exercise the conservative IDIV guard.
        for (std::int64_t c : {std::int64_t{2}, std::int64_t{3}, std::int64_t{-2}, INT64_MIN})
            exercise(runtime(a), runtime(b), runtime(c));
    }
    if constexpr (F::fractional_digits >= 1 && F::fractional_digits <= 18) {
        for (I offset : {I{1}, I{2}, I{7}}) {
            const I b = scale_value + offset;
            const I boundary_a = I{INT64_MAX} * scale_value / b;
            for (int delta = -4; delta <= 4; ++delta) {
                const I a = boundary_a + delta;
                if (a < 0 || a > I{INT64_MAX}) continue;
                for (std::int64_t sign : {std::int64_t{-1}, std::int64_t{1}})
                    exercise(runtime(sign * static_cast<std::int64_t>(a)),
                             runtime(static_cast<std::int64_t>(b)),
                             runtime(static_cast<std::int64_t>(scale_value)));
            }
        }
    }
    std::uint64_t state = 0x71a9'f034'91d5'7c23ULL;
    for (unsigned i = 0; i < 512; ++i) {
        auto a = static_cast<std::int64_t>(next(state));
        auto b = static_cast<std::int64_t>(next(state));
        auto c = static_cast<std::int64_t>(next(state));
        exercise(a, b, c);
    }
}
#endif
} // namespace

int main() {
    rounding_comparison();
    regressions();
#if defined(__SIZEOF_INT128__) && !defined(_WIN32)
    differential<Fixed64<0>>();
    differential<Fixed64<1>>();
    differential<Fixed64<8>>();
    differential<Fixed64<12>>();
    differential<Fixed64<18>>();
    differential<Fixed128<0>>();
    differential<Fixed128<8>>();
    differential<Fixed128<12>>();
    differential<Fixed128<18>>();
    differential<Fixed128<19>>();
    differential<Fixed128<20>>();
    differential<Fixed128<38>>();
#endif
    std::printf("audit_fastpath_rounding passed (%llu checks)\n", static_cast<unsigned long long>(checks));
}
