#include "coverage_support.hpp"
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
}

void run_coverage_format_adapters() {
    formats_and_streams();
}
