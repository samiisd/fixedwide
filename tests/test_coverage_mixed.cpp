#include "coverage_support.hpp"
#include <fixedwide/mixed.hpp>

using namespace coverage_test;
using namespace fixedwide;

namespace {

// Exercise the same constexpr reference helpers at runtime as well. Their
// results are compared to arbitrary-precision integers, not to the fast path.
void comparison_reference() {
    std::mt19937_64 rng(0x6d69786564ULL);
    auto values = boundaries(256);
    for (unsigned i = 0; i < 100; ++i) values.push_back(random_bits(rng, 256));
    for (const auto& x : values)
        for (const auto& y : values) {
            const auto a = from_integer<i256>(x), b = from_integer<i256>(y);
            const unsigned da = static_cast<unsigned>(rng() % 77), db = static_cast<unsigned>(rng() % 77);
            const cpp_int lhs = integer(a) * power10(db), rhs = integer(b) * power10(da);
            const auto result = detail::constexpr_mixed_compare(a, da, b, db);
            CHECK((result < 0) == (lhs < rhs));
            CHECK((result > 0) == (lhs > rhs));
            CHECK((result == 0) == (lhs == rhs));
        }
}

template<class D, class A, class B>
void mixed_pair() {
    std::mt19937_64 rng(17 + A::bits + B::bits + D::bits + A::fractional_digits);
    const cpp_int sa = power10(A::fractional_digits), sb = power10(B::fractional_digits),
                  sd = power10(D::fractional_digits);
    auto xs = boundaries(A::bits), ys = boundaries(B::bits);
    for (unsigned i = 0; i < 20; ++i) {
        xs.push_back(random_bits(rng, A::bits));
        ys.push_back(random_bits(rng, B::bits));
    }
    for (const auto& x : xs)
        for (const auto& y : ys) {
            const auto a = A::from_raw(from_integer<typename A::raw_type>(x));
            const auto b = B::from_raw(from_integer<typename B::raw_type>(y));
            const cpp_int aa = integer(a.raw()), bb = integer(b.raw());
            auto verify = [&](const char* op, const auto& actual, const auto& expected, Rounding mode) {
                if (actual.has_value() != expected.has_value() ||
                    (actual && expected && integer(actual->raw()) != *expected)) {
                    std::cerr << op << " dest=" << D::bits << "," << D::fractional_digits << " a=" << A::bits << ","
                              << A::fractional_digits << ":" << aa << " b=" << B::bits << "," << B::fractional_digits
                              << ":" << bb << " mode=" << int(mode) << " got="
                              << (actual ? integer(actual->raw()).template convert_to<std::string>() : "error")
                              << " expected=" << (expected ? expected->template convert_to<std::string>() : "error")
                              << '\n';
                }
                agrees(actual, expected);
            };
            for (auto mode : modes) {
                verify("add_to", add_to<D>(a, b, mode), rounded((aa * sb + bb * sa) * sd, sa * sb, mode, D::bits),
                       mode);
                verify("sub_to", sub_to<D>(a, b, mode), rounded((aa * sb - bb * sa) * sd, sa * sb, mode, D::bits),
                       mode);
                verify("mul_to", mul_to<D>(a, b, mode), rounded(aa * bb * sd, sa * sb, mode, D::bits), mode);
                verify("div_to", div_to<D>(a, b, mode), rounded(aa * sb * sd, bb * sa, mode, D::bits), mode);
                verify("mul_div_to", mul_div_to<D>(a, b, b, mode), rounded(aa * bb * sd, bb * sa, mode, D::bits), mode);
                verify("fixed_cast", fixed_cast<D>(a, mode), rounded(aa * sd, sa, mode, D::bits), mode);
            }
            CHECK((a == b) == (aa * sb == bb * sa));
            CHECK((a < b) == (aa * sb < bb * sa));
        }
}

// The coverage gate runs this binary on its own. Keep the runtime paths backed
// by an independent integer oracle here as well as by the kernel differential
// suite, including fallbacks whose intermediate overflows but final result fits.
template<class D, class A, class B>
void runtime_case(const cpp_int& x, const cpp_int& y) {
    const auto a = A::from_raw(from_integer<typename A::raw_type>(x));
    const auto b = B::from_raw(from_integer<typename B::raw_type>(y));
    const cpp_int aa = integer(a.raw()), bb = integer(b.raw());
    const cpp_int sa = power10(A::fractional_digits), sb = power10(B::fractional_digits),
                  sd = power10(D::fractional_digits);
    for (auto mode : modes) {
        agrees(add_to<D>(a, b, mode), rounded((aa * sb + bb * sa) * sd, sa * sb, mode, D::bits));
        agrees(sub_to<D>(a, b, mode), rounded((aa * sb - bb * sa) * sd, sa * sb, mode, D::bits));
        agrees(mul_to<D>(a, b, mode), rounded(aa * bb * sd, sa * sb, mode, D::bits));
        agrees(fixed_cast<D>(a, mode), rounded(aa * sd, sa, mode, D::bits));
    }
}

template<class D, class A>
void runtime_cast_case(const cpp_int& x) {
    const auto a = A::from_raw(from_integer<typename A::raw_type>(x));
    const cpp_int aa = integer(a.raw());
    for (auto mode : modes)
        agrees(fixed_cast<D>(a, mode),
               rounded(aa * power10(D::fractional_digits), power10(A::fractional_digits), mode, D::bits));
}

void runtime_narrow_paths() {
    const cpp_int p63 = cpp_int{1} << 63, p64 = cpp_int{1} << 64, p127 = cpp_int{1} << 127, p128 = cpp_int{1} << 128,
                  p192 = cpp_int{1} << 192, p255 = cpp_int{1} << 255;
    const std::vector<cpp_int> narrow{0, 1, -1, 15, -15, 127, 128, -129, p63 - 1, -p63, p64, -p64, p127 - 1, -p127};
    for (const auto& x : narrow)
        for (const auto& y : narrow) {
            runtime_case<Fixed128<12>, Fixed128<12>, Fixed128<12>>(x, y);
            runtime_case<Fixed128<12>, Fixed128<12>, Fixed64<10>>(x, y);
            runtime_case<Fixed128<10>, Fixed128<12>, Fixed128<10>>(x, y);
            runtime_case<Fixed128<18>, Fixed128<12>, Fixed128<6>>(x, y);
            runtime_case<Fixed256<12>, Fixed128<12>, Fixed128<12>>(x, y);
            runtime_case<Fixed256<10>, Fixed128<12>, Fixed128<10>>(x, y);
            runtime_case<Fixed8<0>, Fixed128<1>, Fixed128<0>>(x, y);
        }
    // Alignment can overflow before an opposing term brings the exact sum
    // into range. Native sum overflow can likewise disappear after rescaling.
    runtime_case<Fixed128<1>, Fixed128<0>, Fixed128<1>>(p127 / 10 + 1, -p127 + 1);
    runtime_case<Fixed128<0>, Fixed128<1>, Fixed128<1>>(p127 - 1, p127 - 1);
    for (int q = -3; q <= 3; ++q)
        for (int delta = -1; delta <= 1; ++delta) {
            const cpp_int raw = cpp_int{q} * power10(20) + power10(20) / 2 + delta;
            runtime_case<Fixed128<0>, Fixed128<20>, Fixed128<0>>(raw, 1);
            runtime_cast_case<Fixed128<0>, Fixed256<20>>(raw);
        }

    const std::vector<cpp_int> wide{0,    1,     -1,   p127 - 1, -p127,    p127, -p127 - 1,
                                    p128, -p128, p192, -p192,    p255 - 1, -p255};
    for (const auto& x : wide) {
        for (const auto& y : wide) {
            runtime_case<Fixed256<12>, Fixed256<12>, Fixed256<12>>(x, y);
            runtime_case<Fixed128<12>, Fixed256<12>, Fixed256<12>>(x, y);
            runtime_case<Fixed8<0>, Fixed256<0>, Fixed256<0>>(x, y);
        }
        runtime_case<Fixed256<12>, Fixed256<12>, Fixed128<12>>(x, 1);
        runtime_case<Fixed256<12>, Fixed128<12>, Fixed256<12>>(1, x);
        runtime_cast_case<Fixed128<0>, Fixed256<38>>(x);
        runtime_cast_case<Fixed128<0>, Fixed256<39>>(x);
        runtime_cast_case<Fixed128<0>, Fixed256<76>>(x);
        runtime_cast_case<Fixed128<38>, Fixed256<0>>(x);
    }
    for (const auto& endpoint : narrow) {
        runtime_case<Fixed128<0>, Fixed256<0>, Fixed256<0>>(p128, -p128 + endpoint);
        runtime_case<Fixed8<0>, Fixed256<0>, Fixed256<0>>(p128, -p128 + endpoint);
    }
    for (const auto& x : std::vector<cpp_int>{0, 1, -1, p127 - 1, -p127})
        for (const auto& y : std::vector<cpp_int>{0, 1, -1, p127 - 1, -p127}) {
            runtime_case<Fixed128<0>, Fixed128<19>, Fixed128<20>>(x, y);
            runtime_case<Fixed128<0>, Fixed128<38>, Fixed128<38>>(x, y);
            runtime_case<Fixed128<38>, Fixed128<0>, Fixed128<0>>(x, y);
            runtime_case<Fixed256<76>, Fixed128<0>, Fixed128<0>>(x, y);
            runtime_case<Fixed256<38>, Fixed128<0>, Fixed128<38>>(x, y);
        }
}

#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
// Calling through a volatile pointer intentionally checks the implementations
// of the bound predicates, which ordinary arithmetic evaluates at compile time.
template<auto Function>
void predicate(bool expected) {
    auto (*volatile fn)() noexcept = Function;
    CHECK(fn() == expected);
}
void native_bounds() {
    namespace n = detail::mixed_native;
    for (unsigned d = 0; d <= 76; ++d) CHECK(n::bits_for_pow10(d) == 4 * d);
    for (unsigned a : {8u, 16u, 32u, 64u})
        for (unsigned b : {8u, 16u, 32u, 64u}) CHECK(n::product_bits(a, b) == a + b - 1);
    predicate<&n::alignment_fits<64, 8, 64, 12>>(true);
    predicate<&n::alignment_fits<128, 8, 64, 12>>(false);
    predicate<&n::alignment_fits<64, 0, 64, 18>>(false);
    predicate<&n::alignment_fits<64, 18, 64, 0>>(false);
    predicate<&n::cast_fits<64, 4, 128, 8>>(true);
    predicate<&n::cast_fits<64, 8, 32, 4>>(true);
    predicate<&n::cast_fits<128, 8, 128, 8>>(false);
    predicate<&n::cast_fits<64, 0, 128, 38>>(false);
    predicate<&n::cast_fits<64, 0, 256, 38>>(false);
    predicate<&n::add_fits<64, 4, 64, 8, 128, 12>>(true);
    predicate<&n::add_fits<64, 4, 64, 8, 64, 2>>(true);
    predicate<&n::add_fits<128, 4, 64, 8, 128, 12>>(false);
    predicate<&n::add_fits<64, 4, 64, 8, 256, 12>>(false);
    predicate<&n::add_fits<64, 4, 64, 8, 128, 38>>(false);
    predicate<&n::mul_fits<32, 4, 32, 4, 128, 12>>(true);
    predicate<&n::mul_fits<64, 4, 64, 4, 64, 4>>(true);
    predicate<&n::mul_fits<64, 0, 64, 0, 128, 38>>(false);
    predicate<&n::mul_fits<128, 4, 64, 4, 128, 8>>(false);
    predicate<&n::div_fits<64, 4, 64, 4, 64, 4>>(true);
    predicate<&n::div_fits<64, 12, 64, 4, 64, 4>>(true);
    predicate<&n::div_fits<64, 18, 64, 0, 64, 0>>(false);
    predicate<&n::div_fits<128, 4, 64, 4, 64, 4>>(false);
    predicate<&n::mul_div_fits<32, 2, 32, 2, 32, 2, 64, 2>>(true);
    predicate<&n::mul_div_fits<64, 8, 64, 8, 64, 4, 64, 4>>(true);
    predicate<&n::mul_div_fits<128, 2, 64, 2, 64, 2, 128, 2>>(false);
    predicate<&n::mul_div_fits<64, 0, 64, 0, 64, 0, 128, 38>>(false);
    predicate<&n::mul_div_fits<64, 18, 64, 18, 64, 0, 64, 0>>(false);
    for (bool negative : {false, true}) {
        CHECK(integer(wide::uint128{n::limit_magnitude_u128<8>(negative)}) == 127 + negative);
        CHECK(integer(wide::uint128{n::limit_magnitude_u128<128>(negative)}) == (cpp_int{1} << 127) - 1 + negative);
    }
    CHECK(!n::check_range<8>(128));
    CHECK(!n::check_range<8>(-129));
    CHECK(n::check_range<128>(-42).value() == -42);
    const n::i128 big = n::i128{1} << 110;
    for (n::i128 value : {n::i128{0}, n::i128{1}, -n::i128{7}, big, -big, n::dest_min<128>(), n::dest_max<128>()}) {
        for (n::i128 divisor : {n::i128{1}, n::i128{10}, n::i128{1} << 80})
            for (auto mode : modes) {
                const auto got = n::divide_rounded(value, divisor, mode, n::limit_magnitude_u128<128>(value < 0));
                if (got)
                    agrees(std::expected<i128, ArithmeticError>{i128{*got}},
                           rounded(integer(i128{value}), integer(i128{divisor}), mode, 128));
                else
                    CHECK(rounded(integer(i128{value}), integer(i128{divisor}), mode, 128).error() == got.error());
            }
    }
}
#endif
} // namespace

