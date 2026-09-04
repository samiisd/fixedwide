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
#include <cstdio>
#include <random>
#include <vector>

using fixedwide::Rounding;
namespace detail = fixedwide::detail;

namespace {


constexpr Rounding all_modes[] = {
    Rounding::toward_zero, Rounding::floor, Rounding::ceil,
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
                                                detail::to_int256_raw(b.raw()), B::fractional_digits,
                                                false, Dest::fractional_digits, mode, Dest::bits),
                   "add_to");
        same<Dest>(fixedwide::sub_to<Dest>(a, b, mode),
                   detail::mixed_add_sub_kernel(detail::to_int256_raw(a.raw()), A::fractional_digits,
                                                detail::to_int256_raw(b.raw()), B::fractional_digits,
                                                true, Dest::fractional_digits, mode, Dest::bits),
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

} // namespace

#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
// divide_magnitude has a hardware path for a divisor that fits 64 bits, and
// divq faults rather than wraps when its quotient does not fit one limb. Sweep
// every magnitude and divisor width against the compiler's own division.
void check_divide_magnitude() {
    using u128 = unsigned __int128;
    std::mt19937_64 rng(0x0D17);
    for (unsigned magnitude_bits = 0; magnitude_bits <= 128; ++magnitude_bits) {
        for (unsigned divisor_bits = 1; divisor_bits <= 128; ++divisor_bits) {
            for (int repeat = 0; repeat < 4; ++repeat) {
                const auto mask = [](unsigned bits) {
                    return bits >= 128 ? ~u128{0} : ((u128{1} << bits) - 1);
                };
                u128 magnitude = ((static_cast<u128>(rng()) << 64) | rng()) & mask(magnitude_bits);
                u128 divisor = (((static_cast<u128>(rng()) << 64) | rng()) & mask(divisor_bits)) | 1;
                const auto got = detail::mixed_native::divide_magnitude(magnitude, divisor);
                CHECK(got.quotient == magnitude / divisor);
                CHECK(got.remainder == magnitude % divisor);
            }
        }
    }
}
static_assert(detail::mixed_native::divide_magnitude(
                  static_cast<unsigned __int128>(1'000'000), static_cast<unsigned __int128>(7)).quotient == 142857);
#else
void check_divide_magnitude() {}
#endif

int main() {
    check_divide_magnitude();
    using Price = fixedwide::Fixed64<8>;
    using Rate  = fixedwide::Fixed64<12>;
    using Money = fixedwide::Fixed128<12>;
    using Small = fixedwide::Fixed32<6>;
    using Tiny  = fixedwide::Fixed16<2>;

    std::mt19937_64 rng(0x31CED);

    std::vector<std::int64_t> wide_values{0, 1, -1, 2, -2,
                                          INT64_MAX, INT64_MIN, INT64_MAX - 1, INT64_MIN + 1,
                                          100'000'000LL, -100'000'000LL,
                                          1'000'000'000'000LL, -1'000'000'000'000LL};
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
    sweep<Money, Price, fixedwide::Fixed128<18>>("Money<-Price,F128",
        wide_values, std::vector<fixedwide::wide::int128>{
            fixedwide::wide::int128(0), fixedwide::wide::int128(1),
            fixedwide::wide::int128(-1), fixedwide::wide::int128(1'000'000'000'000LL)});

    std::printf("test_mixed_native passed (%lu checks)\n", checks);
    return 0;
}
