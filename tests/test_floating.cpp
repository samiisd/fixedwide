#include "check.hpp"
#include <fixedwide/all.hpp>
#include <limits>
#include <cmath>
#include <array>
#include <bit>
#include <cfenv>
#include <cstdlib>
#include <random>

using namespace fixedwide;

void test_floating_conversions() {
    using F = Fixed64<4>;
    double d = 123.4567;
    auto f_res = from_float<F>(d);
    CHECK(f_res.has_value());
    CHECK(f_res->raw() == 1234567LL);

    double back = to_float<double>(*f_res);
    CHECK(std::abs(back - d) < 1e-6);

    // Float overload
    float flt = 12.5f;
    auto f_flt = from_float<Fixed32<2>>(flt);
    CHECK(f_flt.has_value() && f_flt->raw() == 1250);

    // NaN / Inf error check
    CHECK(!from_float<F>(std::numeric_limits<double>::quiet_NaN()));
    CHECK(!from_float<F>(std::numeric_limits<double>::infinity()));
    CHECK(!from_float<F>(-std::numeric_limits<double>::infinity()));
    auto nan_err = from_float<F>(std::numeric_limits<double>::quiet_NaN());
    CHECK(nan_err.error() == ArithmeticError::invalid_value);

    // Overflow check
    auto of_err = from_float<Fixed8<1>>(1000.0);
    CHECK(!of_err.has_value() && of_err.error() == ArithmeticError::overflow);
}

