#include "coverage_support.hpp"
#include "to_floating_oracle.hpp"
#include <fixedwide/floating.hpp>
#include <fixedwide/format.hpp>
#include <fixedwide/iostream.hpp>
#include <fixedwide/string.hpp>
#include <cmath>
#include <format>
#include <sstream>
#include <string>

using namespace coverage_test;
using namespace fixedwide;

// Recover every bit of the binary significand without converting through a
// decimal string or a narrower floating type. This is independent of the
// production converter's integer mantissa extraction.
template<class F>
std::pair<cpp_int, cpp_int> rational(F value) {
    int exponent = 0;
    F fraction = std::frexp(std::abs(value), &exponent);
    cpp_int n = 0, d = 1;
    for (int bit = 0; bit < std::numeric_limits<F>::digits; ++bit) {
        fraction *= F{2};
        n <<= 1;
        if (fraction >= F{1}) {
            ++n;
            fraction -= F{1};
        }
    }
    exponent -= std::numeric_limits<F>::digits;
    if (exponent >= 0)
        n <<= static_cast<unsigned>(exponent);
    else
        d <<= static_cast<unsigned>(-exponent);
    if (std::signbit(value)) n = -n;
    return {n, d};
}

template<class T, class F>
void floating() {
    std::vector<F> values{F{0},
                          -F{0},
                          F{1},
                          F{-1},
                          F{0.125},
                          F{-0.125},
                          F{1.25},
                          F{-1.25},
                          F{2.5},
                          F{-2.5},
                          F{3.5},
                          F{-3.5},
                          F{0.1},
                          F{-0.1},
                          std::numeric_limits<F>::min(),
                          std::numeric_limits<F>::denorm_min(),
                          -std::numeric_limits<F>::denorm_min(),
                          std::numeric_limits<F>::max(),
                          -std::numeric_limits<F>::max()};
    for (int power : {-1024, -256, -128, -64, -20, 0, 20, 63, 64, 126, 127, 128, 254, 255, 256, 1024}) {
        const F v = std::ldexp(F{1}, power);
        if (std::isfinite(v) && v != 0) {
            values.push_back(v);
            values.push_back(-v);
            values.push_back(std::nextafter(v, F{0}));
        }
    }
    for (F v : values) {
        const auto [n, d] = rational(v);
        for (auto mode : modes)
            agrees(from_float<T>(v, mode), rounded(n * power10(T::fractional_digits), d, mode, T::bits));
    }
    for (F v : {std::numeric_limits<F>::infinity(), -std::numeric_limits<F>::infinity(),
                std::numeric_limits<F>::quiet_NaN()}) {
        const auto r = from_float<T>(v);
        CHECK(!r && r.error() == ArithmeticError::invalid_value);
        const auto k = detail::from_float_kernel(v, T::fractional_digits, Rounding::nearest_even, T::bits);
        CHECK(!k && k.error() == ArithmeticError::invalid_value);
    }
    const auto zero = to_float<F>(T{});
    CHECK(zero == 0);
    const auto sample = T::from_raw(typename T::raw_type{125});
    const F expected = F{125} / std::pow(F{10}, static_cast<int>(T::fractional_digits));
    const F actual = to_float<F>(sample);
    CHECK(std::abs(actual - expected) <= std::abs(expected) * std::numeric_limits<F>::epsilon() * 4);
    const auto negative = T::from_raw(typename T::raw_type{-125});
    CHECK(to_float<F>(negative) == -actual);
}

