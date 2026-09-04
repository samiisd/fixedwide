// The compile-time path and the runtime kernels are two independent
// implementations of one contract. This test asserts they agree.
//
// Both directions matter: the static_asserts prove the operations are usable in
// a constant expression at all, and the randomized loop proves the simple
// reference implementation and the optimized kernels return the SAME value --
// including the same error -- across every rounding mode, every width, and the
// boundary values where they are most likely to diverge.
#include <fixedwide/arithmetic.hpp>
#include <fixedwide/mixed.hpp>
#include "check.hpp"
#include <cstdio>
#include <random>

using fixedwide::Rounding;
namespace ce = fixedwide::detail::ce;

namespace {

constexpr Rounding all_modes[] = {
    Rounding::toward_zero, Rounding::floor, Rounding::ceil,
    Rounding::nearest_even, Rounding::nearest_away, Rounding::exact,
};

// Compare two std::expected results for exact agreement, value and error alike.
unsigned error_code(fixedwide::ArithmeticError e) { return static_cast<unsigned>(e); }

// Compare a reference result (a raw value) against a kernel result (a Fixed).
template<typename Ref, typename Kern>
void agree(const char* op, const Ref& reference, const Kern& kernel,
           long long a, long long b, unsigned digits, Rounding mode) {
    const bool same_shape = reference.has_value() == kernel.has_value();
    if (!same_shape) {
        std::fprintf(stderr, "%s a=%lld b=%lld digits=%u mode=%u: ref %s, kernel %s\n",
                     op, a, b, digits, static_cast<unsigned>(mode),
                     reference ? "ok" : "error", kernel ? "ok" : "error");
    }
    CHECK(same_shape);
    if (reference.has_value()) {
        CHECK(*reference == kernel->raw());
    } else {
        if (reference.error() != kernel.error()) {
            std::fprintf(stderr, "%s a=%lld b=%lld digits=%u mode=%u: ref err=%u kernel err=%u\n",
                         op, a, b, digits, static_cast<unsigned>(mode),
                         error_code(reference.error()), error_code(kernel.error()));
        }
        CHECK(reference.error() == kernel.error());
    }
}

template<std::size_t Bits, unsigned D, typename Fixed>
void compare_one(Fixed a, Fixed b, Fixed c, Rounding mode) {
    const auto ia = static_cast<long long>(Bits <= 64 ? static_cast<long long>(a.raw()) : 0);
    const auto ib = static_cast<long long>(Bits <= 64 ? static_cast<long long>(b.raw()) : 0);
    agree("mul", ce::mul<Bits, D>(a.raw(), b.raw(), mode), fixedwide::mul(a, b, mode), ia, ib, 0, mode);
    agree("div", ce::div<Bits, D>(a.raw(), b.raw(), mode), fixedwide::div(a, b, mode), ia, ib, 0, mode);
    agree("mul_div", ce::mul_div<Bits, D>(a.raw(), b.raw(), c.raw(), mode),
          fixedwide::mul_div(a, b, c, mode), ia, ib, 0, mode);
    for (unsigned digits = 0; digits <= D; ++digits) {
        agree("quantize", ce::quantize<Bits, D>(a.raw(), digits, mode),
              fixedwide::quantize(a, digits, mode), ia, ib, digits, mode);
    }
    agree("remainder", ce::remainder<Bits>(a.raw(), b.raw()), fixedwide::remainder(a, b), ia, ib, 0, mode);
}

template<std::size_t Bits, unsigned D, typename Fixed, typename Raw>
void sweep(const std::vector<Raw>& values) {
    for (auto x : values) {
        for (auto y : values) {
            for (auto mode : all_modes) {
                compare_one<Bits, D, Fixed>(Fixed::from_raw(x), Fixed::from_raw(y),
                                            Fixed::from_raw(y), mode);
            }
        }
    }
}

} // namespace

