#include "check.hpp"
#include "floating_test_bits.hpp"
#include <fixedwide/floating.hpp>
#include <cfenv>
#include <cstring>
#if defined(__SSE__) || defined(_M_X64)
#include <xmmintrin.h>
#endif
using namespace fixedwide;

// Volatile raw loads keep these checks on the runtime path, rather than
// allowing the optimizer to replace every known input with its final bits.
template<typename Float, typename Target>
Float convert(Target value) {
    if constexpr (Target::bits <= 64) {
        volatile auto raw = value.raw();
        return to_float<Float>(Target::from_raw(raw));
    } else {
        volatile std::uint64_t low = value.raw().low;
        volatile std::uint64_t high = value.raw().high;
        return to_float<Float>(Target::from_raw(wide::int128{low, high}));
    }
}

template<typename Float>
void check_bits(Float actual, unsigned exponent, wide::uint128 significand, bool negative = false) {
    Float expected;
    floating_test::store(expected, negative, exponent, significand);
    constexpr auto size = std::numeric_limits<Float>::digits == 64 ? 10 : sizeof(Float);
    CHECK(std::memcmp(&actual, &expected, size) == 0);
}

template<typename Float>
void check_format() {
    if constexpr (floating_test::supported<Float>) {
        constexpr unsigned p = std::numeric_limits<Float>::digits;
        constexpr unsigned bias = std::numeric_limits<Float>::max_exponent - 1;
        const auto hidden = wide::uint128{1} << (p - 1);
        check_bits(convert<Float>(Fixed128<38>{}), 0, {});
        for (bool negative : {false, true}) {
            const auto integer = [&](wide::uint128 mag) {
                auto bits = negative ? wide::uint128{} - mag : mag;
                return Fixed128<0>::from_raw(wide::int128{bits.low, bits.high});
            };
            // Ties at the source precision, not just double-sized examples.
            const auto tie = (wide::uint128{1} << p) + wide::uint128{1};
            check_bits(convert<Float>(integer(tie)), bias + p, hidden, negative);
            check_bits(convert<Float>(integer(tie + wide::uint128{2})), bias + p, hidden + wide::uint128{2}, negative);
            check_bits(convert<Float>(Fixed128<0>::min()), bias + 127, hidden, true);
            // 0.1, with independently specified known representations.
            const auto tenth = Fixed128<1>::from_raw(negative ? -1 : 1);
            if constexpr (p == 24) check_bits(convert<Float>(tenth), bias - 4, wide::uint128{0xcccccd}, negative);
            if constexpr (p == 53)
                check_bits(convert<Float>(tenth), bias - 4, wide::uint128{0x1999999999999aULL}, negative);
            if constexpr (p == 64)
                check_bits(convert<Float>(tenth), bias - 4, wide::uint128{0xcccccccccccccccdULL}, negative);
            if constexpr (p == 113)
                check_bits(convert<Float>(tenth), bias - 4, wide::uint128{0x999999999999999aULL, 0x1999999999999ULL},
                           negative);
        }
        if constexpr (std::numeric_limits<Float>::digits == 53) {
            check_bits(convert<Float>(Fixed128<8>::from_raw(9'007'199'254'740'993LL)), 1049,
                       wide::uint128{0x15798ee2308c3bULL});
        }
        if constexpr (std::numeric_limits<Float>::digits == 24) {
            check_bits(convert<Float>(Fixed64<13>::from_raw(20'480'003'662'109'374LL)), 138, wide::uint128{0x800001});
            check_bits(convert<Float>(Fixed128<38>::from_raw(1)), 0, wide::uint128{0x6ce3ee});
            check_bits(convert<Float>(Fixed128<38>::from_raw(-1)), 0, wide::uint128{0x6ce3ee}, true);
            check_bits(convert<Float>(Fixed128<38>::from_raw(2)), 1, wide::uint128{0xd9c7dd});
        }
    }
}

int main() {
    const int saved = std::fegetround();
#if defined(__SSE__) || defined(_M_X64)
    const unsigned csr = _mm_getcsr();
#endif
    for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
        CHECK(std::fesetround(mode) == 0);
        for (unsigned flush : {0u, 0x8040u}) {
#if defined(__SSE__) || defined(_M_X64)
            _mm_setcsr((_mm_getcsr() & ~0x8040u) | flush);
#else
            (void)flush;
#endif
            CHECK(std::feclearexcept(FE_ALL_EXCEPT) == 0);
            check_format<float>();
            check_format<double>();
            check_format<long double>();
            CHECK(std::fetestexcept(FE_ALL_EXCEPT) == 0);
        }
    }
    CHECK(std::fesetround(saved) == 0);
#if defined(__SSE__) || defined(_M_X64)
    _mm_setcsr(csr);
#endif
    std::printf("to_float header-only: %llu checks\n", static_cast<unsigned long long>(checks));
}