void run_coverage_mixed() {
    comparison_reference();
    runtime_narrow_paths();
    // The denominator needs the 1024-bit tier; directed rounding still has a representable result.
    const auto x = Fixed256<0>::from_raw(from_integer<i256>(power10(76)));
    const auto z = Fixed256<76>::from_raw(from_integer<i256>(power10(76)));
    agrees(mul_div_to<Fixed256<76>>(x, z, x), rounded(power10(76), 1, Rounding::nearest_even, 256));
    for (auto mode : modes) {
        agrees(mul_div_to<Fixed256<0>>(z, z, x, mode), rounded(1, power10(76), mode, 256));
        agrees(mul_to<Fixed256<76>>(x, x, mode), rounded(power10(228), 1, mode, 256));
        agrees(div_to<Fixed256<76>>(x, Fixed256<76>::from_raw(i256{1}), mode), rounded(power10(228), 1, mode, 256));
    }
    CHECK(detail::mixed_compare_kernel(i256{-100}, 2, i256{-1}, 0) == 0);
    mixed_pair<Fixed64<4>, Fixed32<2>, Fixed16<4>>();
    mixed_pair<Fixed8<0>, Fixed64<8>, Fixed64<12>>();
    mixed_pair<Fixed128<18>, Fixed64<4>, Fixed32<4>>();
    mixed_pair<Fixed128<0>, Fixed64<18>, Fixed64<18>>();
    mixed_pair<Fixed256<76>, Fixed128<38>, Fixed256<0>>();
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    native_bounds();
#endif
}
