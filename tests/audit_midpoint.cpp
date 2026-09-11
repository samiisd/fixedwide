#include "coverage_support.hpp"

namespace {
namespace fw = fixedwide;
namespace ref = coverage_test;

template<class F>
void audit() {
    using Raw = typename F::raw_type;
    auto verify = [](const ref::cpp_int& a, const ref::cpp_int& b) {
        const F x = F::from_raw(ref::from_integer<Raw>(a));
        const F y = F::from_raw(ref::from_integer<Raw>(b));
        for (auto mode : ref::modes) {
            const auto result = fw::midpoint(x, y, mode);
            ref::agrees(result, ref::rounded(a + b, 2, mode, F::bits));
            CHECK(result == fw::midpoint(y, x, mode));
            if (result) CHECK(*result >= (x < y ? x : y) && *result <= (x < y ? y : x));
        }
        CHECK(fw::midpoint(x, y) == fw::midpoint(x, y, fw::Rounding::nearest_even));
    };
    const auto edges = ref::boundaries(F::bits);
    for (const auto& a : edges) for (const auto& b : edges) verify(a, b);
    std::mt19937_64 rng{0x6d6964706f696e74ULL + F::bits + F::fractional_digits};
    auto random = [&] {
        ref::cpp_int value = ref::random_bits(rng, F::bits);
        if ((value & (ref::cpp_int{1} << (F::bits - 1))) != 0) value -= ref::cpp_int{1} << F::bits;
        return value;
    };
    for (unsigned i = 0; i < 10000; ++i) {
        const auto a = random(), b = random();
        verify(a, b);
    }
}
} // namespace

int main() {
    audit<fw::Fixed8<0>>();
    audit<fw::Fixed8<2>>();
    audit<fw::Fixed16<0>>();
    audit<fw::Fixed16<4>>();
    audit<fw::Fixed32<0>>();
    audit<fw::Fixed32<9>>();
    audit<fw::Fixed64<0>>();
    audit<fw::Fixed64<8>>();
    audit<fw::Fixed64<12>>();
    audit<fw::Fixed64<18>>();
    audit<fw::Fixed128<0>>();
    audit<fw::Fixed128<8>>();
    audit<fw::Fixed128<12>>();
    audit<fw::Fixed128<38>>();
    audit<fw::Fixed256<0>>();
    audit<fw::Fixed256<8>>();
    audit<fw::Fixed256<12>>();
    audit<fw::Fixed256<76>>();
    std::printf("midpoint oracle: %llu checks passed\n", static_cast<unsigned long long>(checks));
}
