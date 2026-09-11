// Independent operations (throughput), not a dependency-chain latency test.
// All fixtures and both reference implementations are checked against exact
// Boost arithmetic before timing; every timed result contributes to a checksum.
#include "measurement.hpp"
#include "../tests/coverage_support.hpp"
#include <fixedwide/mixed.hpp>
#include <charconv>
#include <string_view>

namespace {
namespace fw = fixedwide;
namespace ref = coverage_test;
using Mode = fw::Rounding;
constexpr std::size_t count = 256;
constexpr std::size_t mask = count - 1;
volatile std::uint64_t sink = 0;

#if defined(_MSC_VER)
#define FW_NOINLINE __declspec(noinline)
#else
#define FW_NOINLINE __attribute__((noinline))
#endif

template<class F>
std::uint64_t digest(const std::expected<F, fw::ArithmeticError>& result) {
    if (!result) return UINT64_MAX - static_cast<std::uint64_t>(result.error());
    const auto value = result->raw();
    if constexpr (F::bits <= 64)
        return static_cast<std::uint64_t>(value);
    else if constexpr (F::bits == 128)
        return value.low ^ value.high;
    else
        return value.limbs[0] ^ value.limbs[1] ^ value.limbs[2] ^ value.limbs[3];
}

template<class F>
struct Fixture {
    std::array<F, count> a{}, b{};
    std::array<Mode, count> rounding{};
    explicit Fixture(bool full_width) {
        std::mt19937_64 rng{0x6d6964706f696e74ULL};
        const ref::cpp_int limit = ref::cpp_int{1} << F::bits;
        const ref::cpp_int scale = ref::power10(F::fractional_digits);
        for (std::size_t i = 0; i < count; ++i) {
            auto random = [&]() -> ref::cpp_int {
                if (!full_width) return scale + rng() % 100000000;
                ref::cpp_int v = ref::random_bits(rng, F::bits);
                if (v >= limit / 2) v -= limit;
                return v;
            };
            ref::cpp_int x = random(), y = full_width ? random() : x + i % 4;
            a[i] = F::from_raw(ref::from_integer<typename F::raw_type>(x));
            b[i] = F::from_raw(ref::from_integer<typename F::raw_type>(y));
            rounding[i] = ref::modes[i % ref::modes.size()];
        }
        if (full_width) {
            a[0] = b[0] = F::min();
            a[1] = b[1] = F::max();
            a[2] = F::min();
            b[2] = F::max();
            a[3] = F::max();
            b[3] = F::min();
        }
    }
};

enum class Impl { midpoint, widened, native128 };

template<Impl Which, class F>
auto calculate(F a, F b, Mode rounding) {
    if constexpr (Which == Impl::midpoint) {
        return fw::midpoint(a, b, rounding);
    } else if constexpr (Which == Impl::widened) {
        using Wide = fw::basic_fixed<F::bits * 2, F::fractional_digits>;
        const auto sum = fw::add(Wide{a}, Wide{b});
        if (!sum) return std::expected<F, fw::ArithmeticError>{std::unexpected(sum.error())};
        return fw::div_to<F>(*sum, fw::Fixed64<0>::from_raw(2), rounding);
    } else {
#if defined(__SIZEOF_INT128__)
        static_assert(F::bits == 64);
        // Strong native comparator: one widened sum and one rounding, not the
        // incorrect split-halves expression that motivated this feature.
        const __int128 sum = static_cast<__int128>(a.raw()) + b.raw();
        __int128 q = sum / 2;
        if (sum % 2 != 0) {
            switch (rounding) {
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
            case Mode::exact:
                return std::expected<F, fw::ArithmeticError>{std::unexpected(fw::ArithmeticError::inexact)};
            }
        }
        return std::expected<F, fw::ArithmeticError>{F::from_raw(static_cast<std::int64_t>(q))};
#endif
    }
}

template<Impl Which, Mode M, bool Runtime, class F>
FW_NOINLINE std::uint64_t loop(const Fixture<F>& fixture, std::size_t iterations) {
    std::uint64_t sum = 0;
    for (std::size_t i = 0; i < iterations; ++i) {
        const auto j = i & mask;
        auto value = digest(calculate<Which>(fixture.a[j], fixture.b[j], Runtime ? fixture.rounding[j] : M));
#if defined(__GNUC__) || defined(__clang__)
        __asm__ volatile("" : "+r"(value) : : "memory");
#else
        sink = value;
#endif
        sum += value;
    }
    sink = sum;
    return sum;
}

template<Impl Which, Mode M, bool Runtime = false, class F>
void row(const Fixture<F>& fixture, const std::string& prefix, const char* mode) {
    std::array<std::uint64_t, count> expected{};
    for (std::size_t i = 0; i < count; ++i) {
        const auto rounding = Runtime ? fixture.rounding[i] : M;
        const auto a = ref::integer(fixture.a[i].raw()), b = ref::integer(fixture.b[i].raw());
        const auto want = ref::rounded(a + b, 2, rounding, F::bits);
        const auto actual = calculate<Which>(fixture.a[i], fixture.b[i], rounding);
        ref::agrees(actual, want);
        const std::expected<F, fw::ArithmeticError> result =
            want ? std::expected<F, fw::ArithmeticError>{F::from_raw(ref::from_integer<typename F::raw_type>(*want))}
                 : std::expected<F, fw::ArithmeticError>{std::unexpected(want.error())};
        expected[i] = digest(result);
    }
    auto checksum = [&](std::size_t n) {
        std::uint64_t total = 0, tail = 0;
        for (std::size_t i = 0; i < count; ++i) {
            total += expected[i];
            if (i < (n & mask)) tail += expected[i];
        }
        return total * (n / count) + tail;
    };
    CHECK((loop<Which, M, Runtime>(fixture, count + 17) == checksum(count + 17)));
    const char* implementation = Which == Impl::midpoint  ? "midpoint"
                                 : Which == Impl::widened ? "widened"
                                                          : "native128";
    fixedwide_bench::measure(prefix + "." + mode + "." + implementation,
                             [&](std::size_t n) { loop<Which, M, Runtime>(fixture, n); });
    CHECK(sink == checksum(fixedwide_bench::iterations));
}

template<Mode M, bool Runtime = false, class F>
void compare(const Fixture<F>& fixture, const std::string& name, const char* mode) {
    row<Impl::midpoint, M, Runtime>(fixture, name, mode);
    if constexpr (F::bits < 256) row<Impl::widened, M, Runtime>(fixture, name, mode);
#if defined(__SIZEOF_INT128__)
    if constexpr (F::bits == 64) row<Impl::native128, M, Runtime>(fixture, name, mode);
#endif
}

template<class F>
void run() {
    for (bool full_width : {false, true}) {
        const Fixture<F> fixture{full_width};
        const auto name = "Fixed" + std::to_string(F::bits) + ".d" + std::to_string(F::fractional_digits) +
                          (full_width ? ".full-width" : ".quote");
        compare<Mode::nearest_even>(fixture, name, "nearest_even");
        compare<Mode::toward_zero>(fixture, name, "toward_zero");
        compare<Mode::floor>(fixture, name, "floor");
        compare<Mode::ceil>(fixture, name, "ceil");
        compare<Mode::nearest_away>(fixture, name, "nearest_away");
        compare<Mode::exact>(fixture, name, "exact");
        compare<Mode::nearest_even, true>(fixture, name, "runtime");
    }
}

std::size_t positive(const char* text) {
    const std::string_view input{text};
    std::size_t value = 0;
    const auto [end, error] = std::from_chars(input.data(), input.data() + input.size(), value);
    CHECK(error == std::errc{} && end == input.data() + input.size() && value != 0);
    return value;
}
} // namespace

int main(int argc, char** argv) {
    if (argc > 3) {
        std::fprintf(stderr, "usage: %s [iterations [repetitions]]\n", argv[0]);
        return 2;
    }
    fixedwide_bench::iterations = argc > 1 ? positive(argv[1]) : 262144;
    const auto repetitions = argc > 2 ? positive(argv[2]) : 7;
    CHECK(repetitions <= 1000);
    fixedwide_bench::repetitions = static_cast<unsigned>(repetitions);
    std::puts("name,iterations,repetitions,min_ns,median_ns,p95_ns,max_ns,samples_ns");
    run<fw::Fixed64<8>>();
    run<fw::Fixed64<12>>();
    run<fw::Fixed128<12>>();
    run<fw::Fixed256<12>>();
    std::fprintf(stderr, "midpoint benchmark: %llu oracle/checksum checks passed\n",
                 static_cast<unsigned long long>(checks));
}
