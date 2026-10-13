#include "check.hpp"
#include "floating_test_bits.hpp"
#include <fixedwide/floating.hpp>
#include <array>
#include <cfenv>
#if defined(__x86_64__) || defined(_M_X64)
#include <xmmintrin.h>
#endif

using namespace fixedwide;

namespace {
constexpr std::array modes{Rounding::toward_zero,  Rounding::floor,        Rounding::ceil,
                           Rounding::nearest_even, Rounding::nearest_away, Rounding::exact};

template<typename Target, typename Float>
auto convert(bool negative, unsigned exponent, wide::uint128 significand, Rounding rounding) noexcept {
    // Make the integer fields runtime inputs, even under fast-math. The encoder
    // and conversion must not evaluate FP arithmetic or classify via FP compares.
    volatile std::uint64_t low = significand.low, high = significand.high;
    Float value;
    floating_test::store(value, negative, exponent, {low, high});
    return from_float<Target>(value, rounding);
}

template<typename Target, typename Float>
void check_specials() {
    constexpr auto leading = wide::uint128{1} << (floating_test::precision<Float> - 1);
    constexpr unsigned special_exponent = 2 * std::numeric_limits<Float>::max_exponent - 1;
    for (Rounding rounding : modes) {
        for (bool negative : {false, true}) {
            const auto zero = convert<Target, Float>(negative, 0, {}, rounding);
            CHECK(zero && zero->raw() == typename Target::raw_type{0});
            const auto tiny = convert<Target, Float>(negative, 0, wide::uint128{1}, rounding);
            if (rounding == Rounding::exact) {
                CHECK(!tiny && tiny.error() == ArithmeticError::inexact);
            } else {
                const int raw =
                    negative ? (rounding == Rounding::floor ? -1 : 0) : (rounding == Rounding::ceil ? 1 : 0);
                CHECK(tiny && tiny->raw() == typename Target::raw_type{raw});
            }
            // Infinity, signaling NaN, quiet NaN, and a maximal NaN payload.
            for (auto sig :
                 {leading, leading | wide::uint128{1}, leading | (leading >> 1), (leading << 1) - wide::uint128{1}}) {
                const auto invalid = convert<Target, Float>(negative, special_exponent, sig, rounding);
                CHECK(!invalid && invalid.error() == ArithmeticError::invalid_value);
            }
            if constexpr (floating_test::precision<Float> == 64) {
                const auto unnormal = convert<Target, Float>(negative, 1, wide::uint128{1}, rounding);
                CHECK(!unnormal && unnormal.error() == ArithmeticError::invalid_value);
            }
        }
    }
}

template<typename Float>
void check_header_path() {
    if constexpr (floating_test::supported<Float>) {
        static_assert(detail::use_fast_from_float_v<Fixed64<8>, Float>);
        static_assert(detail::use_fast_from_float_v<Fixed128<38>, Float>);
        static_assert(noexcept(from_float<Fixed64<8>>(Float{})));
        constexpr auto leading = wide::uint128{1} << (floating_test::precision<Float> - 1);
        constexpr unsigned bias = std::numeric_limits<Float>::max_exponent - 1;
        CHECK(std::feclearexcept(FE_ALL_EXCEPT) == 0);
        check_specials<Fixed64<8>, Float>();
        check_specials<Fixed64<10>, Float>();
        check_specials<Fixed64<12>, Float>();
        check_specials<Fixed128<38>, Float>();
        CHECK(std::fetestexcept(FE_ALL_EXCEPT) == 0);
        CHECK(convert<Fixed64<0>, Float>(true, bias + 63, leading, Rounding::exact) == Fixed64<0>::min());
        CHECK(convert<Fixed128<0>, Float>(true, bias + 127, leading, Rounding::exact) == Fixed128<0>::min());
        for (unsigned odd : {1u, 3u, 5u, 7u}) {
            const auto top = static_cast<unsigned>(std::bit_width(odd) - 1);
            const auto sig = wide::uint128{odd} << (floating_test::precision<Float> - 1 - top);
            const auto even = convert<Fixed64<0>, Float>(false, bias + top - 1, sig, Rounding::nearest_even);
            CHECK(even && even->raw() == static_cast<std::int64_t>((odd / 2) + ((odd / 2) & 1)));
        }
        // Extra source significand bits must survive: 1 + 2^-(p-1), at D=38.
        // This value remains representable in every supported floating format.
        const auto extra = convert<Fixed128<38>, Float>(false, bias, leading | wide::uint128{1}, Rounding::ceil);
        const auto one = convert<Fixed128<38>, Float>(false, bias, leading, Rounding::exact);
        CHECK(extra && one && extra->raw() > one->raw());
        // Both sides of the 64-bit-product shift==64 half boundary (binary32 D17).
        if constexpr (floating_test::precision<Float> == 24) {
            CHECK(convert<Fixed64<17>, Float>(false, bias - 58, leading, Rounding::nearest_even)->raw() == 0);
            CHECK(
                convert<Fixed64<17>, Float>(false, bias - 58, (leading << 1) - wide::uint128{1}, Rounding::nearest_even)
                    ->raw() == 1);
        }
        CHECK(std::fetestexcept(FE_ALL_EXCEPT) == 0);
    }
}

void check_all() {
    check_header_path<float>();
    check_header_path<double>();
    check_header_path<long double>();
}
} // namespace

int main() {
    const int saved_rounding = std::fegetround();
    CHECK(saved_rounding != -1);
    for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
        if (std::fesetround(mode) == 0) check_all();
    }
    CHECK(std::fesetround(saved_rounding) == 0);
#if defined(__x86_64__) || defined(_M_X64)
    const unsigned saved = _mm_getcsr();
    _mm_setcsr(saved | 0x8040u); // FTZ (bit 15) and DAZ (bit 6).
    check_all();
    _mm_setcsr(saved);
#endif
    std::printf("test_floating_header_only passed (%llu checks, long double precision=%d)\n",
                static_cast<unsigned long long>(checks), floating_test::precision<long double>);
}
