// The mixed-scale fast paths against the general kernel.
//
// The narrow paths in detail/mixed_native.hpp exist because the general kernel
// evaluates every mixed operation in 1024-bit limbs, which measured 70x to 760x
// slower than the arithmetic actually required. They are only safe if they
// return exactly what the kernel returns -- same value, same error, every
// rounding mode. This test calls both and compares.
//
// It deliberately includes operand values chosen to sit on the fast path's
// compile-time bounds, and destination types the bounds must reject.
#include <fixedwide/mixed.hpp>
#include <fixedwide/arithmetic.hpp>
#include "check.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <random>
#include <vector>

using fixedwide::Rounding;
namespace detail = fixedwide::detail;

namespace {

constexpr Rounding all_modes[] = {
    Rounding::toward_zero,  Rounding::floor,        Rounding::ceil,
    Rounding::nearest_even, Rounding::nearest_away, Rounding::exact,
};

template<typename Dest, typename Got>
void same(const Got& fast, const std::expected<fixedwide::wide::int256, fixedwide::ArithmeticError>& general,
          const char* what) {
    (void)what;
    if (!general) {
        CHECK(!fast.has_value());
        CHECK(fast.error() == general.error());
        return;
    }
    CHECK(fast.has_value());
    CHECK(fast->raw() == detail::from_int256_raw<Dest>(*general).raw());
}

// Exercise one (source types -> destination) combination over every mode.
template<typename Dest, typename A, typename B>
void compare_pair(A a, B b) {
    for (auto mode : all_modes) {
        same<Dest>(fixedwide::add_to<Dest>(a, b, mode),
                   detail::mixed_add_sub_kernel(detail::to_int256_raw(a.raw()), A::fractional_digits,
                                                detail::to_int256_raw(b.raw()), B::fractional_digits, false,
                                                Dest::fractional_digits, mode, Dest::bits),
                   "add_to");
        same<Dest>(fixedwide::sub_to<Dest>(a, b, mode),
                   detail::mixed_add_sub_kernel(detail::to_int256_raw(a.raw()), A::fractional_digits,
                                                detail::to_int256_raw(b.raw()), B::fractional_digits, true,
                                                Dest::fractional_digits, mode, Dest::bits),
                   "sub_to");
        same<Dest>(fixedwide::mul_to<Dest>(a, b, mode),
                   detail::mixed_mul_kernel(detail::to_int256_raw(a.raw()), A::fractional_digits,
                                            detail::to_int256_raw(b.raw()), B::fractional_digits,
                                            Dest::fractional_digits, mode, Dest::bits),
                   "mul_to");
        same<Dest>(fixedwide::div_to<Dest>(a, b, mode),
                   detail::mixed_div_kernel(detail::to_int256_raw(a.raw()), A::fractional_digits,
                                            detail::to_int256_raw(b.raw()), B::fractional_digits,
                                            Dest::fractional_digits, mode, Dest::bits),
                   "div_to");
        same<Dest>(fixedwide::mul_div_to<Dest>(a, b, b, mode),
                   detail::mixed_mul_div_kernel(detail::to_int256_raw(a.raw()), A::fractional_digits,
                                                detail::to_int256_raw(b.raw()), B::fractional_digits,
                                                detail::to_int256_raw(b.raw()), B::fractional_digits,
                                                Dest::fractional_digits, mode, Dest::bits),
                   "mul_div_to");
        same<Dest>(fixedwide::fixed_cast<Dest>(a, mode),
                   detail::mixed_cast_kernel(detail::to_int256_raw(a.raw()), A::fractional_digits,
                                             Dest::fractional_digits, mode, Dest::bits),
                   "fixed_cast");
    }
    // Comparison must match the kernel exactly, in both directions.
    const auto general = detail::mixed_compare_kernel(detail::to_int256_raw(a.raw()), A::fractional_digits,
                                                      detail::to_int256_raw(b.raw()), B::fractional_digits);
    CHECK((a <=> b) == general);
    CHECK((a == b) == (general == std::strong_ordering::equal));
}

template<typename Dest, typename A, typename B, typename RawA, typename RawB>
void sweep(const char* label, const std::vector<RawA>& xs, const std::vector<RawB>& ys) {
    (void)label;
    for (auto x : xs) {
        for (auto y : ys) compare_pair<Dest>(A::from_raw(x), B::from_raw(y));
    }
}

template<class Dest, class A, class B>
void multiply_pair(A a, B b) {
    for (auto mode : all_modes) {
        const auto general = detail::mixed_mul_kernel(detail::to_int256_raw(a.raw()), A::fractional_digits,
                                                      detail::to_int256_raw(b.raw()), B::fractional_digits,
                                                      Dest::fractional_digits, mode, Dest::bits);
        same<Dest>(fixedwide::mul_to<Dest>(a, b, mode), general, "exact quotient guard");
        same<Dest>(fixedwide::mul_to<Dest>(b, a, mode), general, "symmetric exact quotient guard");
    }
}

template<unsigned Gap>
void exact_scaled_products() {
    using A = fixedwide::Fixed128<Gap>;
    using B = fixedwide::Fixed64<0>;
    using Dest = fixedwide::Fixed128<0>;
    using Raw = fixedwide::wide::int128;
    constexpr auto divisor = A::scale();
    const std::array quotients{
        Raw{0}, Raw{1}, Raw{-1}, Raw{INT64_MIN}, Raw{INT64_MAX}, Raw{INT64_MIN} - Raw{1}, Raw{INT64_MAX} + Raw{1}};
    constexpr std::array factors{std::int64_t{0},   std::int64_t{1}, std::int64_t{-1}, std::int64_t{2},
                                 std::int64_t{-37}, INT64_MIN,       INT64_MAX};
    for (auto quotient : quotients) {
        const auto exact = quotient * divisor;
        for (int offset : {-1, 0, 1})
            for (auto factor : factors) {
                const auto a = A::from_raw(exact + Raw{offset});
                const auto b = B::from_raw(factor);
                multiply_pair<Dest>(a, b);
                // Valid reconstructed quotients can still overflow a narrower
                // destination; inexact misses must keep overflow precedence.
                multiply_pair<fixedwide::Fixed8<0>>(a, b);
            }
    }
    std::mt19937_64 rng(0xE7AC7000 + Gap);
    for (unsigned i = 0; i < 400; ++i) {
        const auto factor = B::from_raw(std::bit_cast<std::int64_t>(rng()));
        multiply_pair<Dest>(A::from_raw(Raw{rng(), rng()}), factor);
        const auto quotient = Raw{std::bit_cast<std::int64_t>(rng())};
        multiply_pair<Dest>(A::from_raw(quotient * divisor), factor);
    }
}

void check_exact_multiplication() {
    exact_scaled_products<1>();
    exact_scaled_products<10>();
    exact_scaled_products<19>();
    using Money = fixedwide::Fixed128<12>;
    using Price = fixedwide::Fixed64<10>;
    using Raw = fixedwide::wide::int128;
    constexpr std::int64_t scale = 10'000'000'000;
    constexpr auto exact = Rounding::exact;
    // The numerator alone is inexact, but factors from the other operand
    // cancel the denominator. A failed reconstruction cannot report inexact.
    for (auto a : {Raw{1}, Raw{-1}, Raw{scale / 2}, Raw{-scale / 2}})
        for (auto b : {std::int64_t{2}, std::int64_t{-2}, scale, -scale})
            multiply_pair<Money>(Money::from_raw(a), Price::from_raw(b));
    const auto cancelled = fixedwide::mul_to<Money>(Money::from_raw(scale / 2), Price::from_raw(2), exact);
    CHECK(cancelled && cancelled->raw() == Raw{1});
    // Signed minima require no negation in the successful quotient path, and
    // the full signed128 minimum must continue to work through the fallback.
    for (auto money : {Money::min(), Money::max()})
        for (auto factor : {Price::min(), Price::max(), Price{}, Price::from_raw(scale), Price::from_raw(-scale)})
            multiply_pair<Money>(money, factor);
    const auto minimum = fixedwide::mul_to<Money>(Money::min(), Price::from_raw(scale), exact);
    CHECK(minimum && *minimum == Money::min());
    const auto negated_minimum = fixedwide::mul_to<Money>(Money::min(), Price::from_raw(-scale), exact);
    CHECK(!negated_minimum && negated_minimum.error() == fixedwide::ArithmeticError::overflow);

    using Narrow = fixedwide::Fixed8<0>;
    using Scaled = fixedwide::Fixed128<10>;
    using Integer = fixedwide::Fixed64<0>;
    const auto overflow =
        fixedwide::mul_to<Narrow>(Scaled::from_raw(Raw{128} * Raw{scale} + Raw{1}), Integer::from_raw(1), exact);
    CHECK(!overflow && overflow.error() == fixedwide::ArithmeticError::overflow);
    const auto inexact =
        fixedwide::mul_to<Narrow>(Scaled::from_raw(Raw{127} * Raw{scale} + Raw{1}), Integer::from_raw(1), exact);
    CHECK(!inexact && inexact.error() == fixedwide::ArithmeticError::inexact);
    // An excluded scale gap keeps the same general path and rounding policy.
    multiply_pair<fixedwide::Fixed128<0>>(fixedwide::Fixed128<20>::from_raw(Raw{1}), Integer::from_raw(7));
    const auto simple = fixedwide::mul_to<Money>(Money::from_raw(Raw{7} * Raw{scale}), Price::from_raw(-37), exact);
    CHECK(simple && simple->raw() == Raw{-259});
}

} // namespace

