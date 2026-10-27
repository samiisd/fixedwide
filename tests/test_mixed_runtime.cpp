// Runtime-narrow arithmetic must agree with the general rational kernels,
// including the error and its precedence. Use portable raw construction so
// this same suite also validates the forced-portable build.
#include <fixedwide/mixed.hpp>
#include "check.hpp"
#include <array>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

using namespace fixedwide;

namespace {

using I128 = wide::int128;
using I256 = wide::int256;
using Money = Fixed128<12>;
using WideMoney = Fixed256<12>;
using KernelResult = std::expected<I256, ArithmeticError>;

constexpr std::array modes{Rounding::toward_zero,  Rounding::floor,        Rounding::ceil,
                           Rounding::nearest_even, Rounding::nearest_away, Rounding::exact};

template<typename Dest>
void same(const std::expected<Dest, ArithmeticError>& got, const KernelResult& reference) {
    CHECK(got.has_value() == reference.has_value());
    if (reference) {
        CHECK(detail::to_int256_raw(got->raw()) == *reference);
    } else {
        CHECK(got.error() == reference.error());
    }
}

template<typename Dest, typename A>
void check_cast(A a, Rounding mode) {
    same(fixed_cast<Dest>(a, mode), detail::mixed_cast_kernel(detail::to_int256_raw(a.raw()), A::fractional_digits,
                                                              Dest::fractional_digits, mode, Dest::bits));
}

template<typename Dest, typename A, typename B>
void check_pair(A a, B b) {
    const auto ar = detail::to_int256_raw(a.raw());
    const auto br = detail::to_int256_raw(b.raw());
    for (auto mode : modes) {
        same(add_to<Dest>(a, b, mode), detail::mixed_add_sub_kernel(ar, A::fractional_digits, br, B::fractional_digits,
                                                                    false, Dest::fractional_digits, mode, Dest::bits));
        same(sub_to<Dest>(a, b, mode), detail::mixed_add_sub_kernel(ar, A::fractional_digits, br, B::fractional_digits,
                                                                    true, Dest::fractional_digits, mode, Dest::bits));
        same(mul_to<Dest>(a, b, mode), detail::mixed_mul_kernel(ar, A::fractional_digits, br, B::fractional_digits,
                                                                Dest::fractional_digits, mode, Dest::bits));
        check_cast<Dest>(a, mode);
    }
}

template<typename T>
T small(std::int64_t raw) {
    return T::from_raw(static_cast<typename T::raw_type>(raw));
}

template<typename Dest, typename A, typename B>
void small_grid() {
    for (std::int64_t a = -16; a <= 16; ++a) {
        for (std::int64_t b = -16; b <= 16; ++b) check_pair<Dest>(small<A>(a), small<B>(b));
    }
}

std::vector<I128> boundary_values() {
    std::vector<I128> out{
        I128{0}, I128{1}, I128{-1}, I128::min(), I128::max(), I128::min() + I128{1}, I128::max() - I128{1}};
    // 63/64 cover conversion and one/two-limb division. 126/127 cover signed
    // product, alignment, sum, and sign-extension boundaries.
    for (unsigned bit : {63u, 64u, 126u}) {
        const I128 power = I128{1} << bit;
        for (int delta : {-1, 0, 1}) {
            const auto value = power + I128{delta};
            out.push_back(value);
            out.push_back(-value);
        }
    }
    return out;
}

void test_boundaries() {
    const auto values = boundary_values();
    for (auto a : values) {
        for (auto b : values) {
            check_pair<Money>(Money::from_raw(a), Money::from_raw(b));
            check_pair<Fixed128<0>>(Fixed128<0>::from_raw(a), Fixed128<0>::from_raw(b));
            check_pair<Fixed128<18>>(Fixed128<12>::from_raw(a), Fixed128<6>::from_raw(b));
            check_pair<Fixed128<20>>(Fixed128<12>::from_raw(a), Fixed128<6>::from_raw(b));
            check_pair<Fixed64<10>>(Money::from_raw(a), Fixed128<10>::from_raw(b));
            check_pair<WideMoney>(Money::from_raw(a), Money::from_raw(b));
        }
        for (auto b :
             {INT64_MIN, INT64_MIN + 1, std::int64_t{-1}, std::int64_t{0}, std::int64_t{1}, INT64_MAX - 1, INT64_MAX}) {
            check_pair<Money>(Money::from_raw(a), Fixed64<10>::from_raw(b));
            check_pair<Money>(Fixed64<10>::from_raw(b), Money::from_raw(a));
            check_pair<WideMoney>(Fixed128<10>::from_raw(a), Fixed64<12>::from_raw(b));
        }
    }

    // An int128 intermediate overflows, but rescaling makes the answer fit.
    CHECK(mul_to<Fixed128<0>>(Fixed128<1>::max(), small<Fixed128<0>>(2), Rounding::toward_zero));
    CHECK(add_to<Fixed128<0>>(Fixed128<1>::max(), Fixed128<1>::max(), Rounding::toward_zero));
    CHECK(sub_to<Fixed128<0>>(Fixed128<1>::min(), Fixed128<1>::max(), Rounding::toward_zero));

    // Alignment fails before adding a cancelling operand; it is not a result
    // overflow. The mathematical result is three raw units at scale 1.
    const I128 coarse = static_cast<I128>(static_cast<wide::uint128>(I128::max()) / wide::uint128{10}) + I128{1};
    check_pair<Fixed128<1>>(Fixed128<0>::from_raw(coarse), Fixed128<1>::from_raw(-I128::max()));
    const auto cancelled = add_to<Fixed128<1>>(Fixed128<0>::from_raw(coarse), Fixed128<1>::from_raw(-I128::max()));
    CHECK(cancelled);
    CHECK(cancelled->raw() == I128{3});
}

void test_rounding_and_precedence() {
    // Exercise exact ties and either adjacent remainder, both signs and both
    // quotient parities. This reaches all six modes in each public operation.
    for (std::int64_t q = -4; q <= 4; ++q) {
        for (std::int64_t delta = -1; delta <= 1; ++delta) {
            const auto a = small<Fixed128<1>>(q * 10 + 5 + delta);
            check_pair<Fixed128<0>>(a, small<Fixed128<0>>(1));
            check_pair<Fixed128<0>>(small<Fixed128<0>>(0), a);
            for (auto mode : modes) check_cast<Fixed128<0>>(Fixed256<1>{a}, mode);
        }
    }

    // A 20-digit divisor uses the multi-limb/native-compiler division route.
    // Preserve both quotient parity and the exact remainder at its ties too.
    const I128 divisor = Fixed128<20>::scale();
    for (int q = -4; q <= 4; ++q) {
        for (int delta = -1; delta <= 1; ++delta) {
            const I128 raw = I128{q} * divisor + (divisor >> 1) + I128{delta};
            const auto a = Fixed128<20>::from_raw(raw);
            check_pair<Fixed128<0>>(a, small<Fixed128<0>>(1));
            check_pair<Fixed128<0>>(small<Fixed128<0>>(0), a);
            for (auto mode : modes) check_cast<Fixed128<0>>(Fixed256<20>{a}, mode);
        }
    }

    // Truncated quotient already outside the destination must report overflow
    // even for exact mode. At the endpoint, an unrepresentable fraction is
    // inexact for exact mode, and may overflow for a rounding mode.
    for (auto raw : {std::int64_t{1271}, std::int64_t{1275}, std::int64_t{1279}, std::int64_t{1281},
                     std::int64_t{-1281}, std::int64_t{-1285}, std::int64_t{-1289}, std::int64_t{-1291}}) {
        check_pair<Fixed8<0>>(small<Fixed128<1>>(raw), small<Fixed128<0>>(1));
        check_pair<Fixed8<0>>(small<Fixed128<0>>(0), small<Fixed128<1>>(raw));
    }
    CHECK(fixed_cast<Fixed8<0>>(small<Fixed128<1>>(1281), Rounding::exact).error() == ArithmeticError::overflow);
    CHECK(fixed_cast<Fixed8<0>>(small<Fixed128<1>>(1271), Rounding::exact).error() == ArithmeticError::inexact);
    CHECK(fixed_cast<Fixed8<0>>(small<Fixed128<1>>(1275), Rounding::nearest_even).error() == ArithmeticError::overflow);
    CHECK(fixed_cast<Fixed8<0>>(small<Fixed128<1>>(-1281), Rounding::floor).error() == ArithmeticError::overflow);

    for (auto endpoint : {I128{INT64_MIN}, I128{INT64_MAX}}) {
        for (int offset = -11; offset <= 11; ++offset) {
            const auto a = Fixed128<1>::from_raw(endpoint * I128{10} + I128{offset});
            check_pair<Fixed64<0>>(a, small<Fixed128<0>>(1));
            check_pair<Fixed64<0>>(a, small<Fixed128<1>>(0));
            for (auto mode : modes) check_cast<Fixed64<0>>(Fixed256<1>{a}, mode);
        }
    }
}

void test_extreme_scales() {
    // Decimal powers on either side of 64-bit and 128-bit representability.
    // Downscaling by 10^39 or 10^76 must not instantiate a native power that
    // wraps. Wide destination scale-up must retain all its high limbs.
    for (auto a : {I128{0}, I128{1}, I128{-1}, I128::min(), I128::max()}) {
        for (auto b : {I128{0}, I128{1}, I128{-1}, I128::min(), I128::max()}) {
            check_pair<Fixed128<0>>(Fixed128<19>::from_raw(a), Fixed128<19>::from_raw(b));
            check_pair<Fixed128<0>>(Fixed128<19>::from_raw(a), Fixed128<20>::from_raw(b));
            check_pair<Fixed128<0>>(Fixed128<38>::from_raw(a), Fixed128<38>::from_raw(b));
            check_pair<Fixed128<38>>(Fixed128<0>::from_raw(a), Fixed128<0>::from_raw(b));
            check_pair<Fixed256<76>>(Fixed128<0>::from_raw(a), Fixed128<0>::from_raw(b));
            check_pair<Fixed256<38>>(Fixed128<0>::from_raw(a), Fixed128<38>::from_raw(b));
            check_pair<Fixed256<0>>(Fixed128<0>::from_raw(a), Fixed128<38>::from_raw(b));
        }
        for (auto mode : modes) {
            check_cast<Fixed128<0>>(Fixed256<39>::from_raw(I256{a}), mode);
            check_cast<Fixed128<38>>(Fixed256<0>::from_raw(I256{a}), mode);
            check_cast<Fixed128<0>>(Fixed256<76>::from_raw(I256{a}), mode);
        }
    }
}

void test_wide_values() {
    const I256 p127{0, 0x8000'0000'0000'0000ULL, 0, 0};
    const I256 p128{0, 0, 1, 0};
    const std::array values{I256{0},
                            I256{1},
                            I256{-1},
                            I256{I128::min()},
                            I256{I128::max()},
                            p127,
                            -p127 - I256{1},
                            p128,
                            -p128,
                            I256{0, 0, 0, 1},
                            I256{~0ULL, ~0ULL, ~0ULL, 0},
                            I256{0, 0x7FFF'FFFF'FFFF'FFFFULL, ~0ULL, ~0ULL},
                            I256::min(),
                            I256::max()};
    for (auto a : values) {
        for (auto mode : modes) {
            check_cast<Money>(WideMoney::from_raw(a), mode);
            check_cast<Fixed64<10>>(WideMoney::from_raw(a), mode);
            check_cast<Fixed128<0>>(Fixed256<1>::from_raw(a), mode);
            check_cast<Fixed128<0>>(Fixed256<38>::from_raw(a), mode);
            check_cast<Fixed128<38>>(Fixed256<0>::from_raw(a), mode);
        }
        for (auto b : values) {
            check_pair<WideMoney>(WideMoney::from_raw(a), WideMoney::from_raw(b));
            check_pair<Money>(WideMoney::from_raw(a), WideMoney::from_raw(b));
            check_pair<Fixed64<12>>(WideMoney::from_raw(a), WideMoney::from_raw(b));
            check_pair<Fixed8<0>>(Fixed256<0>::from_raw(a), Fixed256<0>::from_raw(b));
        }
        for (auto b : {I128::min(), I128{-1}, I128{0}, I128{1}, I128::max()}) {
            check_pair<WideMoney>(WideMoney::from_raw(a), Money::from_raw(b));
            check_pair<WideMoney>(Money::from_raw(b), WideMoney::from_raw(a));
            check_pair<Money>(WideMoney::from_raw(a), Money::from_raw(b));
            check_pair<Money>(Money::from_raw(b), WideMoney::from_raw(a));
        }
    }
    CHECK(fixed_cast<Money>(WideMoney::from_raw(p127), Rounding::exact).error() == ArithmeticError::overflow);
    CHECK(fixed_cast<Money>(WideMoney::from_raw(-p127 - I256{1}), Rounding::exact).error() ==
          ArithmeticError::overflow);

    // True wide operands can cancel into a narrow destination. Check the
    // result's range only after the full sum/difference, at narrow endpoints
    // and just outside them, for both operand orders and every rounding mode.
    for (auto endpoint :
         {I256{-129}, I256{-128}, I256{-127}, I256{127}, I256{128}, I256{129}, I256{INT64_MIN} - I256{1},
          I256{INT64_MIN}, I256{INT64_MAX}, I256{INT64_MAX} + I256{1}, I256{I128::min()} - I256{1}, I256{I128::min()},
          I256{I128::max()}, I256{I128::max()} + I256{1}}) {
        const I256 right = -p128 + endpoint;
        check_pair<Money>(WideMoney::from_raw(p128), WideMoney::from_raw(right));
        check_pair<Money>(WideMoney::from_raw(right), WideMoney::from_raw(p128));
        check_pair<Fixed64<0>>(Fixed256<0>::from_raw(p128), Fixed256<0>::from_raw(right));
        check_pair<Fixed8<0>>(Fixed256<0>::from_raw(p128), Fixed256<0>::from_raw(right));
    }
    CHECK(add_to<Money>(WideMoney::max(), WideMoney::min()) == small<Money>(-1));
    CHECK(sub_to<Money>(WideMoney::min(), WideMoney::min()) == Money{});
    // A representable int256 result can overflow int128, and overflowing the
    // int256 arithmetic itself must not be mistaken for narrow cancellation.
    CHECK(add_to<Money>(WideMoney{Money::max()}, WideMoney{Money::max()}).error() == ArithmeticError::overflow);
    CHECK(add_to<Money>(WideMoney::max(), WideMoney::max()).error() == ArithmeticError::overflow);
    CHECK(sub_to<Money>(WideMoney::min(), WideMoney::max()).error() == ArithmeticError::overflow);
}

void check_replace(Money total, Money previous, Money next) {
    const auto removed = sub_to<WideMoney>(total, previous, Rounding::exact);
    const auto removed_reference =
        detail::mixed_add_sub_kernel(I256{total.raw()}, 12, I256{previous.raw()}, 12, true, 12, Rounding::exact, 256);
    same(removed, removed_reference);
    CHECK(removed);
    const auto replaced = add_to<WideMoney>(*removed, next, Rounding::exact);
    const auto replaced_reference =
        detail::mixed_add_sub_kernel(*removed_reference, 12, I256{next.raw()}, 12, false, 12, Rounding::exact, 256);
    same(replaced, replaced_reference);
    CHECK(replaced);
    same(fixed_cast<Money>(*replaced, Rounding::exact),
         detail::mixed_cast_kernel(*replaced_reference, 12, 12, Rounding::exact, 128));
}

void test_replacement() {
    for (auto t : {I128::min(), I128{-1}, I128{0}, I128{1}, I128::max()}) {
        for (auto p : {I128::min(), I128{-1}, I128{0}, I128{1}, I128::max()}) {
            for (auto n : {I128::min(), I128{-1}, I128{0}, I128{1}, I128::max()}) {
                check_replace(Money::from_raw(t), Money::from_raw(p), Money::from_raw(n));
            }
        }
    }
    // Both directions overflow the first int128 operation, then cancel back
    // into range. The wide intermediate must survive until the final cast.
    const auto high = sub_to<WideMoney>(Money::max(), Money::min(), Rounding::exact);
    const auto low = sub_to<WideMoney>(Money::min(), Money::max(), Rounding::exact);
    CHECK(high && low);
    CHECK(!fixed_cast<Money>(*high));
    CHECK(!fixed_cast<Money>(*low));
    const auto restored_high = add_to<WideMoney>(*high, Money::min());
    const auto restored_low = add_to<WideMoney>(*low, Money::max());
    CHECK(restored_high && restored_low);
    CHECK(fixed_cast<Money>(*restored_high) == Money::max());
    CHECK(fixed_cast<Money>(*restored_low) == Money::min());
}

void test_native_minimum_division() {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    const auto minimum = static_cast<__int128>(I128::min());
    for (auto mode : modes) {
        const auto result = detail::mixed_native::divide_rounded(minimum, 1, mode,
                                                                 detail::mixed_native::limit_magnitude_u128<128>(true));
        CHECK(result);
        CHECK(*result == minimum);
    }
#endif
}

void test_random() {
    std::mt19937_64 rng{0x6'10'128};
    for (unsigned i = 0; i < 1024; ++i) {
        const I128 full_a{rng(), rng()};
        const I128 full_b{rng(), rng()};
        const I128 full_c{rng(), rng()};
        // Full-width draws force product/alignment fallbacks; narrow signed
        // draws exercise successful native paths, including the upper limb.
        I128 narrow_a{rng(), rng() & 7u};
        I128 narrow_b{rng() & 0xFFFF'FFFF'FFFFULL, 0};
        if ((rng() & 1u) != 0) narrow_a = -narrow_a;
        if ((rng() & 1u) != 0) narrow_b = -narrow_b;
        for (auto pair : {std::array{full_a, full_b}, std::array{narrow_a, narrow_b}}) {
            check_pair<Money>(Money::from_raw(pair[0]), Money::from_raw(pair[1]));
            check_pair<Money>(Money::from_raw(pair[0]), Fixed64<10>::from_raw(static_cast<std::int64_t>(pair[1])));
            check_pair<Money>(Fixed128<8>::from_raw(pair[0]), Fixed128<12>::from_raw(pair[1]));
            check_pair<Fixed64<10>>(Fixed128<18>::from_raw(pair[0]), Fixed128<12>::from_raw(pair[1]));
            check_pair<WideMoney>(Money::from_raw(pair[0]), Money::from_raw(pair[1]));
            check_replace(Money::from_raw(pair[0]), Money::from_raw(pair[1]), Money::from_raw(full_c));
        }
        const I256 wide_raw{rng(), rng(), rng(), rng()};
        for (auto mode : modes) {
            check_cast<Money>(WideMoney::from_raw(wide_raw), mode);
            check_cast<Money>(Fixed256<38>::from_raw(wide_raw), mode);
            check_cast<Money>(WideMoney::from_raw(I256{narrow_a}), mode);
        }
        check_pair<WideMoney>(WideMoney::from_raw(wide_raw), Money::from_raw(full_b));
    }
}

} // namespace

int main() {
    small_grid<Fixed128<1>, Fixed128<1>, Fixed128<1>>();
    small_grid<Fixed128<0>, Fixed128<1>, Fixed64<0>>();
    small_grid<Fixed128<3>, Fixed128<1>, Fixed64<1>>();
    small_grid<Fixed128<0>, Fixed128<1>, Fixed128<2>>();
    small_grid<Fixed8<0>, Fixed128<1>, Fixed128<0>>();
    small_grid<Fixed256<1>, Fixed128<1>, Fixed128<1>>();
    test_boundaries();
    test_rounding_and_precedence();
    test_extreme_scales();
    test_wide_values();
    test_replacement();
    test_native_minimum_division();
    test_random();
    std::printf("test_mixed_runtime passed (%llu checks)\n", static_cast<unsigned long long>(checks));
}
