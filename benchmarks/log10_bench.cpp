#include <fixedwide/log10.hpp>
#include <fixedwide/floating.hpp>
#include "measurement.hpp"
#include <array>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace fw = fixedwide;
namespace bench = fixedwide_bench;
using U = fw::wide::uint256;

namespace {
constexpr std::size_t corpus_size = 1024;

std::uint64_t random_word(std::uint64_t& state) {
    state += 0x9e3779b97f4a7c15ULL;
    auto z = state;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

// Different algorithm from log10_floor: count exact powers of ten in order.
// A positive signed 256-bit raw value is below 10^77, which still fits U.
int reference(U raw) {
    int exponent = -1;
    for (U power(1); power <= raw; power = (power << 3) + (power << 1)) ++exponent;
    return exponent;
}

template<class F>
F from_unsigned(U value) {
    typename F::raw_type raw{};
    if constexpr (F::bits <= 64) {
        raw = static_cast<typename F::raw_type>(value.limbs[0]);
    } else if constexpr (F::bits == 128) {
        raw.low = value.limbs[0];
        raw.high = value.limbs[1];
    } else {
        for (std::size_t i = 0; i < 4; ++i) raw.limbs[i] = value.limbs[i];
    }
    return F::from_raw(raw);
}

template<class T>
void consume(T value) {
    // An observable sink on every iteration, and a memory clobber to prevent
    // hoisting a repeated traversal of the runtime-generated corpus. Timings
    // include loop/index/sink overhead; these are throughput, not latency.
    asm volatile("" : : "m"(value) : "memory");
}

template<class F>
void run(const char* type, const char* distribution) {
    const U maximum = (U(1) << static_cast<unsigned>(F::bits - 1)) - U(1);
    std::uint64_t state = 0x4c6f673130ULL + F::bits;
    std::vector<U> boundaries;
    for (U power(1); power <= maximum; power = (power << 3) + (power << 1)) {
        if (power != U(1)) boundaries.push_back(power - U(1));
        boundaries.push_back(power);
        if (power < maximum) boundaries.push_back(power + U(1));
    }
    std::array<F, corpus_size> fixed{};
    std::array<double, corpus_size> doubles{};
    std::array<long double, corpus_size> long_doubles{};
    std::size_t double_errors = 0;
    std::size_t long_double_errors = 0;
    for (std::size_t i = 0; i < corpus_size; ++i) {
        U raw;
        if (std::string_view(distribution) == "boundaries") {
            raw = boundaries[i % boundaries.size()];
        } else {
            for (auto& limb : raw.limbs) limb = random_word(state);
            raw = raw & maximum;
            if (std::string_view(distribution) == "log_uniform") {
                raw = raw >> static_cast<unsigned>(random_word(state) % (F::bits - 1));
            }
            if (raw.is_zero()) raw = U(1);
        }
        fixed[i] = from_unsigned<F>(raw);
        const int expected = reference(raw) - static_cast<int>(F::fractional_digits);
        const auto result = fw::log10_floor(fixed[i]);
        if (!result || *result != expected) std::abort();
        doubles[i] = fw::to_float<double>(fixed[i]);
        long_doubles[i] = fw::to_float<long double>(fixed[i]);
        double_errors += static_cast<std::size_t>(static_cast<int>(std::floor(std::log10(doubles[i]))) != expected);
        long_double_errors +=
            static_cast<std::size_t>(static_cast<int>(std::floor(std::log10(long_doubles[i]))) != expected);
    }
    // Deterministic Fisher-Yates, with identical permutations for every path.
    for (std::size_t i = corpus_size - 1; i != 0; --i) {
        const auto j = static_cast<std::size_t>(random_word(state) % (i + 1));
        std::swap(fixed[i], fixed[j]);
        std::swap(doubles[i], doubles[j]);
        std::swap(long_doubles[i], long_doubles[j]);
    }
    std::fprintf(stderr, "%s,%s,%zu,%zu,%zu\n", type, distribution, corpus_size, double_errors, long_double_errors);
    const auto prefix = std::string(type) + "." + distribution + ".";
    auto measure = [&](const char* name, auto operation) {
        bench::measure(prefix + name, [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) consume(operation(i & (corpus_size - 1)));
        });
    };
    measure("fixedwide", [&](std::size_t i) { return fw::log10_floor(fixed[i]).value_or(-1000); });
    measure("double_e2e", [&](std::size_t i) {
        return static_cast<int>(std::floor(std::log10(fw::to_float<double>(fixed[i]))));
    });
    measure("long_double_e2e", [&](std::size_t i) {
        return static_cast<int>(std::floor(std::log10(fw::to_float<long double>(fixed[i]))));
    });
    measure("double_floor_preconverted", [&](std::size_t i) {
        return static_cast<int>(std::floor(std::log10(doubles[i])));
    });
    measure("long_double_floor_preconverted", [&](std::size_t i) {
        return static_cast<int>(std::floor(std::log10(long_doubles[i])));
    });
    // Full fractional logarithms on already converted inputs. These do MORE
    // mathematical work and are separately labelled, not equivalent APIs.
    measure("double_log_only", [&](std::size_t i) { return std::log10(doubles[i]); });
    measure("long_double_log_only", [&](std::size_t i) { return std::log10(long_doubles[i]); });
}

template<class T>
bool parse_positive(const char* text, T& result) {
    const std::string_view input(text);
    const auto parsed = std::from_chars(input.data(), input.data() + input.size(), result);
    return parsed.ec == std::errc{} && parsed.ptr == input.data() + input.size() && result != 0;
}
} // namespace

int main(int argc, char** argv) {
    if (argc > 3 || (argc > 1 && !parse_positive(argv[1], bench::iterations)) ||
        (argc > 2 && !parse_positive(argv[2], bench::repetitions))) {
        std::fprintf(stderr, "usage: %s [positive_iterations [positive_repetitions]]\n", argv[0]);
        return 2;
    }
    std::fprintf(stderr, "# sizeof(long double)=%zu, digits=%d, radix=%d\n", sizeof(long double),
                 std::numeric_limits<long double>::digits, std::numeric_limits<long double>::radix);
    std::fputs("type,distribution,values,double_floor_mismatches,long_double_floor_mismatches\n", stderr);
    std::puts("name,iterations,repetitions,min_ns,median_ns,p95_ns,max_ns,samples_ns");
    for (const char* distribution : {"uniform", "log_uniform", "boundaries"}) {
        run<fw::Fixed32<4>>("Fixed32_4", distribution);
        run<fw::Fixed64<8>>("Fixed64_8", distribution);
        run<fw::Fixed128<12>>("Fixed128_12", distribution);
        run<fw::Fixed256<18>>("Fixed256_18", distribution);
    }
}