#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
template<std::uint64_t Divisor>
void check_exact_divisor() {
    using namespace detail::mixed_native;
    constexpr auto divisor = static_cast<i128>(Divisor);
    constexpr auto lo64 = static_cast<i128>(INT64_MIN), hi64 = static_cast<i128>(INT64_MAX);
    const auto verify = [](i128 value) {
        std::int64_t quotient;
        const bool accepted = exact_quotient_i64<Divisor>(value, quotient);
        // Ordinary signed128 division is independent of the inverse and the
        // reconstruction multiply used by the implementation.
        const auto reference = value / divisor;
        CHECK(accepted == (value % divisor == 0 && reference >= lo64 && reference <= hi64));
        if (accepted) CHECK(static_cast<i128>(quotient) == reference);
    };
    constexpr auto lo128 = std::bit_cast<i128>(u128{1} << 127);
    constexpr auto hi128 = std::bit_cast<i128>((u128{1} << 127) - 1);
    for (i128 quotient : {i128{0}, i128{1}, i128{-1}, lo64, hi64, lo64 - 1, hi64 + 1}) {
        if (quotient < lo128 / divisor || quotient > hi128 / divisor) continue;
        const auto product = quotient * divisor;
        for (int delta : {-1, 0, 1}) {
            if ((delta < 0 && product == lo128) || (delta > 0 && product == hi128)) continue;
            verify(product + delta);
        }
    }
    verify(std::bit_cast<i128>(u128{1} << 127));
    verify(std::bit_cast<i128>((u128{1} << 127) - 1));
    std::mt19937_64 rng(0xD171D3 + Divisor);
    for (unsigned i = 0; i < 1000; ++i) {
        verify(std::bit_cast<i128>((static_cast<u128>(rng()) << 64) | rng()));
        verify(static_cast<i128>(std::bit_cast<std::int64_t>(rng())) * divisor);
    }
}