// Regression: cross-scale comparison is advertised as constexpr, so it must
// actually work in a constant expression for EVERY width -- not only the
// narrow mixed_native fast path. Before detail::constexpr_mixed_compare these
// static_asserts did not compile: the operator fell through to
// mixed_compare_kernel, which lives in the compiled library.
namespace mixed_constexpr_compare {
using namespace fixedwide;

// Same value, different scales and widths.
static_assert(Fixed64<4>::from_raw(1'000'000) == Fixed128<8>::from_raw(10'000'000'000LL));
static_assert(Fixed32<2>::from_raw(150) == Fixed256<12>::from_raw(wide::int256(1'500'000'000'000ULL)));
static_assert(Fixed128<0>::from_raw(wide::int128(7)) == Fixed64<6>::from_raw(7'000'000));

// Ordering, including across the sign.
static_assert(Fixed64<4>::from_raw(1'000'000) < Fixed128<8>::from_raw(10'000'000'001LL));
static_assert(Fixed64<4>::from_raw(1'000'000) > Fixed128<8>::from_raw(9'999'999'999LL));
static_assert(Fixed128<2>::from_raw(wide::int128(-1)) < Fixed64<4>::from_raw(0));
static_assert(Fixed128<2>::from_raw(wide::int128(-1)) < Fixed32<6>::from_raw(1));
static_assert(Fixed64<2>::from_raw(-100) > Fixed128<4>::from_raw(wide::int128(-20'000)));
static_assert(Fixed64<2>::from_raw(-100) == Fixed128<4>::from_raw(wide::int128(-10'000)));

// Both zero, reached by different scales.
static_assert(Fixed8<2>::from_raw(0) == Fixed256<76>::from_raw(wide::int256()));

// The extremes, where the aligned magnitudes are widest: comparing a scale-0
// 256-bit value against an 18-decimal one lifts the latter by 10^18, so the
// aligned magnitude needs about 315 bits.
static_assert(Fixed256<0>::max() > Fixed64<18>::max());
// Fixed256<76>::min() is about -5.79, because all 76 of its digits are
// fractional; Fixed64<0>::min() is -9223372036854775808. Aligning them lifts
// the latter by 10^76, to about 316 bits.
static_assert(Fixed256<76>::min() > Fixed64<0>::min());
static_assert(Fixed256<76>::max() < Fixed64<0>::max());
} // namespace mixed_constexpr_compare

int main() {
    // --- usable in a constant expression at all -------------------------
    using T = fixedwide::Fixed64<12>;
    static_assert(fixedwide::mul(T::from_raw(2500000000000), T::from_raw(4000000000000))->raw()
                  == 10000000000000);
    static_assert(fixedwide::div(T::from_raw(1000000000000), T::from_raw(3000000000000))->raw()
                  == 333333333333);
    static_assert(!fixedwide::div(T::from_raw(1), T::from_raw(0)).has_value());
    static_assert(!fixedwide::div(T::from_raw(1000000000000), T::from_raw(3000000000000),
                                  Rounding::exact).has_value());
    static_assert(fixedwide::quantize(T::from_raw(1239000000000), 2)->raw() == 1240000000000);
    static_assert(fixedwide::remainder(T::from_raw(7), T::from_raw(3))->raw() == 1);

    // --- the two implementations agree ----------------------------------
    std::mt19937_64 rng(0xC0FFEE);

    {   // 64-bit, boundary values plus random ones
        std::vector<std::int64_t> values{0, 1, -1, 2, -2, 5, -5, 10, -10,
                                         INT64_MAX, INT64_MIN, INT64_MAX - 1, INT64_MIN + 1,
                                         1000000000000LL, -1000000000000LL,
                                         999999999999LL, 500000000000LL, -500000000000LL};
        for (int i = 0; i < 24; ++i) values.push_back(static_cast<std::int64_t>(rng()));
        sweep<64, 12, fixedwide::Fixed64<12>, std::int64_t>(values);
    }
    {   // narrow widths, where the signed minimum and the scale interact
        std::vector<std::int32_t> values{0, 1, -1, INT32_MAX, INT32_MIN, 100000, -100000, 7, -7};
        for (int i = 0; i < 12; ++i) values.push_back(static_cast<std::int32_t>(rng()));
        sweep<32, 4, fixedwide::Fixed32<4>, std::int32_t>(values);
    }
    {
        std::vector<std::int8_t> values{0, 1, -1, INT8_MAX, INT8_MIN, 99, -99};
        sweep<8, 2, fixedwide::Fixed8<2>, std::int8_t>(values);
    }
    {   // 128-bit, including values that force the wide path
        using Raw = fixedwide::wide::int128;
        std::vector<Raw> values{Raw(0), Raw(1), Raw(-1), Raw(1000000000000LL), Raw(-1000000000000LL),
                                fixedwide::Fixed128<12>::max().raw(),
                                fixedwide::Fixed128<12>::min().raw()};
        for (int i = 0; i < 10; ++i) values.push_back(Raw(rng(), rng()));
        sweep<128, 12, fixedwide::Fixed128<12>, Raw>(values);
    }

    std::printf("test_constexpr passed (%lu checks)\n", checks);
    return 0;
}
