#include "coverage_support.hpp"
#include <fixedwide/mixed.hpp>
#include "src/detail.hpp"
#include "src/native.hpp"

using namespace coverage_test;
using namespace fixedwide;

template<class T>
void constructors() {
    constexpr unsigned D = T::fractional_digits;
    const cpp_int scale_value = power10(D);
    auto test = [&]<class I>(I input) {
        cpp_int v;
#if defined(__SIZEOF_INT128__)
        if constexpr (sizeof(I) == 16) {
            const bool neg = std::is_same_v<I, __int128> && input < 0;
            auto u = static_cast<unsigned __int128>(input);
            if (neg) u = 0 - u;
            v = (cpp_int{static_cast<std::uint64_t>(u >> 64)} << 64) + static_cast<std::uint64_t>(u);
            if (neg) v = -v;
        } else
#endif
            v = cpp_int{input};
        agrees(fixedwide::from_integer<T>(input), rounded(v * scale_value, 1, Rounding::exact, T::bits));
    };
    for (std::int64_t v : std::array<std::int64_t, 9>{INT64_MIN, -65536, -42, -1, 0, 1, 42, 65536, INT64_MAX}) test(v);
    test(std::uint64_t{0});
    test(std::uint64_t{UINT64_MAX});
    test(std::uint32_t{UINT32_MAX});
    test(std::int8_t{-128});
    test(std::int16_t{-32768});
    test(std::int32_t{INT32_MIN});
    test(std::uint8_t{255});
    test(std::uint16_t{65535});
    test(false);
    test(true);
#if defined(__SIZEOF_INT128__)
    const unsigned __int128 high = static_cast<unsigned __int128>(1) << 127;
    test(high - 1);
    test(high);
    test(~static_cast<unsigned __int128>(0));
    test(static_cast<__int128>(high));
    test(static_cast<__int128>(high - 1));
    test(static_cast<__int128>(-42));
    test(static_cast<__int128>(42));
#endif
    for (bool neg : {false, true}) {
        const cpp_int lim = (cpp_int{1} << (T::bits - 1)) - (neg ? 0 : 1);
        CHECK(integer(detail::limit_magnitude_u256<T::bits>(neg)) == lim);
        CHECK(integer(detail::limit_magnitude_u256(T::bits, neg)) == lim);
        CHECK(integer(detail::max_integer_allowed_for_sign<T::bits, D>(neg)) == lim / scale_value);
        CHECK(integer(detail::max_integer_allowed<T::bits, D>(neg)) == lim / scale_value);
    }
    CHECK(integer(T::scale()) == scale_value);
    CHECK(integer(T::min().raw()) == -(cpp_int{1} << (T::bits - 1)));
    CHECK(integer(T::max().raw()) == (cpp_int{1} << (T::bits - 1)) - 1);
}

template<class T>
void constexpr_reference_runtime() {
    namespace ce = fixedwide::detail::ce;
    using Raw = typename T::raw_type;
    constexpr unsigned D = T::fractional_digits;
    const cpp_int scale_value = power10(D);
    auto vals = boundaries(T::bits);
    vals.resize(std::min<std::size_t>(vals.size(), 25));
    for (const auto& av : vals)
        for (const auto& bv : vals) {
            const auto a = from_integer<Raw>(av), b = from_integer<Raw>(bv);
            const auto c = from_integer<Raw>(bv + (bv == 0 ? 1 : 0));
            for (const auto mode : modes) {
                agrees(ce::mul<T::bits, D>(a, b, mode), rounded(av * bv, scale_value, mode, T::bits));
                agrees(ce::div<T::bits, D>(a, b, mode), rounded(av * scale_value, bv, mode, T::bits));
                agrees(ce::mul_div<T::bits, D>(a, b, c, mode), rounded(av * bv, integer(c), mode, T::bits));
            }
            const auto rem = ce::remainder<T::bits>(a, b);
            if (bv == 0)
                CHECK(!rem && rem.error() == ArithmeticError::division_by_zero);
            else {
                CHECK(rem);
                CHECK(integer(*rem) == av % bv);
            }
        }
    for (const auto& av : vals)
        for (auto mode : modes)
            for (unsigned digits : {0u, D, D + 1}) {
                const auto result = ce::quantize<T::bits, D>(from_integer<Raw>(av), digits, mode);
                if (digits > D) {
                    CHECK(!result && result.error() == ArithmeticError::invalid_precision);
                    continue;
                }
                const auto q = rounded(av, power10(D - digits), mode, T::bits);
                if (!q) {
                    CHECK(!result && result.error() == q.error());
                    continue;
                }
                agrees(result, rounded(*q * power10(D - digits), 1, Rounding::exact, T::bits));
            }
    const auto z = Raw{0};
    const auto res = ce::mul_div<T::bits, D>(z, z, z, Rounding::nearest_even);
    CHECK(!res && res.error() == ArithmeticError::division_by_zero);
}

