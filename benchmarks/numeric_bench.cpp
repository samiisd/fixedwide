// Scalar API throughput and dependency-chain latency. Conversions, random input
// generation and allocation are outside timing. Raw binary floating operations
// do NOT offer the same precision, overflow or decimal-rounding contract.
#include <fixedwide/arithmetic.hpp>
#include "measurement.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace fw = fixedwide;
constexpr std::size_t data_size = 256;
using fixedwide_bench::iterations; using fixedwide_bench::repetitions;
using fixedwide_bench::filter; using fixedwide_bench::measure;
enum class Op { compare, add, sub, mul, div, mul_div };

// Keep floating results in FP registers, integers in integer registers. No
// type-specific checksum, implicit FP->integer conversion or forced float spill.
// Every loop also gets a memory barrier so input loads cannot be hoisted away.
inline void escape(float v) { __asm__ __volatile__("" : : "x"(v) : "memory"); }
inline void escape(double v) { __asm__ __volatile__("" : : "x"(v) : "memory"); }
inline void escape(std::int64_t v) { __asm__ __volatile__("" : : "r"(v) : "memory"); }
inline void escape(fw::i128 v) {
    __asm__ __volatile__("" : : "r"(static_cast<std::uint64_t>(v)),
        "r"(static_cast<std::uint64_t>(static_cast<fw::u128>(v) >> 64)) : "memory");
}
inline void escape(fw::FP64 v) { escape(v.raw()); }
inline void escape(fw::FP128 v) { escape(v.raw()); }

template<class T> T make(std::int64_t raw) {
    if constexpr (std::is_floating_point_v<T>) return static_cast<T>(static_cast<long double>(raw) / fw::scale);
    else return T::from_raw(raw);
}
template<class T> T require(std::expected<T, fw::ArithmeticError> value) {
    // Retain the real checked API contract. All timed inputs are valid, but
    // removing this check would incorrectly benchmark ignoring error results.
    if (!value) std::abort();
    return *value;
}
template<class T, Op op> T calculate(T a, T b, T c) {
    if constexpr (std::is_floating_point_v<T>) {
        if constexpr (op == Op::add) return a + b;
        if constexpr (op == Op::sub) return a - b;
        if constexpr (op == Op::mul) return a * b;
        if constexpr (op == Op::div) return a / b;
        if constexpr (op == Op::mul_div) return (a * b) / c;
    } else {
        if constexpr (op == Op::add) return require(fw::add(a, b));
        if constexpr (op == Op::sub) return require(fw::sub(a, b));
        if constexpr (op == Op::mul) return require(fw::mul(a, b, fw::Rounding::toward_zero));
        if constexpr (op == Op::div) return require(fw::div(a, b, fw::Rounding::toward_zero));
        if constexpr (op == Op::mul_div) return require(fw::mul_div(a, b, c, fw::Rounding::toward_zero));
    }
}

