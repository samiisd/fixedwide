#include "check.hpp"
#include <fixedwide/floating.hpp>
#include <array>
#include <bit>
#include <cfenv>
#if defined(__x86_64__) || defined(_M_X64)
#include <xmmintrin.h>
#endif

using namespace fixedwide;

namespace {
constexpr std::array modes{Rounding::toward_zero, Rounding::floor, Rounding::ceil,
                          Rounding::nearest_even, Rounding::nearest_away, Rounding::exact};

template<typename Target, typename Float>
auto convert(std::uint64_t bits, Rounding rounding) noexcept {
    // Prevent constant folding: these inputs must pass through the runtime
    // header path, including under fast-math and flush-to-zero modes.
    volatile std::uint64_t input = bits;
    return from_float<Target>(std::bit_cast<Float>(static_cast<std::uint64_t>(input)), rounding);
}

template<typename Target, typename Float>
void check_specials() {
    for (Rounding rounding : modes) {
        for (std::uint64_t sign : {0ULL, 0x8000'0000'0000'0000ULL}) {
            const auto zero = convert<Target, Float>(sign, rounding);
            CHECK(zero && zero->raw() == typename Target::raw_type{0});
            const auto tiny = convert<Target, Float>(sign | 1, rounding);
            if (rounding == Rounding::exact) {
                CHECK(!tiny && tiny.error() == ArithmeticError::inexact);
            } else {
                const int raw = sign != 0 ? (rounding == Rounding::floor ? -1 : 0)
                                          : (rounding == Rounding::ceil ? 1 : 0);
                CHECK(tiny && tiny->raw() == typename Target::raw_type{raw});
            }
            for (std::uint64_t special : {0x7ff0'0000'0000'0000ULL, 0x7ff0'0000'0000'0001ULL,
                                          0x7ff8'0000'0000'0000ULL, 0x7fff'ffff'ffff'ffffULL}) {
                const auto invalid = convert<Target, Float>(sign | special, rounding);
                CHECK(!invalid && invalid.error() == ArithmeticError::invalid_value);
            }
        }
    }
}

template<typename Float>
void check_header_path() {
    if constexpr (detail::use_fast_from_float_v<Fixed64<8>, Float>) {
        CHECK(std::feclearexcept(FE_ALL_EXCEPT) == 0);
        check_specials<Fixed64<8>, Float>();
        check_specials<Fixed64<10>, Float>();
        check_specials<Fixed64<12>, Float>();
        check_specials<Fixed128<32>, Float>();
        // Integer extraction must not even raise FE_INVALID for a signaling NaN.
        CHECK(std::fetestexcept(FE_ALL_EXCEPT) == 0);
        CHECK(convert<Fixed64<0>, Float>(0xc3e0'0000'0000'0000ULL, Rounding::exact) == Fixed64<0>::min());
        CHECK(convert<Fixed128<0>, Float>(0xc7e0'0000'0000'0000ULL, Rounding::exact) == Fixed128<0>::min());
        // Positive 1.5: ties-to-even increments an odd quotient, toward_zero does not.
        CHECK(convert<Fixed64<0>, Float>(0x3ff8'0000'0000'0000ULL, Rounding::nearest_even)->raw() == 2);
        CHECK(convert<Fixed64<0>, Float>(0x3ff8'0000'0000'0000ULL, Rounding::toward_zero)->raw() == 1);
    }
}
} // namespace

int main() {
    check_header_path<double>();
#if defined(__x86_64__) || defined(_M_X64)
    const unsigned saved = _mm_getcsr();
    _mm_setcsr(saved | 0x8040u); // FTZ (bit 15) and DAZ (bit 6).
    check_header_path<double>();
    _mm_setcsr(saved);
#endif
    std::printf("test_floating_header_only passed (%llu checks)\n", static_cast<unsigned long long>(checks));
}