void compatibility_and_kernels() {
    std::mt19937_64 rng(0x61726974686d6574ULL);
    auto vals = boundaries(64);
    for (unsigned i = 0; i < 100; ++i) vals.push_back(random_bits(rng, 64) - (cpp_int{1} << 63));
    for (const auto& a : vals)
        for (const auto& b : vals)
            for (auto mode : modes) {
                agrees(mul_wide(FP64::from_raw(a.convert_to<std::int64_t>()),
                                FP64::from_raw(b.convert_to<std::int64_t>()), mode),
                       rounded(a * b, power10(12), mode, 128));
            }
    for (const auto& v : boundaries(128))
        agrees(narrow(FP128::from_raw(from_integer<i128>(v))), rounded(v, 1, Rounding::exact, 64));
    // Call out-of-line kernels as well as public inline fast paths. The public
    // callers use both in real configurations; a test must not miss an entire
    // fallback because this host always selects the fast path.
    for (unsigned decimals = 0; decimals <= 38; ++decimals)
        for (unsigned i = 0; i < 30; ++i) {
            const cpp_int av = random_bits(rng, 128) - (cpp_int{1} << 127),
                          bv = random_bits(rng, 128) - (cpp_int{1} << 127);
            const auto a = from_integer<i128>(av), b = from_integer<i128>(bv);
            for (auto mode : modes) {
                agrees(detail::mul128_kernel(a, b, decimals, mode), rounded(av * bv, power10(decimals), mode, 128));
                agrees(detail::div128_kernel(a, b, decimals, mode), rounded(av * power10(decimals), bv, mode, 128));
                agrees(detail::mul_div128_kernel(a, b, 13, 0, mode), rounded(av * bv, 13, mode, 128));
            }
        }
    for (unsigned decimals = 0; decimals <= 18; ++decimals)
        for (auto mode : modes) {
            const auto s = power10(decimals).convert_to<std::int64_t>();
            for (std::int64_t a : {INT64_MIN, INT64_MAX, std::int64_t{-7}, std::int64_t{7}, std::int64_t{0}}) {
                agrees(detail::mul64_kernel(a, 3, s, mode), rounded(cpp_int{a} * 3, s, mode, 64));
                agrees(detail::div64_kernel(a, 3, s, mode), rounded(cpp_int{a} * s, 3, mode, 64));
                agrees(detail::mul_div64_kernel(a, -5, 3, mode), rounded(cpp_int{a} * -5, 3, mode, 64));
            }
        }
    CHECK(!detail::quantize64_kernel(0, 2, 3, Rounding::exact));
    CHECK(!detail::quantize128_kernel(i128{}, 2, 3, Rounding::exact));
    CHECK(!detail::quantize256_kernel(i256{}, 2, 3, Rounding::exact));
    // Wide widening construction is a lossless raw copy at a common scale.
    for (std::int8_t v : {std::int8_t{-128}, std::int8_t{0}, std::int8_t{127}}) {
        const auto a = Fixed8<2>::from_raw(v);
        CHECK(integer(Fixed16<2>{a}.raw()) == v);
        CHECK(integer(Fixed32<2>{a}.raw()) == v);
        CHECK(integer(Fixed64<2>{a}.raw()) == v);
        CHECK(integer(Fixed128<2>{a}.raw()) == v);
        CHECK(integer(Fixed256<2>{a}.raw()) == v);
        CHECK(integer(Fixed256<2>{Fixed128<2>{a}}.raw()) == v);
    }
}

template<class T>
void public_arithmetic() {
    const cpp_int s = power10(T::fractional_digits);
    auto vals = boundaries(T::bits);
    vals.resize(std::min<std::size_t>(vals.size(), 30));
    std::mt19937_64 rng(100 + T::bits + T::fractional_digits);
    for (unsigned i = 0; i < 50; ++i)
        vals.push_back(integer(from_integer<typename T::raw_type>(random_bits(rng, T::bits))));
    for (const auto& av : vals)
        for (const auto& bv : vals) {
            const auto a = T::from_raw(from_integer<typename T::raw_type>(av)),
                       b = T::from_raw(from_integer<typename T::raw_type>(bv));
            const auto c = T::from_raw(coverage_test::from_integer<typename T::raw_type>(cpp_int{13}));
            CHECK((a == b) == (av == bv));
            CHECK((a < b) == (av < bv));
            agrees(add(a, b), rounded(av + bv, 1, Rounding::exact, T::bits));
            agrees(sub(a, b), rounded(av - bv, 1, Rounding::exact, T::bits));
            agrees(negate(a), rounded(-av, 1, Rounding::exact, T::bits));
            agrees(abs(a), rounded(boost::multiprecision::abs(av), 1, Rounding::exact, T::bits));
            const auto rem = remainder(a, b);
            if (bv == 0)
                CHECK(!rem && rem.error() == ArithmeticError::division_by_zero);
            else {
                CHECK(rem);
                CHECK(integer(rem->raw()) == av % bv);
            }
            for (auto mode : modes) {
                agrees(mul(a, b, mode), rounded(av * bv, s, mode, T::bits));
                agrees(div(a, b, mode), rounded(av * s, bv, mode, T::bits));
                agrees(mul_div(a, b, c, mode), rounded(av * bv, 13, mode, T::bits));
                CHECK(!mul_div(a, b, T{}, mode));
            }
        }
    for (const auto& a : vals)
        for (auto mode : modes)
            for (unsigned d = 0; d <= T::fractional_digits + 1; ++d) {
                const auto got = quantize(T::from_raw(from_integer<typename T::raw_type>(a)), d, mode);
                if (d > T::fractional_digits) {
                    CHECK(!got && got.error() == ArithmeticError::invalid_precision);
                    continue;
                }
                const auto unit = power10(T::fractional_digits - d);
                auto q = rounded(a, unit, mode, T::bits);
                if (!q)
                    CHECK(!got && got.error() == q.error());
                else
                    agrees(got, rounded(*q * unit, 1, Rounding::exact, T::bits));
            }
}