template<class T> struct Inputs {
    std::array<T, data_size> a, b, c;
    Inputs() {
        std::mt19937_64 rng(0x826041);
        for (std::size_t i = 0; i < data_size; ++i) {
            auto raw = static_cast<std::int64_t>(rng() % (1000 * fw::scale)) + fw::scale;
            if (i % 4 == 0) raw = -raw;
            a[i] = make<T>(raw);
            b[i] = make<T>(static_cast<std::int64_t>(rng() % (10 * fw::scale)) + fw::scale);
            c[i] = make<T>(static_cast<std::int64_t>(rng() % (10 * fw::scale)) + fw::scale);
        }
    }
};
template<class T, Op op> void operation(const char* type, const char* operation) {
    Inputs<T> inputs;
    const std::string suffix = std::string(type) + "." + operation;
    measure("throughput." + suffix, [&](std::size_t count) {
        for (std::size_t i = 0; i < count; ++i) {
            const auto j = i & (data_size - 1);
            if constexpr (op == Op::compare) escape(static_cast<std::int64_t>(inputs.a[j] < inputs.b[j]));
            else escape(calculate<T, op>(inputs.a[j], inputs.b[j], inputs.c[j]));
        }
    });
    if constexpr (op != Op::compare) {
        // Bounded exact alternating chain: +/-0.125 for add/sub; * or / 2,0.5
        // for mul/div. This does not drift to zero/infinity or converge to a
        // fixed point. These are intentionally NOT the random throughput inputs.
        const std::array<T, 2> operand = op == Op::add || op == Op::sub
            ? std::array{make<T>(fw::scale / 8), make<T>(-fw::scale / 8)}
            : std::array{make<T>(2 * fw::scale), make<T>(fw::scale / 2)};
        const auto one = make<T>(fw::scale);
        measure("latency." + suffix, [&](std::size_t count) {
            T state = make<T>(fw::scale + fw::scale / 4);
            for (std::size_t i = 0; i < count; ++i) {
                state = calculate<T, op>(state, operand[i & 1], one);
                escape(state);
            }
        });
    }
}
// Runtime-loaded, non-binary-exact factors. Each pair approximately cancels
// so a long chain stays bounded; the library still rounds at EVERY step.
// mul_div swaps arbitrary numerator/denominator pairs, rather than dividing
// by economic one as the original chain does.
template<class T, Op op> void inexact_chain(const char* type, const char* name) {
    std::array<T, data_size> factors{}, denominators{};
    std::mt19937_64 rng(0x3001208);
    for (std::size_t i = 0; i < data_size; i += 2) {
        const auto f = fw::scale + static_cast<std::int64_t>(rng() % (fw::scale / 100)) + 1;
        if constexpr (op == Op::mul_div) {
            const auto d = fw::scale + static_cast<std::int64_t>(rng() % (fw::scale / 100)) + 1;
            factors[i] = make<T>(f); denominators[i] = make<T>(d);
            factors[i+1] = make<T>(d); denominators[i+1] = make<T>(f);
        } else {
            const auto inverse = static_cast<std::int64_t>((fw::i128{fw::scale} * fw::scale) / f);
            factors[i] = make<T>(f); factors[i+1] = make<T>(inverse);
            denominators[i] = denominators[i+1] = make<T>(fw::scale);
        }
    }
    measure(std::string("inexact_chain.") + type + "." + name, [&](std::size_t count) {
        T state = make<T>(fw::scale + fw::scale / 4);
        for (std::size_t i = 0; i < count; ++i) {
            const auto j = i & (data_size - 1);
            state = calculate<T, op>(state, factors[j], denominators[j]);
            escape(state);
        }
    });
}
template<Op op> void inexact_chain(const char* name) {
    inexact_chain<float, op>("float", name); inexact_chain<double, op>("double", name);
    inexact_chain<fw::FP64, op>("FP64", name); inexact_chain<fw::FP128, op>("FP128", name);
}
template<Op op> void operation(const char* name) {
    operation<float, op>("float", name);
    operation<double, op>("double", name);
    operation<fw::FP64, op>("FP64", name);
    operation<fw::FP128, op>("FP128", name);
}
template<class T> void floor(const char* name) {
    Inputs<T> inputs;
    measure(std::string("throughput.") + name + ".read", [&](std::size_t count) {
        for (std::size_t i = 0; i < count; ++i) escape(inputs.a[i & (data_size - 1)]);
    });
}
int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--iterations" && i+1 < argc) iterations = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--repetitions" && i+1 < argc) repetitions = static_cast<unsigned>(std::strtoul(argv[++i], nullptr, 10));
        else if (arg == "--filter" && i+1 < argc) filter = argv[++i];
        else return 2;
    }
    if (iterations < 4096 || repetitions < 3 || repetitions > 1000) return 2;
    static_assert(std::numeric_limits<float>::is_iec559 && std::numeric_limits<float>::digits == 24);
    static_assert(std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::digits == 53);
    std::fprintf(stderr, "compiler=%s; digits=%u; FP128 size/alignment=%zu/%zu; float/double mantissa=24/53; scalar; no FMA contraction; seed=0x826041\n",
        __VERSION__, fw::fractional_digits, sizeof(fw::FP128), alignof(fw::FP128));
    std::puts("name,iterations,repetitions,min_ns,median_ns,p95_batch_ns,max_ns,samples_ns");
    floor<float>("float"); floor<double>("double"); floor<fw::FP64>("FP64"); floor<fw::FP128>("FP128");
    operation<Op::compare>("compare"); operation<Op::add>("add"); operation<Op::sub>("sub");
    operation<Op::mul>("mul"); operation<Op::div>("div"); operation<Op::mul_div>("mul_div");
    inexact_chain<Op::mul>("mul"); inexact_chain<Op::div>("div"); inexact_chain<Op::mul_div>("mul_div");
}
