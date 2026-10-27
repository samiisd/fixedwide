// The general side calls the compiled kernels directly: calling the public API
// on both sides would silently stop measuring a baseline once it is optimized.
#include <fixedwide/mixed.hpp>
#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
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
std::size_t iterations = 1U << 20;
unsigned repetitions = 7;
std::uint64_t sink = 0;
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

template<class F>
double time_loop(F operation, std::size_t count) {
    // Both paths include the same indexing, expected handling and unsigned sink.
    // The memory clobber prevents hoisting repeated traversals of the corpus;
    // the operation wrappers remain noinline on both sides.
    std::uint64_t checksum = 0;
    const auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < count; ++i) {
        const auto result = operation(i & (corpus_size - 1));
        checksum +=
            result ? (result->raw().low ^ result->raw().high) : (static_cast<std::uint64_t>(result.error()) + 1);
        asm volatile("" : "+r"(checksum) : : "memory");
    }
    const auto end = std::chrono::steady_clock::now();
    sink += checksum; // Deliberately modulo 2^64, even for full-range fixtures.
    return std::chrono::duration<double, std::nano>(end - start).count() / static_cast<double>(count);
}

double median(std::vector<double> values) {
    std::sort(values.begin(), values.end());
    const std::size_t middle = values.size() / 2;
    return (values.size() & 1U) != 0 ? values[middle] : (values[middle - 1] + values[middle]) / 2;
}

void print_samples(const std::vector<double>& values) {
    std::putchar('"');
    for (std::size_t i = 0; i < values.size(); ++i) std::printf("%s%.6f", i == 0 ? "" : ";", values[i]);
    std::putchar('"');
}

template<class General, class Public>
void measure_pair(const char* name, General general, Public optimized, ResultKind expected = ResultKind::success) {
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
    }
    std::fprintf(stderr, "%s,%zu,%zu,%zu,0\n", name, corpus_size, successes, overflows);
    (void)time_loop(general, corpus_size);
    (void)time_loop(optimized, corpus_size);
    std::vector<double> general_samples(repetitions), public_samples(repetitions);
    for (unsigned repeat = 0; repeat < repetitions; ++repeat) {
        // Alternate pair order to reduce systematic frequency/temperature drift.
        if ((repeat & 1U) == 0) {
            general_samples[repeat] = time_loop(general, iterations);
            public_samples[repeat] = time_loop(optimized, iterations);
        } else {
            public_samples[repeat] = time_loop(optimized, iterations);
            general_samples[repeat] = time_loop(general, iterations);
        }
    }
    const double general_ns = median(general_samples);
    const double public_ns = median(public_samples);
    std::printf("%s,%zu,%u,%.6f,%.6f,%.6f,%.3f,%.3f,", name, iterations, repetitions, general_ns, public_ns,
                public_ns - general_ns, 100.0 * (public_ns / general_ns - 1.0), general_ns / public_ns);
    print_samples(general_samples);
    std::putchar(',');
    print_samples(public_samples);
    std::putchar('\n');
    std::fflush(stdout);
}

template<class Right>
void measure_products(const char* name, const std::vector<Money>& left, const std::vector<Right>& right,
                      bool expect_fallback, ResultKind expected = ResultKind::success) {
    for (std::size_t i = 0; i < corpus_size; ++i)
        if (product_fits(left[i], right[i]) == expect_fallback) fail("incorrect product fixture magnitude");
    measure_pair(
        name, [&](std::size_t i) { return general_product(left[i], right[i]); },
        [&](std::size_t i) { return public_product(left[i], right[i]); }, expected);
}

template<class T>
bool parse_positive(const char* text, T& result) {
    const std::string_view value(text);
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
    return parsed.ec == std::errc{} && parsed.ptr == value.data() + value.size() && result != 0;
}

Raw signed_raw(std::mt19937_64& rng, std::uint64_t bound, Raw factor) {
    const Raw value = Raw(rng() % (bound + 1)) * factor;
    return (rng() & 1U) != 0 ? -value : value;
}
} // namespace