#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
template<unsigned D>
void scaled_kernels() {
    auto vals = boundaries(128);
    for (const auto& a : vals)
        for (const auto& b : vals)
            for (auto mode : modes) {
                agrees(detail::mul128_scaled<D>(from_integer<i128>(a), from_integer<i128>(b), mode),
                       rounded(a * b, power10(D), mode, 128));
                agrees(detail::div128_scaled<D>(from_integer<i128>(a), from_integer<i128>(b), mode),
                       rounded(a * power10(D), b, mode, 128));
            }
}
#endif

void rounding_predicates() {
    for (std::uint64_t d = 1; d < 50; ++d)
        for (std::uint64_t r = 0; r < d; ++r)
            for (std::uint64_t q = 0; q < 8; ++q) {
                const bool tie_increment = r * 2 > d || (r * 2 == d && (q & 1) != 0);
                CHECK(detail_arith::nearest_even_inc(q, r, d) == tie_increment);
                for (bool negative : {false, true})
                    for (auto mode : modes) {
                        const bool expected = r != 0 && (mode == Rounding::floor          ? negative
                                                         : mode == Rounding::ceil         ? !negative
                                                         : mode == Rounding::nearest_even ? tie_increment
                                                         : mode == Rounding::nearest_away ? r * 2 >= d
                                                                                          : false);
                        CHECK(detail_arith::round_inc_u64(q, r, d, mode, negative) == expected);
                    }
                const auto rr = static_cast<std::int64_t>(r), dd = static_cast<std::int64_t>(d);
                CHECK(detail_arith::nearest_adj(static_cast<std::int64_t>(q), rr, dd, false) ==
                      static_cast<std::int64_t>(tie_increment));
                CHECK(detail_arith::nearest_adj(-static_cast<std::int64_t>(q), -rr, -dd, true) ==
                      -static_cast<std::int64_t>(tie_increment));
            }
    for (const auto& v : boundaries(128))
        CHECK(detail_arith::fits64(from_integer<i128>(v)) == (v >= INT64_MIN && v <= INT64_MAX));
}

void run_coverage_arithmetic() {
    constructors<Fixed8<0>>();
    constructors<Fixed8<2>>();
    constructors<Fixed16<4>>();
    constructors<Fixed32<9>>();
    constructors<Fixed64<0>>();
    constructors<Fixed64<12>>();
    constructors<Fixed128<0>>();
    constructors<Fixed128<12>>();
    constructors<Fixed256<0>>();
    constructors<Fixed256<76>>();
    constexpr_reference_runtime<Fixed8<2>>();
    constexpr_reference_runtime<Fixed64<12>>();
    constexpr_reference_runtime<Fixed128<38>>();
    constexpr_reference_runtime<Fixed256<76>>();
    CHECK(detail::limit_magnitude_u256(3, false).is_zero());
    CHECK(integer(detail::pow10_wide<wide::uint256>(77)) == power10(77));
    CHECK(integer(detail::pow10_wide<std::uint64_t>(19)) == power10(19));
    rounding_predicates();
    compatibility_and_kernels();
    // The widened quotient still needs its final nearest-even increment.
    for (std::int64_t delta = 0; delta < 50; ++delta)
        for (int sign : {-1, 1}) {
            const cpp_int a = cpp_int{INT64_MAX} - delta, b = cpp_int{INT64_MAX} - 17;
            agrees(mul(FP128::from_raw(from_integer<i128>(a * sign)), FP128::from_raw(from_integer<i128>(b))),
                   rounded(a * sign * b, power10(12), Rounding::nearest_even, 128));
        }
    public_arithmetic<Fixed8<2>>();
    public_arithmetic<Fixed16<4>>();
    public_arithmetic<Fixed32<9>>();
    public_arithmetic<Fixed64<0>>();
    public_arithmetic<Fixed64<12>>();
    public_arithmetic<Fixed64<18>>();
    public_arithmetic<Fixed128<0>>();
    public_arithmetic<Fixed128<12>>();
    public_arithmetic<Fixed128<38>>();
    public_arithmetic<Fixed256<18>>();
    public_arithmetic<Fixed256<76>>();
#if defined(__SIZEOF_INT128__) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    scaled_kernels<0>();
    scaled_kernels<12>();
    scaled_kernels<19>();
#endif
}
