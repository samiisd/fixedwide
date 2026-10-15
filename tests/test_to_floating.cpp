#include "check.hpp"
#include "to_floating_oracle.hpp"
#include <fixedwide/floating.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <cstring>
#include <random>
#include <cstdlib>

using namespace fixedwide;
using boost::multiprecision::cpp_int;
namespace {
std::uint64_t comparisons = 0;
std::uint64_t legacy_differences = 0;
int last_legacy_precision = 0;
using namespace to_floating_test;
template<typename Float, typename Target>
void compare(const cpp_int& raw) {
    const cpp_int limit = cpp_int{1} << (Target::bits - 1);
    if (raw < -limit || raw >= limit) return;
    const auto input = from_integer<Target>(raw);
    const Float expected = oracle<Float>(raw, Target::fractional_digits);
    const Float actual = to_float<Float>(input);
    ++comparisons;
    if (!same(actual, expected)) {
        std::fprintf(stderr, "to_float mismatch: bits=%zu D=%u precision=%d raw=%s\n", Target::bits,
                     Target::fractional_digits, std::numeric_limits<Float>::digits, raw.str().c_str());
        std::abort();
    }
#ifndef FIXEDWIDE_TEST_ALTERNATE_LDBL
    const Float legacy =
        detail::to_float_kernel(detail::to_int256_raw(input.raw()), Target::fractional_digits, Float{});
    if (!same(legacy, expected)) {
        if (last_legacy_precision != std::numeric_limits<Float>::digits)
            std::printf("legacy differs: bits=%zu D=%u precision=%d raw=%s\n", Target::bits, Target::fractional_digits,
                        std::numeric_limits<Float>::digits, raw.str().c_str());
        last_legacy_precision = std::numeric_limits<Float>::digits;
        ++legacy_differences;
    }
#endif
}

template<typename Float, typename Target>
void check_type(unsigned samples) {
    static_assert(detail::use_fast_to_float_v<Float, Target::bits>);
    const auto both = [](const cpp_int& raw) {
        compare<Float, Target>(raw);
        compare<Float, Target>(-raw);
    };
    const cpp_int scale = pow10(Target::fractional_digits);
    both(0);
    for (int i = 0; i <= 16; ++i) {
        both(cpp_int{i});
        both(scale + i);
        both(scale - i);
    }
    // All raw bit widths, signed minima and neighbours, both sides of every
    // normalization/limb boundary; no construction through double.
    for (unsigned bit = 0; bit < Target::bits; ++bit)
        for (int offset : {-2, -1, 0, 1, 2}) both((cpp_int{1} << bit) + offset);
    // Exact halfway cases, both output parities, carry-out and their neighbours.
    constexpr int p = std::numeric_limits<Float>::digits;
    cpp_int five = scale >> Target::fractional_digits;
    for (cpp_int significand :
         std::array<cpp_int, 3>{cpp_int{1} << (p - 1), (cpp_int{1} << (p - 1)) + 1, (cpp_int{1} << p) - 1}) {
        const cpp_int tie = (2 * significand + 1) * five;
        for (int offset : {-1, 0, 1}) both(tie + offset);
    }
    std::mt19937_64 rng(0x746f666c6f6174ULL + Target::bits * 100 + Target::fractional_digits);
    const cpp_int maximum = (cpp_int{1} << (Target::bits - 1)) - 1;
    for (unsigned i = 0; i < samples; ++i) {
        cpp_int raw = (cpp_int{rng()} << 64) | rng();
        raw &= maximum;
        both(raw);
        // Log-uniform magnitudes as well as full-width random limbs.
        both(raw >> (rng() % Target::bits));
    }
}

template<typename Float, std::size_t Bits, std::size_t... Ds>
void check_scales(unsigned samples, std::index_sequence<Ds...>) {
    (check_type<Float, basic_fixed<Bits, static_cast<unsigned>(Ds)>>(samples), ...);
}

template<typename Float>
void check_format(unsigned samples) {
    if constexpr (floating_test::supported<Float>) {
        check_scales<Float, 8>(samples, std::make_index_sequence<3>{});
        check_scales<Float, 16>(samples, std::make_index_sequence<5>{});
        check_scales<Float, 32>(samples, std::make_index_sequence<10>{});
        check_scales<Float, 64>(samples, std::make_index_sequence<19>{});
        check_scales<Float, 128>(samples, std::make_index_sequence<39>{});
        // Exhaust every signed 8-bit raw value at every valid scale.
        for (int i = -128; i < 128; ++i) {
            compare<Float, Fixed8<0>>(cpp_int{i});
            compare<Float, Fixed8<1>>(cpp_int{i});
            compare<Float, Fixed8<2>>(cpp_int{i});
        }
#ifndef FIXEDWIDE_TEST_ALTERNATE_LDBL
        // The fallback is still the original kernel, not an accidental fast path.
        static_assert(!detail::use_fast_to_float_v<Float, 256>);
        const auto value = Fixed256<76>::from_raw(wide::int256::max());
        CHECK(same(to_float<Float>(value), detail::to_float_kernel(value.raw(), 76, Float{})));
#endif
    }
}
} // namespace

int main() {
    unsigned samples = 128;
    if (const char* env = std::getenv("FIXEDWIDE_TO_FLOAT_SAMPLES"))
        samples = static_cast<unsigned>(std::strtoul(env, nullptr, 10));
    check_format<float>(samples);
    check_format<double>(samples);
    check_format<long double>(samples);
    std::printf("to_float: %llu independent rational comparisons, %llu legacy differences; %llu checks\n",
                static_cast<unsigned long long>(comparisons), static_cast<unsigned long long>(legacy_differences),
                static_cast<unsigned long long>(checks));
}
