// The general side calls the compiled kernels directly: calling the public API
// on both sides would silently stop measuring a baseline once it is optimized.
#include <fixedwide/mixed.hpp>
#include <benchmark/benchmark.h>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace fw = fixedwide;
using Money = fw::Fixed128<12>;
using Price = fw::Fixed64<10>;
using Whole = fw::Fixed128<0>;
using Wide = fw::Fixed256<12>;
using Raw = fw::wide::int128;

namespace {
constexpr std::size_t corpus_size = 1U << 16;
std::size_t validations = 0;
enum class ResultKind { success, overflow, mixed };

[[noreturn]] void fail(const char* message) {
    std::fprintf(stderr, "VALIDATION FAILED: %s\n", message);
    std::exit(1);
}

template<class Dest>
std::expected<Dest, fw::ArithmeticError> convert(std::expected<fw::wide::int256, fw::ArithmeticError> result) {
    if (!result) return std::unexpected(result.error());
    return fw::detail::from_int256_raw<Dest>(*result);
}

template<class Right>
[[gnu::noinline]] std::expected<Money, fw::ArithmeticError> general_product(Money a, Right b) {
    return convert<Money>(fw::detail::mixed_mul_kernel(fw::detail::to_int256_raw(a.raw()), 12,
                                                       fw::detail::to_int256_raw(b.raw()), Right::fractional_digits, 12,
                                                       fw::Rounding::nearest_even, 128));
}

template<class Right>
[[gnu::noinline]] std::expected<Money, fw::ArithmeticError> public_product(Money a, Right b) {
    return fw::mul_to<Money>(a, b, fw::Rounding::nearest_even);
}

template<class Dest, bool Subtract, class A, class B>
[[gnu::noinline]] std::expected<Dest, fw::ArithmeticError> general_add_sub(A a, B b) {
    return convert<Dest>(fw::detail::mixed_add_sub_kernel(
        fw::detail::to_int256_raw(a.raw()), A::fractional_digits, fw::detail::to_int256_raw(b.raw()),
        B::fractional_digits, Subtract, Dest::fractional_digits, fw::Rounding::nearest_even, Dest::bits));
}

template<class Dest, bool Subtract, class A, class B>
[[gnu::noinline]] std::expected<Dest, fw::ArithmeticError> public_add_sub(A a, B b) {
    if constexpr (Subtract)
        return fw::sub_to<Dest>(a, b, fw::Rounding::nearest_even);
    else
        return fw::add_to<Dest>(a, b, fw::Rounding::nearest_even);
}

[[gnu::noinline]] std::expected<Money, fw::ArithmeticError> general_cast(Wide a) {
    return convert<Money>(fw::detail::mixed_cast_kernel(a.raw(), 12, 12, fw::Rounding::exact, 128));
}

[[gnu::noinline]] std::expected<Money, fw::ArithmeticError> public_cast(Wide a) {
    return fw::fixed_cast<Money>(a, fw::Rounding::exact);
}

[[gnu::noinline]] std::expected<Money, fw::ArithmeticError> general_replace(Money total, Money previous, Money next) {
    const auto removed = fw::detail::mixed_add_sub_kernel(fw::detail::to_int256_raw(total.raw()), 12,
                                                          fw::detail::to_int256_raw(previous.raw()), 12, true, 12,
                                                          fw::Rounding::exact, 256);
    if (!removed) return std::unexpected(removed.error());
    const auto replaced = fw::detail::mixed_add_sub_kernel(*removed, 12, fw::detail::to_int256_raw(next.raw()), 12,
                                                           false, 12, fw::Rounding::exact, 256);
    if (!replaced) return std::unexpected(replaced.error());
    return convert<Money>(fw::detail::mixed_cast_kernel(*replaced, 12, 12, fw::Rounding::exact, 128));
}

[[gnu::noinline]] std::expected<Money, fw::ArithmeticError> public_replace(Money total, Money previous, Money next) {
    const auto removed = fw::sub_to<Wide>(total, previous, fw::Rounding::exact);
    if (!removed) return std::unexpected(removed.error());
    const auto replaced = fw::add_to<Wide>(*removed, next, fw::Rounding::exact);
    if (!replaced) return std::unexpected(replaced.error());
    return fw::fixed_cast<Money>(*replaced, fw::Rounding::exact);
}

bool fits_int128(fw::wide::int256 value) {
    const std::uint64_t extension = (value.limbs[1] >> 63) != 0 ? ~std::uint64_t{0} : 0;
    return value.limbs[2] == extension && value.limbs[3] == extension;
}

template<class Right>
bool product_fits(Money left, Right right) {
    return fits_int128(fw::detail::to_int256_raw(left.raw()) * fw::detail::to_int256_raw(right.raw()));
}

template<class Operation>
void register_operation(std::string name, Operation operation) {
    benchmark::RegisterBenchmark(name, [operation](benchmark::State& state) {
        std::size_t index = 0;
        for (auto _ : state) {
            auto result = operation(index++ & (corpus_size - 1));
            benchmark::DoNotOptimize(result);
        }
        state.SetItemsProcessed(state.iterations());
    })->Unit(benchmark::kNanosecond);
}

template<class General, class Public>
void register_pair(const char* name, General general, Public optimized, ResultKind expected = ResultKind::success) {
    std::size_t successes = 0;
    std::size_t overflows = 0;
    for (std::size_t i = 0; i < corpus_size; ++i) {
        const auto reference = general(i);
        const auto result = optimized(i);
        if (reference.has_value() != result.has_value() ||
            (reference ? reference->raw() != result->raw() : reference.error() != result.error())) {
            std::fprintf(stderr, "%s sample=%zu\n", name, i);
            fail("public result differs from the general kernel (value or error)");
        }
        if ((expected == ResultKind::success && !reference) ||
            (expected == ResultKind::overflow && (reference || reference.error() != fw::ArithmeticError::overflow)))
            fail("fixture produced an unexpected result kind");
        successes += static_cast<std::size_t>(reference.has_value());
        overflows += static_cast<std::size_t>(!reference && reference.error() == fw::ArithmeticError::overflow);
        ++validations;
    }
    std::fprintf(stderr, "%s,%zu,%zu,%zu,0\n", name, corpus_size, successes, overflows);
    register_operation(std::string(name) + "/general", general);
    register_operation(std::string(name) + "/public", optimized);
}

template<class Right>
void register_products(const char* name, const std::vector<Money>& left, const std::vector<Right>& right,
                       bool expect_fallback, ResultKind expected = ResultKind::success) {
    for (std::size_t i = 0; i < corpus_size; ++i)
        if (product_fits(left[i], right[i]) == expect_fallback) fail("incorrect product fixture magnitude");
    // Registered callbacks own an immutable snapshot, and both sides read the
    // exact same allocation. Later fixture generation cannot mutate their data.
    const auto a = std::make_shared<const std::vector<Money>>(left);
    const auto b = std::make_shared<const std::vector<Right>>(right);
    register_pair(
        name, [a, b](std::size_t i) { return general_product((*a)[i], (*b)[i]); },
        [a, b](std::size_t i) { return public_product((*a)[i], (*b)[i]); }, expected);
}

template<class Dest, bool Subtract, class A, class B>
void register_add_sub(const char* name, const std::vector<A>& left, const std::vector<B>& right) {
    const auto a = std::make_shared<const std::vector<A>>(left);
    const auto b = std::make_shared<const std::vector<B>>(right);
    register_pair(
        name, [a, b](std::size_t i) { return general_add_sub<Dest, Subtract>((*a)[i], (*b)[i]); },
        [a, b](std::size_t i) { return public_add_sub<Dest, Subtract>((*a)[i], (*b)[i]); });
}

void register_cast(const char* name, const std::vector<Wide>& values, ResultKind expected = ResultKind::success) {
    const auto data = std::make_shared<const std::vector<Wide>>(values);
    register_pair(
        name, [data](std::size_t i) { return general_cast((*data)[i]); },
        [data](std::size_t i) { return public_cast((*data)[i]); }, expected);
}

Raw signed_raw(std::mt19937_64& rng, std::uint64_t bound, Raw factor) {
    const Raw value = Raw(rng() % (bound + 1)) * factor;
    return (rng() & 1U) != 0 ? -value : value;
}

Raw random_raw(std::mt19937_64& rng) {
    // Sequence draws explicitly: constructor argument evaluation order differs
    // between GCC and Clang, which otherwise changes the supposedly same corpus.
    const auto low = rng();
    const auto high = rng();
    return Raw(low, high);
}
} // namespace