namespace {
constexpr std::array rounding_modes{Rounding::toward_zero, Rounding::floor, Rounding::ceil,
                                   Rounding::nearest_even, Rounding::nearest_away, Rounding::exact};
constexpr bool binary64 = sizeof(double) == sizeof(std::uint64_t) && std::numeric_limits<double>::is_iec559 &&
                          std::numeric_limits<double>::radix == 2 && std::numeric_limits<double>::digits == 53 &&
                          std::numeric_limits<double>::max_exponent == 1024;
static_assert(detail::use_fast_from_float_v<Fixed64<8>, double> == binary64);
static_assert(detail::use_fast_from_float_v<Fixed64<10>, double> == binary64);
static_assert(detail::use_fast_from_float_v<Fixed64<12>, double> == binary64);
static_assert(detail::use_fast_from_float_v<Fixed128<32>, double> == binary64);
static_assert(!detail::use_fast_from_float_v<Fixed128<33>, double>);
static_assert(!detail::use_fast_from_float_v<Fixed256<8>, double>);
static_assert(!detail::use_fast_from_float_v<Fixed64<8>, float>);
static_assert(!detail::use_fast_from_float_v<Fixed64<8>, long double>);
static_assert(noexcept(from_float<Fixed64<8>>(0.0)));
static_assert(noexcept(from_double<Fixed128<22>>(0.0)));
std::uint64_t differential_cases = 0;

template<typename Target>
void compare_kernel(double value) {
    for (Rounding rounding : rounding_modes) {
        const auto expected = detail::from_float_kernel(value, Target::fractional_digits, rounding, Target::bits);
        const auto actual = from_float<Target>(value, rounding);
        const bool equal = expected ? actual && detail::to_int256_raw(actual->raw()) == *expected
                                    : !actual && actual.error() == expected.error();
        if (!equal) {
            std::fprintf(stderr, "double conversion mismatch: width=%zu digits=%u value=%a rounding=%u\n", Target::bits,
                         Target::fractional_digits, value, static_cast<unsigned>(rounding));
        }
        CHECK(equal);
        CHECK(from_double<Target>(value, rounding) == actual);
        ++differential_cases;
    }
}

template<typename Target, typename Float = double>
void compare_bits(std::uint64_t bits) {
    if constexpr (detail::use_fast_from_float_v<Target, Float>) {
        compare_kernel<Target>(std::bit_cast<Float>(bits & 0x7fff'ffff'ffff'ffffULL));
        compare_kernel<Target>(std::bit_cast<Float>(bits | 0x8000'0000'0000'0000ULL));
    }
}

template<typename Target>
void check_ties() {
    // odd * 2^(-D-1) scales to odd * 5^D / 2 EXACTLY. 5^D is 1 mod 4,
    // so odd=1,5 gives an even quotient and odd=3,7 an odd quotient.
    wide::uint128 five{1};
    for (unsigned d = 0; d < Target::fractional_digits; ++d) five = five * wide::uint128{5};
    for (unsigned odd : {1u, 3u, 5u, 7u}) {
        const double tie = std::ldexp(static_cast<double>(odd), -static_cast<int>(Target::fractional_digits) - 1);
        const wide::uint128 q = (five * wide::uint128{odd}) >> 1;
        for (bool negative : {false, true}) {
            const double value = negative ? -tie : tie;
            compare_kernel<Target>(value);
            compare_kernel<Target>(std::nextafter(value, -std::numeric_limits<double>::infinity()));
            compare_kernel<Target>(std::nextafter(value, std::numeric_limits<double>::infinity()));
            for (Rounding rounding : {Rounding::nearest_even, Rounding::nearest_away}) {
                const auto rounded = q + wide::uint128{rounding == Rounding::nearest_away || (q.low & 1) != 0};
                const auto expected = detail::to_int256_raw(wide::int128(rounded.low, rounded.high));
                const auto actual = from_float<Target>(value, rounding);
                CHECK(actual && detail::to_int256_raw(actual->raw()) == (negative ? -expected : expected));
            }
            const auto exact = from_float<Target>(value, Rounding::exact);
            CHECK(!exact && exact.error() == ArithmeticError::inexact);
        }
    }
}

template<typename Target>
void differential_type(std::size_t samples) {
    // Includes both signed zeros, subnormals, the normal boundary, signaling
    // and quiet NaNs with multiple payloads, infinities and the largest finite.
    for (std::uint64_t bits : {0ULL, 1ULL, 2ULL, 0x0008'0000'0000'0000ULL, 0x000f'ffff'ffff'ffffULL,
                               0x0010'0000'0000'0000ULL, 0x0010'0000'0000'0001ULL, 0x7fefffffffffffffULL,
                               0x7ff0'0000'0000'0000ULL, 0x7ff0'0000'0000'0001ULL, 0x7ff8'0000'0000'0000ULL,
                               0x7fff'ffff'ffff'ffffULL}) {
        compare_bits<Target>(bits);
    }
    // Limb-sized shifts, the >=128 remainder path, and the last legal left
    // shifts for the 64/128-bit D=0 signed limits.
    for (int power : {-129, -128, -127, -65, -64, -63, -2, -1, 0, 1, 10, 11, 12, 63, 64, 74, 75, 76, 127, 128}) {
        const auto exponent = static_cast<std::uint64_t>(1075 - static_cast<int>(Target::fractional_digits) + power);
        compare_bits<Target>(exponent << 52);
        compare_bits<Target>((exponent << 52) | 0x000f'ffff'ffff'ffffULL);
    }
    check_ties<Target>();
    // At large raw magnitudes, adjacent quanta need not be distinguishable in
    // double. Walk representable neighbours around both scaled limits instead.
    const double edge = std::ldexp(1.0, static_cast<int>(Target::bits) - 1) /
                        std::pow(10.0, static_cast<double>(Target::fractional_digits));
    for (bool negative : {false, true}) {
        double lower = negative ? -edge : edge;
        double upper = lower;
        for (int i = 0; i < 16; ++i) {
            compare_kernel<Target>(lower);
            compare_kernel<Target>(upper);
            lower = std::nextafter(lower, -std::numeric_limits<double>::infinity());
            upper = std::nextafter(upper, std::numeric_limits<double>::infinity());
        }
    }
    std::mt19937_64 rng(0x6f7261636c65ULL + Target::bits * 100 + Target::fractional_digits);
    std::uniform_real_distribution<double> magnitude(0.0, 5'000.0);
    for (std::size_t i = 0; i < samples; ++i) {
        compare_bits<Target>(rng());
        const double value = magnitude(rng);
        compare_kernel<Target>(value);
        compare_kernel<Target>(-value);
    }
}

template<typename Target>
void check_error(double value, Rounding rounding, ArithmeticError error) {
    const auto result = from_float<Target>(value, rounding);
    CHECK(!result && result.error() == error);
}

void check_limits_and_rounding() {
    for (Rounding rounding : rounding_modes) {
        CHECK(from_float<Fixed64<0>>(-0x1p63, rounding) == Fixed64<0>::min());
        CHECK(from_float<Fixed128<0>>(-0x1p127, rounding) == Fixed128<0>::min());
        check_error<Fixed64<0>>(0x1p63, rounding, ArithmeticError::overflow);
        check_error<Fixed128<0>>(0x1p127, rounding, ArithmeticError::overflow);
    }
    // Small raw widths expose individual quanta and rounding overflow exactly.
    CHECK(from_float<Fixed8<0>>(127.25, Rounding::nearest_even)->raw() == 127);
    CHECK(from_float<Fixed8<0>>(-128.25, Rounding::toward_zero)->raw() == -128);
    CHECK(from_float<Fixed8<0>>(-128.5, Rounding::nearest_even)->raw() == -128);
    check_error<Fixed8<0>>(127.25, Rounding::ceil, ArithmeticError::overflow);
    check_error<Fixed8<0>>(127.5, Rounding::nearest_even, ArithmeticError::overflow);
    check_error<Fixed8<0>>(-128.25, Rounding::floor, ArithmeticError::overflow);
    check_error<Fixed8<0>>(-128.5, Rounding::nearest_away, ArithmeticError::overflow);
    check_error<Fixed8<0>>(127.25, Rounding::exact, ArithmeticError::inexact);
    check_error<Fixed8<0>>(128.25, Rounding::exact, ArithmeticError::overflow);
    check_error<Fixed8<0>>(-129.25, Rounding::exact, ArithmeticError::overflow);
    // D=32, right shift=128: the product can be above half, unlike D<=22.
    CHECK(from_float<Fixed128<32>>(0x1.fffffffffffffp-108)->raw() == wide::int128{1});
    CHECK(from_float<Fixed128<32>>(0x1p-108)->raw() == wide::int128{0});
    // Public fallback paths remain available beyond the fast-path width/scale.
    compare_kernel<Fixed128<33>>(0.125);
    compare_kernel<Fixed128<38>>(-0.5);
    compare_kernel<Fixed256<76>>(0.125);
}

void check_floating_environment() {
    const int saved = std::fegetround();
    CHECK(saved != -1);
    for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
        if (std::fesetround(mode) != 0) continue; // Some targets do not support all modes.
        compare_kernel<Fixed64<8>>(0.1);
        compare_kernel<Fixed64<10>>(-0.1);
        compare_kernel<Fixed64<12>>(0x1p-13);
        compare_bits<Fixed128<32>>(1);
    }
    CHECK(std::fesetround(saved) == 0);
}

void test_fast_double() {
    std::size_t samples = 2048;
    if (const char* value = std::getenv("FIXEDWIDE_FLOAT_SAMPLES")) {
        char* end = nullptr;
        const auto requested = std::strtoull(value, &end, 10);
        CHECK(end != value && *end == '\0' && requested > 0 && requested <= 1'000'000);
        samples = static_cast<std::size_t>(requested);
    }
    differential_type<Fixed8<0>>(samples);
    differential_type<Fixed8<2>>(samples);
    differential_type<Fixed16<4>>(samples);
    differential_type<Fixed32<6>>(samples);
    differential_type<Fixed32<9>>(samples);
    differential_type<Fixed64<0>>(samples);
    differential_type<Fixed64<2>>(samples);
    differential_type<Fixed64<6>>(samples);
    differential_type<Fixed64<8>>(samples);
    differential_type<Fixed64<10>>(samples);
    differential_type<Fixed64<12>>(samples);
    differential_type<Fixed64<18>>(samples);
    differential_type<Fixed128<0>>(samples);
    differential_type<Fixed128<8>>(samples);
    differential_type<Fixed128<19>>(samples);
    differential_type<Fixed128<22>>(samples);
    differential_type<Fixed128<27>>(samples);
    differential_type<Fixed128<28>>(samples);
    differential_type<Fixed128<32>>(samples);
    check_limits_and_rounding();
    check_floating_environment();
    std::printf("double/kernel differential comparisons: %llu\n", static_cast<unsigned long long>(differential_cases));
}
} // namespace

int main() {
    test_floating_conversions();
    test_fast_double();
    CHECK(Fixed8<2>::min().to_double() == to_double(Fixed8<2>::min()));
    CHECK(Fixed16<4>::max().to_double() == to_double(Fixed16<4>::max()));
    CHECK(Fixed32<9>::min().to_double() == to_double(Fixed32<9>::min()));
    CHECK(Fixed64<8>::max().to_double() == to_double(Fixed64<8>::max()));
    CHECK(Fixed128<12>::min().to_double() == to_double(Fixed128<12>::min()));
    CHECK(Fixed256<76>::max().to_double() == to_double(Fixed256<76>::max()));
    std::printf("test_floating passed (%lu checks)\n", checks);
    return 0;
}
