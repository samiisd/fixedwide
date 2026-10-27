#include <benchmark/benchmark.h>
#include <array>
#include <cstdint>
#include <limits>
using I = __int128;
using U = unsigned __int128;
extern "C" bool signed_builtin(I, I, I*);
extern "C" bool unsigned_builtin(I, I, I*);
extern "C" bool split_unsigned(I, I, I*);
extern "C" bool narrowed_operand(I, I, I*);
extern "C" bool split_builtin(I, I, I*);
extern "C" bool reject_large(I, I, I*);
extern "C" bool separate_small(I, I, I*);
extern "C" bool partial_product(I, I, I*);
extern "C" bool scale_builtin(I, I*);
extern "C" bool scale_bound(I, I*);
struct Pair {
    I a;
    I b;
};
using Pairs = std::array<Pair, 1024>;
static std::uint64_t next(std::uint64_t& x) {
    x += 0x9e3779b97f4a7c15ULL;
    auto z = x;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}
static Pairs make_pairs(int category) {
    std::uint64_t seed = 71;
    Pairs result{};
    for (auto& pair : result) {
        U a, b;
        if (category == 0) {
            a = next(seed) % 10000000000000000000ULL;
            b = next(seed) % 5000000000000ULL;
        } // FX
        else if (category == 1) {
            a = U(next(seed) % 10000000000000000000ULL) * 10;
            b = next(seed) % 5000000000000ULL;
        } // price
        else if (category == 2) {
            a = (U(next(seed)) << 64) | next(seed);
            b = (U(next(seed)) << 64) | next(seed);
        } // both wide
        else {
            a = (U(next(seed)) << 64) | next(seed);
            b = next(seed) % 1000000000000ULL + 1;
        } // one wide
        pair = {I(a >> 1), I(b >> 1)};
        if (next(seed) & 1) pair.a = -pair.a;
        if (next(seed) & 1) pair.b = -pair.b;
    }
    return result;
}
template<bool (*Multiply)(I, I, I*)>
static void product(benchmark::State& state) {
    const auto pairs = make_pairs(static_cast<int>(state.range(0)));
    I output = 0;
    std::uint64_t sink = 0;
    for (auto _ : state) {
        for (const auto& p : pairs) {
            bool ok = Multiply(p.a, p.b, &output);
            benchmark::DoNotOptimize(ok);
            if (ok) sink += std::uint64_t(U(output));
        }
        benchmark::DoNotOptimize(sink);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(pairs.size()));
}
template<bool (*Scale)(I, I*)>
static void scale(benchmark::State& state) {
    const auto pairs = make_pairs(static_cast<int>(state.range(0)));
    I output = 0;
    std::uint64_t sink = 0;
    for (auto _ : state) {
        for (const auto& p : pairs) {
            bool ok = Scale(p.a, &output);
            benchmark::DoNotOptimize(ok);
            if (ok) sink += std::uint64_t(U(output));
        }
        benchmark::DoNotOptimize(sink);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(pairs.size()));
}
#define PRODUCTS(F) BENCHMARK_TEMPLATE(product, F)->ArgName("dataset")->DenseRange(0, 3)
PRODUCTS(signed_builtin);
PRODUCTS(unsigned_builtin);
PRODUCTS(split_unsigned);
PRODUCTS(narrowed_operand);
PRODUCTS(split_builtin);
PRODUCTS(reject_large);
PRODUCTS(separate_small);
PRODUCTS(partial_product);
BENCHMARK_TEMPLATE(scale, scale_builtin)->ArgName("dataset")->Arg(0)->Arg(2);
BENCHMARK_TEMPLATE(scale, scale_bound)->ArgName("dataset")->Arg(0)->Arg(2);
BENCHMARK_MAIN();
