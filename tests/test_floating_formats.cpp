#include "check.hpp"
#include "floating_test_bits.hpp"
#include <fixedwide/floating.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <cfenv>
#include <cstdlib>
#include <random>

using namespace fixedwide;
using boost::multiprecision::cpp_int;
namespace {
constexpr std::array modes{Rounding::toward_zero,  Rounding::floor,        Rounding::ceil,
                           Rounding::nearest_even, Rounding::nearest_away, Rounding::exact};
std::uint64_t comparisons = 0;

cpp_int integer(wide::uint128 value) {
    return (cpp_int{value.high} << 64) + value.low;
}

template<typename Target>
cpp_int integer(Target value) {
    if constexpr (Target::bits <= 64)
        return cpp_int{value.raw()};
    else {
        const auto raw = value.raw();
        cpp_int result;
        if constexpr (Target::bits == 128)
            result = (cpp_int{raw.high} << 64) + raw.low;
        else {
            for (int i = 3; i >= 0; --i) {
                result <<= 64;
                result += raw.limbs[i];
            }
        }
        if (raw.is_negative()) result -= cpp_int{1} << Target::bits;
        return result;
    }
}

template<typename Target>
std::expected<cpp_int, ArithmeticError> oracle(wide::uint128 significand, int power, bool negative, Rounding rounding) {
    cpp_int numerator = integer(significand);
    for (unsigned d = 0; d < Target::fractional_digits; ++d) numerator *= 10;
    cpp_int denominator = 1;
    if (power >= 0)
        numerator <<= power;
    else
        denominator <<= -power;
    cpp_int q = numerator / denominator;
    const cpp_int r = numerator % denominator;
    const cpp_int limit = (cpp_int{1} << (Target::bits - 1)) - (negative ? 0 : 1);
    if (q > limit) return std::unexpected(ArithmeticError::overflow);
    if (r != 0) {
        if (rounding == Rounding::exact) return std::unexpected(ArithmeticError::inexact);
        bool up = false;
        switch (rounding) {
        case Rounding::floor: up = negative; break;
        case Rounding::ceil: up = !negative; break;
        case Rounding::nearest_even: up = 2 * r > denominator || (2 * r == denominator && (q & 1) != 0); break;
        case Rounding::nearest_away: up = 2 * r >= denominator; break;
        default: break;
        }
        if (up) ++q;
        if (q > limit) return std::unexpected(ArithmeticError::overflow);
    }
    if (negative) q = -q;
    return q;
}

template<typename Target, typename Float>
void compare(bool negative, unsigned exponent, wide::uint128 significand, unsigned char padding = 0) {
    Float value;
    floating_test::store(value, negative, exponent, significand, padding);
    const int power = static_cast<int>(exponent == 0 ? 1 : exponent) - (std::numeric_limits<Float>::max_exponent - 1) -
                      (floating_test::precision<Float> - 1);
    for (Rounding mode : modes) {
        const auto expected = oracle<Target>(significand, power, negative, mode);
        const auto actual = from_float<Target>(value, mode);
        const bool equal =
            expected ? actual && integer(*actual) == *expected : !actual && actual.error() == expected.error();
        if (!equal) {
            std::fprintf(stderr, "precision=%d target=%zu/%u sign=%u exponent=%u sig=%llx:%llx rounding=%u\n",
                         floating_test::precision<Float>, Target::bits, Target::fractional_digits,
                         static_cast<unsigned>(negative), exponent, static_cast<unsigned long long>(significand.high),
                         static_cast<unsigned long long>(significand.low), static_cast<unsigned>(mode));
        }
        CHECK(equal);
#ifndef FIXEDWIDE_TEST_ALTERNATE_LDBL
        // A real, unchanged, independently decoded reference on the host ABI.
        // Never call glibc frexpl under an incompatible -mlong-double-* ABI.
        const auto generic = detail::from_float_kernel(value, Target::fractional_digits, mode, Target::bits);
        CHECK(generic ? actual && detail::to_int256_raw(actual->raw()) == *generic
                      : !actual && actual.error() == generic.error());
#endif
        ++comparisons;
    }
}

template<typename Target, typename Float>
void exercise(std::size_t samples) {
    constexpr unsigned p = floating_test::precision<Float>;
    constexpr unsigned bias = std::numeric_limits<Float>::max_exponent - 1;
    constexpr auto leading = wide::uint128{1} << (p - 1);
    constexpr auto mask = leading - wide::uint128{1};
    for (bool sign : {false, true}) {
        for (auto sig : {wide::uint128{}, wide::uint128{1}, mask}) compare<Target, Float>(sign, 0, sig);
        // Normal minimum, maximum; binary shifts around every intermediate width.
        for (unsigned exponent : {1u, bias, 2 * bias}) {
            compare<Target, Float>(sign, exponent, leading);
            compare<Target, Float>(sign, exponent, leading | mask);
        }
        for (int shift : {-257, -256, -255, -129, -128, -127, -65, -64, -63, -1, 0, 1, 63, 64, 127, 128, 255, 256}) {
            const int exponent = shift + static_cast<int>(bias + p - 1) - static_cast<int>(Target::fractional_digits);
            if (exponent > 0 && exponent <= static_cast<int>(2 * bias)) {
                compare<Target, Float>(sign, static_cast<unsigned>(exponent), leading);
                compare<Target, Float>(sign, static_cast<unsigned>(exponent), leading | mask);
            }
        }
        // Exact ties on the decimal grid; both quotient parities, plus neighbours.
        for (unsigned odd : {1u, 3u, 5u, 7u}) {
            const auto top = static_cast<unsigned>(std::bit_width(odd) - 1);
            const auto sig = wide::uint128{odd} << (p - 1 - top);
            const auto exponent = bias + top - Target::fractional_digits - 1;
            for (int delta : {-1, 0, 1}) {
                auto near = delta < 0 ? sig - wide::uint128{1} : sig + wide::uint128{delta};
                unsigned e = exponent;
                if (near < leading) {
                    near = leading | mask;
                    --e;
                }
                compare<Target, Float>(sign, e, near);
            }
        }
        // Near signed limits, all decimal scales, without rounding a boundary
        // through a floating division. Find neighbouring representable inputs.
        cpp_int scale = 1;
        for (unsigned d = 0; d < Target::fractional_digits; ++d) scale *= 10;
        const cpp_int limit = (cpp_int{1} << (Target::bits - 1)) - (sign ? 0 : 1);
        int power = static_cast<int>(Target::bits) - static_cast<int>(p);
        cpp_int sig;
        do {
            cpp_int n = limit, den = scale;
            if (power >= 0)
                den <<= power;
            else
                n <<= -power;
            sig = n / den;
            if (sig < integer(leading))
                --power;
            else
                break;
        } while (true);
        for (int delta = -3; delta <= 3; ++delta) {
            cpp_int neighbour = sig + delta;
            int e = power;
            if (neighbour < integer(leading)) {
                neighbour <<= 1;
                --e;
            }
            if (neighbour >= integer(leading << 1)) {
                neighbour >>= 1;
                ++e;
            }
            const cpp_int low_mask = (cpp_int{1} << 64) - 1;
            const wide::uint128 s{static_cast<std::uint64_t>(neighbour & low_mask),
                                  static_cast<std::uint64_t>(neighbour >> 64)};
            compare<Target, Float>(sign, static_cast<unsigned>(e + static_cast<int>(bias + p - 1)), s);
        }
    }
    std::mt19937_64 rng(0x665'c0ffeeULL + Target::bits + Target::fractional_digits + p);
    for (std::size_t i = 0; i < samples; ++i) {
        auto sig = wide::uint128{rng(), rng()} & mask;
        const auto exponent = static_cast<unsigned>(rng() % (2 * bias + 1));
        if (exponent != 0) sig |= leading;
        compare<Target, Float>((rng() & 1) != 0, exponent, sig);
        // Concentrate a second population around useful magnitudes, not just
        // extreme exponents that trivially overflow/underflow the destination.
        compare<Target, Float>((rng() & 1) != 0, bias - 32 + static_cast<unsigned>(rng() % 46), sig | leading);
    }
    if constexpr (p == 64) {
        // x87 pseudo-denormals, and padding bytes that must not affect values.
        compare<Target, Float>(false, 0, leading | wide::uint128{123});
        compare<Target, Float>(true, bias, leading | wide::uint128{123}, 0x5a);
        compare<Target, Float>(true, bias, leading | wide::uint128{123}, 0xa5);
    }
}

template<typename Float>
void all_types(std::size_t samples) {
    if constexpr (floating_test::supported<Float>) {
        static_assert(detail::use_fast_from_float_v<Fixed64<8>, Float>);
        static_assert(detail::use_fast_from_float_v<Fixed128<38>, Float>);
        static_assert(!detail::use_fast_from_float_v<Fixed256<0>, Float>);
        if constexpr (floating_test::precision<Float> == 24) {
            static_assert(detail::float_intermediate_bits_v<Fixed64<17>, Float> == 64);
            static_assert(detail::float_intermediate_bits_v<Fixed64<18>, Float> == 128);
        } else if constexpr (floating_test::precision<Float> == 64) {
            static_assert(detail::float_intermediate_bits_v<Fixed128<27>, Float> == 128);
            static_assert(detail::float_intermediate_bits_v<Fixed128<28>, Float> == 256);
        } else if constexpr (floating_test::precision<Float> == 113) {
            static_assert(detail::float_intermediate_bits_v<Fixed64<6>, Float> == 128);
            static_assert(detail::float_intermediate_bits_v<Fixed64<7>, Float> == 256);
        }
        exercise<Fixed8<0>, Float>(samples);
        exercise<Fixed8<2>, Float>(samples);
        exercise<Fixed16<4>, Float>(samples);
        exercise<Fixed32<9>, Float>(samples);
        exercise<Fixed64<0>, Float>(samples);
        exercise<Fixed64<4>, Float>(samples);
        exercise<Fixed64<6>, Float>(samples);
        exercise<Fixed64<8>, Float>(samples);
        exercise<Fixed64<10>, Float>(samples);
        exercise<Fixed64<12>, Float>(samples);
        exercise<Fixed64<17>, Float>(samples);
        exercise<Fixed64<18>, Float>(samples);
        exercise<Fixed128<0>, Float>(samples);
        exercise<Fixed128<6>, Float>(samples);
        exercise<Fixed128<7>, Float>(samples);
        exercise<Fixed128<18>, Float>(samples);
        exercise<Fixed128<27>, Float>(samples);
        exercise<Fixed128<28>, Float>(samples);
        exercise<Fixed128<32>, Float>(samples);
        exercise<Fixed128<33>, Float>(samples);
        exercise<Fixed128<38>, Float>(samples);
        constexpr auto leading = wide::uint128{1} << (floating_test::precision<Float> - 1);
        constexpr unsigned bias = std::numeric_limits<Float>::max_exponent - 1;
        for (bool sign : {false, true}) {
            for (auto sig : {leading, leading | wide::uint128{1}, leading | (leading >> 1)}) {
                Float value;
                floating_test::store(value, sign, 2 * bias + 1, sig);
                for (Rounding mode : modes) {
                    const auto actual = from_float<Fixed64<12>>(value, mode);
                    CHECK(!actual && actual.error() == ArithmeticError::invalid_value);
#ifndef FIXEDWIDE_TEST_ALTERNATE_LDBL
                    const auto generic = detail::from_float_kernel(value, 12, mode, 64);
                    CHECK(!generic && generic.error() == actual.error());
#endif
                }
            }
#ifndef FIXEDWIDE_TEST_ALTERNATE_LDBL
            // The wider destination must still use the unchanged generic path.
            compare<Fixed256<76>, Float>(sign, bias, leading | (leading >> 2));
#endif
        }
        const int saved = std::fegetround();
        for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
            if (std::fesetround(mode) == 0) exercise<Fixed64<12>, Float>(4);
        }
        CHECK(std::fesetround(saved) == 0);
    }
}
} // namespace

int main() {
    std::size_t samples = 1024;
    if (const char* env = std::getenv("FIXEDWIDE_FLOAT_SAMPLES")) {
        char* end = nullptr;
        const auto n = std::strtoull(env, &end, 10);
        CHECK(end != env && *end == '\0' && n > 0 && n <= 1'000'000);
        samples = static_cast<std::size_t>(n);
    }
#ifndef FIXEDWIDE_TEST_ALTERNATE_LDBL
    all_types<float>(samples);
    all_types<double>(samples);
#endif
    all_types<long double>(samples);
    std::printf("floating formats: %llu exact-rational comparisons passed (long double precision=%d)\n",
                static_cast<unsigned long long>(comparisons), floating_test::precision<long double>);
}