int main(int argc, char** argv) {
    bool validate_only = false;
    int remaining = 1;
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--validate-only")
            validate_only = true;
        else
            argv[remaining++] = argv[i];
    }
    argc = remaining;
    argv[argc] = nullptr;
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 2;
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    constexpr const char* backend = "native";
#else
    constexpr const char* backend = "portable";
#endif
    std::fprintf(stderr, "# compiler=%s backend=%s seed=42 samples=%zu\n", __VERSION__, backend, corpus_size);
    std::fputs("workload,samples,successes,overflows,mismatches\n", stderr);
    benchmark::AddCustomContext("compiler", __VERSION__);
    benchmark::AddCustomContext("backend", backend);
    benchmark::AddCustomContext("seed", "42");
    benchmark::AddCustomContext("samples_per_workload", std::to_string(corpus_size));
    benchmark::AddCustomContext("google_benchmark", FIXEDWIDE_GOOGLE_BENCHMARK_VERSION);

    std::mt19937_64 rng(42);
    std::vector<Money> slope(corpus_size), amount(corpus_size), fx(corpus_size), total(corpus_size),
        previous(corpus_size), next(corpus_size);
    std::vector<Price> price(corpus_size);
    std::vector<Wide> wide(corpus_size);
    std::vector<Whole> whole(corpus_size);
    for (std::size_t i = 0; i < corpus_size; ++i) {
        // 0.01..1000.00 lots, each lot 100,000 units, with scale 12.
        slope[i] = Money::from_raw(Raw(rng() % 100'000 + 1) * Raw(1'000'000'000'000'000ULL));
        // +/-500 at scale 10; +/-10^7 and +/-10^9 at scale 12.
        price[i] = Price::from_raw(static_cast<std::int64_t>(rng() % 10'000'000'000'001ULL) - 5'000'000'000'000LL);
        amount[i] = Money::from_raw(signed_raw(rng, 10'000'000'000'000'000ULL, Raw(1000)));
        fx[i] = Money::from_raw(Raw(rng() % 4'800'000'000'001ULL + 200'000'000'000ULL));
        total[i] = Money::from_raw(signed_raw(rng, 1'000'000'000'000'000'000ULL, Raw(1000)));
        wide[i] = Wide::from_raw(fw::detail::to_int256_raw(total[i].raw()));
    }
    // Every source has been initialized before a neighbor is read.
    for (std::size_t i = 0; i < corpus_size; ++i) {
        previous[i] = amount[i];
        next[i] = amount[(i + 1) & (corpus_size - 1)];
    }
    register_products("mul_price.realistic", slope, price, false);
    register_products("mul_fx.realistic", amount, fx, false);
    const auto replace = [&](const char* name, ResultKind expected = ResultKind::success) {
        const auto t = std::make_shared<const std::vector<Money>>(total);
        const auto p = std::make_shared<const std::vector<Money>>(previous);
        const auto n = std::make_shared<const std::vector<Money>>(next);
        register_pair(
            name, [t, p, n](std::size_t i) { return general_replace((*t)[i], (*p)[i], (*n)[i]); },
            [t, p, n](std::size_t i) { return public_replace((*t)[i], (*p)[i], (*n)[i]); }, expected);
    };
    replace("replace.realistic");
    register_add_sub<Money, false>("add_mixed.realistic", amount, price);
    register_add_sub<Money, true>("sub_mixed.realistic", amount, price);
    register_cast("cast.sign_extension", wide);

    for (std::size_t i = 0; i < corpus_size; ++i) {
        // Products need 136 or 161 bits but the rescaled destination fits.
        const auto low = rng();
        const Raw a(low, (std::uint64_t{1} << 26) + (rng() & 0x3ffU));
        slope[i] = Money::from_raw((i & 1U) != 0 ? -a : a);
        price[i] = Price::from_raw(static_cast<std::int64_t>((std::uint64_t{1} << 45) + (rng() & 0xffffU)));
        const auto fx_low = rng();
        fx[i] = Money::from_raw(Raw(fx_low, 64 + (rng() & 7U)));
    }
    register_products("mul_price.fallback_success", slope, price, true);
    register_products("mul_fx.fallback_success", slope, fx, true);
    for (std::size_t i = 0; i < corpus_size; ++i) {
        const auto low = rng();
        const Raw a(low, (std::uint64_t{1} << 61) | (rng() >> 3));
        slope[i] = Money::from_raw((i & 1U) != 0 ? -a : a);
        fx[i] = Money::from_raw(a);
    }
    register_products("mul_price.fallback_overflow", slope, price, true, ResultKind::overflow);
    register_products("mul_fx.fallback_overflow", slope, fx, true, ResultKind::overflow);

    for (std::size_t i = 0; i < corpus_size; ++i) {
        const Raw high = Raw::max() - Raw(rng() & 0xffffU);
        const Raw delta(rng(), std::uint64_t{1} << 36);
        total[i] = Money::from_raw((i & 1U) != 0 ? -high : high);
        previous[i] = Money::from_raw((i & 1U) != 0 ? delta : -delta);
        next[i] = previous[i];
        const auto removed = fw::detail::to_int256_raw(total[i].raw()) - fw::detail::to_int256_raw(previous[i].raw());
        if (fits_int128(removed)) fail("wide-cancellation fixture does not overflow int128");
        wide[i] = Wide::from_raw(removed);
        whole[i] = Whole::from_raw(delta);
    }
    replace("replace.wide_cancellation");
    register_cast("cast.outside_int128", wide, ResultKind::overflow);
    // Alignment to scale 12 overflows int128; the final scale-zero answer fits.
    register_add_sub<Whole, false>("add_mixed.fallback_success", total, whole);
    register_add_sub<Whole, true>("sub_mixed.fallback_success", total, whole);
    for (std::size_t i = 0; i < corpus_size; ++i) {
        total[i] = Money::from_raw(random_raw(rng));
        previous[i] = Money::from_raw(random_raw(rng));
        next[i] = Money::from_raw(random_raw(rng));
    }
    replace("replace.full_range", ResultKind::mixed);
    benchmark::AddCustomContext("validated_comparisons", std::to_string(validations));
    if (!validate_only) benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
}