template<std::size_t... Gap>
void check_exact_quotients(std::index_sequence<Gap...>) {
    (check_exact_divisor<static_cast<std::uint64_t>(detail::mixed_native::pow10_v<Gap + 1>)>(), ...);
    check_exact_divisor<1>();
    check_exact_divisor<UINT64_MAX>();
    static_assert([] {
        std::int64_t quotient;
        return detail::mixed_native::exact_quotient_i64<10'000'000'000>(-370'000'000'000, quotient) && quotient == -37;
    }());
    static_assert(!detail::mixed_native::try_mul<12, 10, 128, 12>(fixedwide::wide::int128{70'000'000'000LL},
                                                                  std::int64_t{-37}, Rounding::exact));
}

// divide_magnitude has a hardware path for a divisor that fits 64 bits, and
// divq faults rather than wraps when its quotient does not fit one limb. Sweep
// every magnitude and divisor width against the compiler's own division.
void check_divide_magnitude() {
    using u128 = unsigned __int128;
    std::mt19937_64 rng(0x0D17);
    for (unsigned magnitude_bits = 0; magnitude_bits <= 128; ++magnitude_bits) {
        for (unsigned divisor_bits = 1; divisor_bits <= 128; ++divisor_bits) {
            for (int repeat = 0; repeat < 4; ++repeat) {
                const auto mask = [](unsigned bits) { return bits >= 128 ? ~u128{0} : ((u128{1} << bits) - 1); };
                u128 magnitude = ((static_cast<u128>(rng()) << 64) | rng()) & mask(magnitude_bits);
                u128 divisor = (((static_cast<u128>(rng()) << 64) | rng()) & mask(divisor_bits)) | 1;
                const auto got = detail::mixed_native::divide_magnitude(magnitude, divisor);
                CHECK(got.quotient == magnitude / divisor);
                CHECK(got.remainder == magnitude % divisor);
            }
        }
    }
}
static_assert(detail::mixed_native::divide_magnitude(static_cast<unsigned __int128>(1'000'000),
                                                     static_cast<unsigned __int128>(7))
                  .quotient == 142857);