// Coverage profiles deliberately come from test_coverage alone. Keep focused
// reverse-conversion cases here as well as the standalone exhaustive suite;
// otherwise its passing tests contribute no counters to the coverage gate.
namespace coverage_floating {
template<class T, class F>
void reverse() {
    if constexpr (detail::use_fast_to_float_v<F, T::bits> && floating_test::supported<F>) {
        const cpp_int limit = cpp_int{1} << (T::bits - 1);
        auto both = [&](const cpp_int& magnitude) {
            for (bool negative : {false, true}) {
                const cpp_int raw = negative ? cpp_int{-magnitude} : magnitude;
                if (raw < -limit || raw >= limit) continue;
                const T value = T::from_raw(from_integer<typename T::raw_type>(raw));
                // This reference divides arbitrary-precision integers at final
                // output spacing; it does not use the production reciprocal,
                // normalization or encoder, nor the inaccurate legacy kernel.
                CHECK(
                    to_floating_test::same(to_float<F>(value), to_floating_test::oracle<F>(raw, T::fractional_digits)));
            }
        };
        both(0);
        both(limit); // Signed minimum: the positive counterpart is out of range.
        // Each normalization boundary with/without discarded nonzero bits.
        // raw=1 at D=38 also exercises binary32 subnormal output spacing.
        for (unsigned bit = 0; bit < T::bits; ++bit)
            for (int delta : {-1, 0, 1}) both((cpp_int{1} << bit) + delta);

        // Exactly halfway, both parities, and a carry into the next exponent.
        // Multiply by 5^D so these are exact rational ties, not FP estimates.
        constexpr int precision = std::numeric_limits<F>::digits;
        const cpp_int five = power10(T::fractional_digits) >> T::fractional_digits;
        for (const cpp_int& significand : std::array<cpp_int, 3>{
                 cpp_int{1} << (precision - 1), (cpp_int{1} << (precision - 1)) + 1, (cpp_int{1} << precision) - 1}) {
            const cpp_int tie = (2 * significand + 1) * five;
            for (int delta : {-1, 0, 1}) both(tie + delta);
        }
        std::mt19937_64 rng(0x746f666c6f6174ULL + T::bits * 100 + T::fractional_digits);
        for (unsigned i = 0; i < 128; ++i) both(random_bits(rng, T::bits - 1));
    }
}

template<class F>
void reverse_format() {
    // Representative scales cover constant 64-bit division, the native divide,
    // both reciprocal sizes, and wide-precision encoders on their native ABI.
    reverse<Fixed64<0>, F>();
    reverse<Fixed64<8>, F>();
    reverse<Fixed64<12>, F>();
    reverse<Fixed64<18>, F>();
    reverse<Fixed128<0>, F>();
    reverse<Fixed128<8>, F>();
    reverse<Fixed128<12>, F>();
    reverse<Fixed128<27>, F>();
    reverse<Fixed128<28>, F>();
    reverse<Fixed128<38>, F>();

    // <=128-bit inputs no longer visit these compiled wrappers. Fixed256
    // remains a public fallback; use exactly representable values so its
    // known extra rounding is not used as the correctness oracle above.
    CHECK(to_float<F>(Fixed256<0>{}) == F{0});
    for (int raw : {-125, 125})
        CHECK(to_float<F>(Fixed256<2>::from_raw(wide::int256{raw})) == static_cast<F>(raw) / F{100});
    for (unsigned bit : {0u, 64u, 128u, 192u}) {
        const auto raw = from_integer<wide::int256>(cpp_int{1} << bit);
        const F expected = std::ldexp(F{1}, static_cast<int>(bit));
        CHECK(to_float<F>(Fixed256<0>::from_raw(raw)) == expected);
        CHECK(to_float<F>(Fixed256<0>::from_raw(-raw)) == -expected);
    }
}

void shift_width_input() {
    // In the 64-bit binary64/D=0 intermediate these inputs need a right
    // shift of exactly 64. Retain a nonzero remainder even with quotient=0.
    if constexpr (detail::use_fast_from_float_v<Fixed64<0>, double>) {
        for (double value : {0x1p-12, -0x1p-12}) {
            const auto [n, d] = rational(value);
            for (auto mode : modes) agrees(from_float<Fixed64<0>>(value, mode), rounded(n, d, mode, 64));
        }
    }
}
} // namespace coverage_floating

void formats_and_streams() {
    using T = Fixed64<4>;
    const T v = T::from_raw(123456);
    CHECK(std::format("{}", v) == "12.3456");
    CHECK(std::format("{:f}", v) == "12.3456");
    CHECK(std::format("{:.0}", v) == "12");
    CHECK(std::format("{:1}", v) == "12.3456");
    CHECK(std::format("{:12}", v) == "     12.3456");
    CHECK(std::format("{:<12.2f}", v) == "12.35       ");
    CHECK(std::format("{:>12.2f}", v) == "       12.35");
    CHECK(std::format("{:*^12.2f}", v) == "***12.35****");
    CHECK(std::format("{:*>12.2f}", v) == "*******12.35");
    CHECK(std::format("{:*<12.2f}", v) == "12.35*******");
    for (std::string_view spec : {"{:.}", "{:.f}", "{:.5}", "{:q}", "{:+}", "{:fX}"}) {
        bool threw = false;
        try {
            (void)std::vformat(spec, std::make_format_args(v));
        } catch (const std::format_error&) {
            threw = true;
        }
        CHECK(threw);
    }
    // Exercise parser ranges that terminate before a closing brace too.
    for (std::string_view spec : {"", "f", ".2", "<", "*>8.2f"}) {
        std::formatter<T> formatter;
        std::format_parse_context context(spec);
        CHECK(formatter.parse(context) == context.end());
    }
    for (std::string_view spec : {".", ".5", "q"}) {
        bool threw = false;
        try {
            std::formatter<T> formatter;
            std::format_parse_context context(spec);
            (void)formatter.parse(context);
        } catch (const std::format_error&) {
            threw = true;
        }
        CHECK(threw);
    }
    std::ostringstream output;
    output << v;
    CHECK(output.str() == "12.3456");
    T target = T::from_raw(99);
    std::istringstream valid("-1.2500 3.0000");
    valid >> target;
    CHECK(target == T::from_raw(-12500));
    valid >> target;
    CHECK(target == T::from_raw(30000));
    for (const char* text : {"", "not-a-number", "1.00001", "99999999999999999999999999"}) {
        std::istringstream input(text);
        input >> target;
        CHECK(input.fail());
        CHECK(target == T::from_raw(30000));
    }
    CHECK(from_double64(1.25) == FP64::from_raw(1'250'000'000'000LL));
    CHECK(from_double128(-1.25) == FP128::from_raw(i128{-1'250'000'000'000LL}));
    CHECK(from_double<T>(1.25) == T::from_raw(12500));
    CHECK(to_double(T::from_raw(12500)) == 1.25);
}
void run_coverage_adapters() {
    floating<Fixed8<2>, float>();
    floating<Fixed64<0>, double>();
    floating<Fixed64<12>, float>();
    floating<Fixed128<38>, double>();
    floating<Fixed256<76>, long double>();
    coverage_floating::reverse_format<float>();
    coverage_floating::reverse_format<double>();
    coverage_floating::reverse_format<long double>();
    coverage_floating::shift_width_input();
}

void run_coverage_format_adapters() {
    formats_and_streams();
}