int main(int argc, char** argv) {
    if (argc > 3 || (argc > 1 && !parse_positive(argv[1], iterations)) ||
        (argc > 2 && !parse_positive(argv[2], repetitions))) {
        std::fprintf(stderr, "usage: %s [positive_iterations [positive_repetitions]]\n", argv[0]);
        return 2;
    }
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    constexpr const char* backend = "native";
#else
    constexpr const char* backend = "portable";
#endif
    std::fprintf(stderr, "# compiler=%s backend=%s seed=42 samples=%zu\n", __VERSION__, backend, corpus_size);
    std::fputs("workload,samples,successes,overflows,mismatches\n", stderr);
    std::puts("workload,iterations,repetitions,general_median_ns,public_median_ns,delta_ns,delta_percent,speedup,"
              "general_samples_ns,public_samples_ns");

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
    measure_products("mul_price.realistic", slope, price, false);
    measure_products("mul_fx.realistic", amount, fx, false);
    const auto replace = [&](const char* name, ResultKind expected = ResultKind::success) {
        measure_pair(
            name, [&](std::size_t i) { return general_replace(total[i], previous[i], next[i]); },
            [&](std::size_t i) { return public_replace(total[i], previous[i], next[i]); }, expected);
    };
    replace("replace.realistic");
    measure_pair(
        "add_mixed.realistic", [&](std::size_t i) { return general_add_sub<Money, false>(amount[i], price[i]); },
        [&](std::size_t i) { return public_add_sub<Money, false>(amount[i], price[i]); });
    measure_pair(
        "sub_mixed.realistic", [&](std::size_t i) { return general_add_sub<Money, true>(amount[i], price[i]); },
        [&](std::size_t i) { return public_add_sub<Money, true>(amount[i], price[i]); });
    measure_pair(
        "cast.sign_extension", [&](std::size_t i) { return general_cast(wide[i]); },
        [&](std::size_t i) { return public_cast(wide[i]); });

    for (std::size_t i = 0; i < corpus_size; ++i) {
        // Products need 136 or 161 bits but the rescaled destination fits.
        const Raw a(rng(), (std::uint64_t{1} << 26) + (rng() & 0x3ffU));
        slope[i] = Money::from_raw((i & 1U) != 0 ? -a : a);
        price[i] = Price::from_raw(static_cast<std::int64_t>((std::uint64_t{1} << 45) + (rng() & 0xffffU)));
        fx[i] = Money::from_raw(Raw(rng(), 64 + (rng() & 7U)));
    }
    measure_products("mul_price.fallback_success", slope, price, true);
    measure_products("mul_fx.fallback_success", slope, fx, true);
    for (std::size_t i = 0; i < corpus_size; ++i) {
        const Raw a(rng(), (std::uint64_t{1} << 61) | (rng() >> 3));
        slope[i] = Money::from_raw((i & 1U) != 0 ? -a : a);
        fx[i] = Money::from_raw(a);
    }
    measure_products("mul_price.fallback_overflow", slope, price, true, ResultKind::overflow);
    measure_products("mul_fx.fallback_overflow", slope, fx, true, ResultKind::overflow);

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
    measure_pair(
        "cast.outside_int128", [&](std::size_t i) { return general_cast(wide[i]); },
        [&](std::size_t i) { return public_cast(wide[i]); }, ResultKind::overflow);
    // Alignment to scale 12 overflows int128; the final scale-zero answer fits.
    measure_pair(
        "add_mixed.fallback_success", [&](std::size_t i) { return general_add_sub<Whole, false>(total[i], whole[i]); },
        [&](std::size_t i) { return public_add_sub<Whole, false>(total[i], whole[i]); });
    measure_pair(
        "sub_mixed.fallback_success", [&](std::size_t i) { return general_add_sub<Whole, true>(total[i], whole[i]); },
        [&](std::size_t i) { return public_add_sub<Whole, true>(total[i], whole[i]); });
    for (std::size_t i = 0; i < corpus_size; ++i) {
        total[i] = Money::from_raw(Raw(rng(), rng()));
        previous[i] = Money::from_raw(Raw(rng(), rng()));
        next[i] = Money::from_raw(Raw(rng(), rng()));
    }
    replace("replace.full_range", ResultKind::mixed);
    std::fprintf(stderr, "# unsigned_sink=%llu\n", static_cast<unsigned long long>(sink));
}