#else
void check_divide_magnitude() {}
#endif

int main() {
    check_divide_magnitude();
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    check_exact_quotients(std::make_index_sequence<19>{});
#endif
    check_exact_multiplication();
    using Price = fixedwide::Fixed64<8>;
    using Rate = fixedwide::Fixed64<12>;
    using Money = fixedwide::Fixed128<12>;
    using Small = fixedwide::Fixed32<6>;
    using Tiny = fixedwide::Fixed16<2>;

    std::mt19937_64 rng(0x31CED);

    std::vector<std::int64_t> wide_values{0,
                                          1,
                                          -1,
                                          2,
                                          -2,
                                          INT64_MAX,
                                          INT64_MIN,
                                          INT64_MAX - 1,
                                          INT64_MIN + 1,
                                          100'000'000LL,
                                          -100'000'000LL,
                                          1'000'000'000'000LL,
                                          -1'000'000'000'000LL};
    for (int i = 0; i < 14; ++i) wide_values.push_back(static_cast<std::int64_t>(rng()));

    std::vector<std::int32_t> narrow_values{0, 1, -1, INT32_MAX, INT32_MIN, 1'000'000, -1'000'000};
    for (int i = 0; i < 8; ++i) narrow_values.push_back(static_cast<std::int32_t>(rng()));

    std::vector<std::int16_t> tiny_values{0, 1, -1, INT16_MAX, INT16_MIN, 100, -100};

    // Combinations that take the fast path.
    sweep<Money, Price, Rate>("Money<-Price,Rate", wide_values, wide_values);
    sweep<Rate, Price, Small>("Rate<-Price,Small", wide_values, narrow_values);
    sweep<Price, Small, Tiny>("Price<-Small,Tiny", narrow_values, tiny_values);
    sweep<Small, Tiny, Tiny>("Small<-Tiny,Tiny", tiny_values, tiny_values);

    // Combinations the bounds must reject, so the kernel runs and still agrees.
    sweep<fixedwide::Fixed256<24>, Price, Rate>("F256<-Price,Rate", wide_values, wide_values);
    sweep<Money, Price, fixedwide::Fixed128<18>>(
        "Money<-Price,F128", wide_values,
        std::vector<fixedwide::wide::int128>{fixedwide::wide::int128(0), fixedwide::wide::int128(1),
                                             fixedwide::wide::int128(-1),
                                             fixedwide::wide::int128(1'000'000'000'000LL)});

    std::printf("test_mixed_native passed (%lu checks)\n", checks);
    return 0;
}
