#include <fixedwide/unchecked.hpp>
#include "check.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <bit>
#include <limits>
#include <random>

namespace fw = fixedwide;
using boost::multiprecision::cpp_int;

constexpr std::array modes{fw::Rounding::nearest_even, fw::Rounding::nearest_away, fw::Rounding::toward_zero,
                          fw::Rounding::floor,        fw::Rounding::ceil,         fw::Rounding::exact};

template<class F, class Operation>
void verify(std::int64_t a, std::int64_t b, std::int64_t c, fw::Rounding mode, Operation operation) {
    if (c == 0) return;
    cpp_int numerator = cpp_int(a) * b;
    cpp_int denominator = c;
    const bool negative = (numerator < 0) != (denominator < 0);
    if (numerator < 0) numerator = -numerator;
    if (denominator < 0) denominator = -denominator;
    cpp_int quotient = numerator / denominator;
    const cpp_int remainder = numerator % denominator;
    if (mode == fw::Rounding::exact && remainder != 0) return;
    bool increment = false;
    if (remainder != 0) {
        switch (mode) {
        case fw::Rounding::floor: increment = negative; break;
        case fw::Rounding::ceil: increment = !negative; break;
        case fw::Rounding::nearest_away: increment = 2 * remainder >= denominator; break;
        case fw::Rounding::nearest_even:
            increment = 2 * remainder > denominator || (2 * remainder == denominator && quotient % 2 != 0);
            break;
        default: break;
        }
    }
    if (increment) ++quotient;
    if (negative) quotient = -quotient;
    if (quotient < F::min().raw() || quotient > F::max().raw()) return;
    CHECK(cpp_int(operation().raw()) == quotient);
}

template<class F>
void triple(typename F::raw_type x, typename F::raw_type y, typename F::raw_type z) {
    const auto a = F::from_raw(x);
    const auto b = F::from_raw(y);
    const auto c = F::from_raw(z);
    for (auto mode : modes) {
        verify<F>(x, y, F::scale(), mode, [&] { return fw::unchecked::mul(a, b, mode); });
        verify<F>(x, F::scale(), y, mode, [&] { return fw::unchecked::div(a, b, mode); });
        verify<F>(x, y, z, mode, [&] { return fw::unchecked::mul_div(a, b, c, mode); });
    }
}

template<class F>
void sample() {
    using Raw = typename F::raw_type;
    constexpr Raw low = F::min().raw();
    constexpr Raw high = F::max().raw();
    const std::array<Raw, 13> edges{low,
                                  static_cast<Raw>(low + 1),
                                  static_cast<Raw>(low + 2),
                                  Raw{-10},
                                  Raw{-3},
                                  Raw{-2},
                                  Raw{-1},
                                  Raw{0},
                                  Raw{1},
                                  Raw{2},
                                  Raw{3},
                                  static_cast<Raw>(high - 1),
                                  high};
    for (auto a : edges)
        for (auto b : edges)
            for (auto c : edges) triple<F>(a, b, c);
    std::mt19937_64 rng(0x554e434845434b45ULL);
    for (int i = 0; i < 2000; ++i) {
        const auto a = static_cast<Raw>(rng());
        const auto b = static_cast<Raw>(rng());
        const auto c = static_cast<Raw>(rng());
        triple<F>(a, b, c);
    }
}

template<unsigned D>
void scales64() {
    sample<fw::Fixed64<D>>();
    if constexpr (D < 18) scales64<D + 1>();
}

int main() {
    scales64<0>();
    sample<fw::Fixed32<0>>();
    sample<fw::Fixed32<4>>();
    sample<fw::Fixed32<9>>();
    sample<fw::Fixed16<0>>();
    sample<fw::Fixed16<4>>();
    for (int a = -128; a <= 127; ++a) {
        for (int b = -128; b <= 127; ++b) {
            const auto x = static_cast<std::int8_t>(a);
            const auto y = static_cast<std::int8_t>(b);
            triple<fw::Fixed8<0>>(x, y, y);
            triple<fw::Fixed8<1>>(x, y, y);
            triple<fw::Fixed8<2>>(x, y, y);
        }
    }
    std::printf("unchecked oracle: %llu checks OK\n", static_cast<unsigned long long>(checks));
}
