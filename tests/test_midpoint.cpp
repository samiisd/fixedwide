#include "check.hpp"
#include <fixedwide/arithmetic.hpp>
#include <array>
#include <expected>
#include <type_traits>

namespace {
namespace fw = fixedwide;
using Mode = fw::Rounding;
constexpr std::array modes{Mode::toward_zero,  Mode::floor,        Mode::ceil,
                           Mode::nearest_even, Mode::nearest_away, Mode::exact};

template<class A, class B>
concept HasMidpoint = requires(A a, B b) { fw::midpoint(a, b); };
static_assert(!HasMidpoint<fw::Fixed64<8>, fw::Fixed64<12>>);
static_assert(!HasMidpoint<fw::Fixed64<8>, fw::Fixed128<8>>);
static_assert(!HasMidpoint<fw::Fixed64<8>, int>);

template<class F>
constexpr F raw(int value) {
    return F::from_raw(static_cast<typename F::raw_type>(value));
}

template<class F>
constexpr bool constant_checks() {
    static_assert(noexcept(fw::midpoint(F{}, F{})));
    static_assert(std::is_same_v<decltype(fw::midpoint(F{}, F{})), std::expected<F, fw::ArithmeticError>>);
    for (auto mode : modes) {
        if (fw::midpoint(F::min(), F::min(), mode) != F::min()) return false;
        if (fw::midpoint(F::max(), F::max(), mode) != F::max()) return false;
        if (fw::midpoint(raw<F>(1), raw<F>(3), mode) != raw<F>(2)) return false;
        const auto middle = fw::midpoint(F::min(), F::max(), mode);
        if (mode == Mode::exact) {
            if (middle || middle.error() != fw::ArithmeticError::inexact) return false;
        } else {
            const int expected = mode == Mode::floor || mode == Mode::nearest_away ? -1 : 0;
            if (middle != raw<F>(expected)) return false;
        }
        if (middle != fw::midpoint(F::max(), F::min(), mode)) return false;
    }
    return fw::midpoint(raw<F>(0), raw<F>(1)) == raw<F>(0) && fw::midpoint(raw<F>(1), raw<F>(2)) == raw<F>(2) &&
           fw::midpoint(raw<F>(-1), raw<F>(0)) == raw<F>(0) && fw::midpoint(raw<F>(-2), raw<F>(-1)) == raw<F>(-2);
}
static_assert(constant_checks<fw::Fixed8<2>>());
static_assert(constant_checks<fw::Fixed16<4>>());
static_assert(constant_checks<fw::Fixed32<9>>());
static_assert(constant_checks<fw::Fixed64<8>>());
static_assert(constant_checks<fw::Fixed64<12>>());
static_assert(constant_checks<fw::Fixed64<18>>());
static_assert(constant_checks<fw::Fixed128<38>>());
static_assert(constant_checks<fw::Fixed256<76>>());

// The sum fits int here. Independent quotient/remainder reference, not the
// bit identity used by midpoint; applies to negative half-units as well.
std::expected<int, fw::ArithmeticError> small_reference(int a, int b, Mode mode) {
    const int sum = a + b;
    int q = sum / 2;
    if (sum % 2 == 0) return q;
    switch (mode) {
    case Mode::toward_zero: break;
    case Mode::floor:
        if (sum < 0) --q;
        break;
    case Mode::ceil:
        if (sum > 0) ++q;
        break;
    case Mode::nearest_even:
        if (q % 2 != 0) q += sum < 0 ? -1 : 1;
        break;
    case Mode::nearest_away: q += sum < 0 ? -1 : 1; break;
    case Mode::exact: return std::unexpected(fw::ArithmeticError::inexact);
    }
    return q;
}

template<class F>
void small_range(int low, int high) {
    for (int a = low; a <= high; ++a) {
        for (int b = low; b <= high; ++b) {
            for (auto mode : modes) {
                const auto expected = small_reference(a, b, mode);
                const auto result = fw::midpoint(raw<F>(a), raw<F>(b), mode);
                CHECK(result.has_value() == expected.has_value());
                if (result)
                    CHECK(*result == raw<F>(*expected));
                else
                    CHECK(result.error() == expected.error());
            }
        }
    }
}

template<class F>
void boundaries() {
    using Raw = typename F::raw_type;
    const auto lo1 = F::from_raw(static_cast<Raw>(F::min().raw() + Raw{1}));
    const auto hi1 = F::from_raw(static_cast<Raw>(F::max().raw() - Raw{1}));
    const std::array values{F::min(), lo1, raw<F>(-2), raw<F>(-1), F{}, raw<F>(1), raw<F>(2), hi1, F::max()};
    for (auto mode : modes) {
        for (auto a : values) {
            CHECK(fw::midpoint(a, a, mode) == a);
            for (auto b : values) {
                const auto result = fw::midpoint(a, b, mode);
                CHECK(result == fw::midpoint(b, a, mode));
                if (result)
                    CHECK(*result >= (a < b ? a : b) && *result <= (a < b ? b : a));
                else
                    CHECK(mode == Mode::exact && result.error() == fw::ArithmeticError::inexact);
            }
        }
        const auto low_tie = fw::midpoint(F::min(), lo1, mode);
        const auto high_tie = fw::midpoint(hi1, F::max(), mode);
        if (mode == Mode::exact) {
            CHECK(!low_tie && low_tie.error() == fw::ArithmeticError::inexact);
            CHECK(!high_tie && high_tie.error() == fw::ArithmeticError::inexact);
        } else {
            CHECK(low_tie == (mode == Mode::ceil || mode == Mode::toward_zero ? lo1 : F::min()));
            CHECK(high_tie == (mode == Mode::ceil || mode == Mode::nearest_away ? F::max() : hi1));
        }
    }
    // Exercise carries across every limb without a wider production type.
    for (unsigned bit = 1; bit < F::bits - 1; ++bit) {
        const Raw p = static_cast<Raw>(Raw{1} << bit);
        for (const Raw center : {p, static_cast<Raw>(-p)}) {
            const F a = F::from_raw(static_cast<Raw>(center - Raw{1}));
            const F b = F::from_raw(static_cast<Raw>(center + Raw{1}));
            for (auto mode : modes) CHECK(fw::midpoint(a, b, mode) == F::from_raw(center));
        }
    }
    CHECK(constant_checks<F>());
}
} // namespace

int main() {
    small_range<fw::Fixed8<0>>(-128, 127);
    small_range<fw::Fixed8<1>>(-128, 127);
    small_range<fw::Fixed8<2>>(-128, 127);
    small_range<fw::Fixed16<4>>(-16, 16);
    small_range<fw::Fixed32<9>>(-16, 16);
    small_range<fw::Fixed64<8>>(-16, 16);
    small_range<fw::Fixed64<12>>(-16, 16);
    small_range<fw::Fixed128<38>>(-16, 16);
    small_range<fw::Fixed256<76>>(-16, 16);
    boundaries<fw::Fixed8<2>>();
    boundaries<fw::Fixed16<4>>();
    boundaries<fw::Fixed32<9>>();
    boundaries<fw::Fixed64<8>>();
    boundaries<fw::Fixed64<12>>();
    boundaries<fw::Fixed64<18>>();
    boundaries<fw::Fixed128<38>>();
    boundaries<fw::Fixed256<76>>();
    // The motivating regression: truncating each half first loses one raw unit.
    using F = fw::Fixed64<8>;
    CHECK(fw::midpoint(F::from_raw(100000001), F::from_raw(100000003), Mode::toward_zero) == F::from_raw(100000002));
    std::printf("midpoint: %llu checks passed\n", static_cast<unsigned long long>(checks));
}
