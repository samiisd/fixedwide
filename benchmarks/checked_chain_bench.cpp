#include "checked_chain_cases.hpp"
#include "measurement.hpp"
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <string_view>
#include <vector>

namespace {
using namespace checked_chain_bench;
constexpr std::size_t data_size = 4096;
unsigned order = 0;

[[noreturn]] void fail(const char* message) {
    std::fprintf(stderr, "checked-chain benchmark: %s\n", message);
    std::exit(1);
}

template<class T>
void escape(const T& value) {
#if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__("" : : "r"(&value) : "memory");
#else
    // The benchmark CMake target currently supports GCC and Clang only.
    (void)value;
#endif
}

template<class Inputs, class Expected, class Accumulated, class Unchecked>
void bench(const char* name, const Inputs& inputs, Expected expected, Accumulated accumulated, Unchecked unchecked) {
    // Proves each checked intermediate succeeds BEFORE assuming the unchecked
    // preconditions. Timed regions contain neither validation nor input RNG.
    for (const auto& in : inputs) {
        const auto reference = expected(in);
        if (!reference) fail("fixture violates an unchecked precondition");
        if (accumulated(in) != reference || unchecked(in) != *reference) fail("policy results disagree");
    }
    const auto timed = [&](const char* policy, auto operation) {
        fixedwide_bench::measure(std::string(name) + "." + policy, [&](std::size_t count) {
            for (std::size_t i = 0; i < count; ++i) {
                const auto result = operation(inputs[i & (data_size - 1)]);
                escape(result);
            }
        });
    };
    if (order == 0) {
        timed("expected", expected);
        timed("accumulated", accumulated);
        timed("unchecked", unchecked);
    } else if (order == 1) {
        timed("accumulated", accumulated);
        timed("unchecked", unchecked);
        timed("expected", expected);
    } else {
        timed("unchecked", unchecked);
        timed("expected", expected);
        timed("accumulated", accumulated);
    }
}

std::size_t positive_number(std::string_view value) {
    std::size_t number{};
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), number);
    if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() || number == 0) fail("invalid count");
    return number;
}
} // namespace

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--iterations" && i + 1 < argc) {
            fixedwide_bench::iterations = positive_number(argv[++i]);
        } else if (arg == "--repetitions" && i + 1 < argc) {
            const auto count = positive_number(argv[++i]);
            if (count > std::numeric_limits<unsigned>::max()) fail("repetition count too large");
            fixedwide_bench::repetitions = static_cast<unsigned>(count);
        } else if (arg == "--order" && i + 1 < argc) {
            order = static_cast<unsigned>(positive_number(argv[++i]) % 3);
        } else if (arg == "--filter" && i + 1 < argc) {
            fixedwide_bench::filter = argv[++i];
        } else {
            fail("usage: fixedwide_checked_chain_bench [--iterations N] [--repetitions N] [--filter TEXT] [--order "
                 "1|2|3]");
        }
    }
    std::mt19937_64 rng(0xC4ECED68);
    std::vector<AddInput> add_inputs(data_size);
    std::vector<WideInput> wide_inputs(data_size);
    std::vector<FloatInput> float_inputs(data_size);
    for (std::size_t i = 0; i < data_size; ++i) {
        for (auto& v : add_inputs[i].v) {
            auto value = F128::from_raw(fw::i128{rng(), rng() & 0xFFFFFF});
            if ((rng() & 1u) != 0) value = *fw::negate(value);
            v = value;
        }
        auto wide = [&] { return F256::from_raw(fw::i256{rng(), (rng() & 0x3FF) + 1, 0, 0}); };
        wide_inputs[i] = {wide(), wide(), wide()};
        auto money = [&] {
            return F128::from_raw(fw::i128{static_cast<std::int64_t>(rng() % 1'000'000'000'000'000ULL + 1)});
        };
        float_inputs[i] = {0.5 + static_cast<double>(rng() % 1024) / 1024.0, money(), money(), money(), money()};
    }
    std::puts("workload,iterations,repetitions,min_ns,median_ns,p95_ns,max_ns,samples_ns");
    bench(
        "add8.Fixed128", add_inputs, [](const auto& in) { return add8_expected(in); },
        [](const auto& in) { return add8_accumulated(in); }, [](const auto& in) { return add8_unchecked(in); });
    bench(
        "mul_chain.Fixed256", wide_inputs, [](const auto& in) { return wide_expected(in); },
        [](const auto& in) { return wide_accumulated(in); }, [](const auto& in) { return wide_unchecked(in); });
    bench(
        "float_mixed", float_inputs, [](const auto& in) { return float_expected(in); },
        [](const auto& in) { return float_accumulated(in); }, [](const auto& in) { return float_unchecked(in); });
    std::fprintf(stderr, "validated %zu inputs per workload across all three policies\n", data_size);
}
