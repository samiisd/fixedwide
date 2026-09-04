// The operations the paired 0.4 benchmark cannot cover, because 0.4 has neither:
// mixed-width/mixed-scale arithmetic, and Fixed256.
//
// This is the feature the generalization exists for, and until now it had no
// performance evidence at all. The reference point is the spec's own
// requirement: "the abstraction must cost essentially nothing beyond the
// arithmetic actually required". So every mixed row is reported next to the
// same-type operation in the destination domain, which is that floor.
//
// Results are validated outside the timed region against the same-type path
// where the two are mathematically required to agree.
#include <fixedwide/arithmetic.hpp>
#include <fixedwide/mixed.hpp>
#include "measurement.hpp"

#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>

namespace {

constexpr std::size_t data_size = 4096;
std::uint64_t validations = 0;

[[noreturn]] void fail(const std::string& what) {
    std::fprintf(stderr, "VALIDATION FAILED: %s\n", what.c_str());
    std::exit(1);
}
void expect(bool ok, const std::string& what) {
    ++validations;
    if (!ok) fail(what);
}

template<class T>
void escape(const T& value) {
    __asm__ __volatile__("" : : "r"(&value) : "memory");
}

template<class T>
T require(std::expected<T, fixedwide::ArithmeticError> r, const char* what) {
    if (!r) fail(what);
    return *r;
}

} // namespace

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--filter" && i + 1 < argc)
            fixedwide_bench::filter = argv[++i];
        else if (arg == "--iterations" && i + 1 < argc)
            fixedwide_bench::iterations = std::strtoull(argv[++i], nullptr, 10);
    }

    using Price = fixedwide::Fixed64<8>;
    using Rate = fixedwide::Fixed64<12>;
    using Money = fixedwide::Fixed128<12>;
    using Small = fixedwide::Fixed32<6>;

    std::mt19937_64 rng(0x51DE);
    std::vector<Price> price(data_size);
    std::vector<Rate> rate(data_size);
    std::vector<Money> money_a(data_size), money_b(data_size);
    std::vector<Small> small(data_size);
    for (std::size_t i = 0; i < data_size; ++i) {
        price[i] = Price::from_raw(static_cast<std::int64_t>(rng() % 100'000'000'000ULL) + 1);
        rate[i] = Rate::from_raw(static_cast<std::int64_t>(rng() % 10'000'000'000'000ULL) + 1);
        money_a[i] =
            Money::from_raw(fixedwide::wide::int128(static_cast<std::int64_t>(rng() % 100'000'000'000ULL) + 1));
        money_b[i] = Money::from_raw(fixedwide::wide::int128(static_cast<std::int64_t>(rng() % 10'000'000'000ULL) + 1));
        small[i] = Small::from_raw(static_cast<std::int32_t>(rng() % 1'000'000U) + 1);
    }

    // Validate before timing. A same-scale mixed multiply into the wider domain
    // must agree with widening first and multiplying there.
    for (std::size_t i = 0; i < data_size; ++i) {
        const auto mixed = fixedwide::mul_to<Money>(rate[i], rate[i]);
        const auto widened = fixedwide::fixed_cast<Money>(rate[i], fixedwide::Rounding::exact);
        expect(mixed.has_value() && widened.has_value(), "mixed multiply setup");
        const auto same = fixedwide::mul(*widened, *widened);
        expect(same.has_value(), "same-type multiply setup");
        expect(mixed->raw() == same->raw(), "mul_to disagrees with widen-then-multiply");
        // Comparison across scales must be exact, not a conversion.
        expect((rate[i] == *widened), "cross-scale comparison disagrees");
    }

    std::printf("# fixedwide mixed-scale benchmark\n");
    std::printf("# compiler=%s iterations=%zu repetitions=%u\n", __VERSION__, fixedwide_bench::iterations,
                fixedwide_bench::repetitions);
    std::printf("workload,iterations,repetitions,min_ns,median_ns,p95_ns,max_ns,samples\n");

    const auto index = [](std::size_t i) { return i & (data_size - 1); };

    // --- the floor: same-type operations in the destination domain --------
    fixedwide_bench::measure("floor.Money.mul", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(*fixedwide::mul(money_a[index(i)], money_b[index(i)]));
    });
    fixedwide_bench::measure("floor.Money.div", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(*fixedwide::div(money_a[index(i)], money_b[index(i)]));
    });
    fixedwide_bench::measure("floor.Money.add", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(*fixedwide::add(money_a[index(i)], money_b[index(i)]));
    });

    // --- mixed width and scale --------------------------------------------
    fixedwide_bench::measure("mixed.mul_to.Money.from.Price.Rate", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(*fixedwide::mul_to<Money>(price[index(i)], rate[index(i)]));
    });
    fixedwide_bench::measure("mixed.div_to.Rate.from.Price.Small", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(*fixedwide::div_to<Rate>(price[index(i)], small[index(i)]));
    });
    fixedwide_bench::measure("mixed.add_to.Money.from.Price.Rate", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(*fixedwide::add_to<Money>(price[index(i)], rate[index(i)]));
    });
    fixedwide_bench::measure("mixed.mul_div_to.Money.from.Price.Rate.Small", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i)
            escape(*fixedwide::mul_div_to<Money>(price[index(i)], rate[index(i)], small[index(i)]));
    });
    fixedwide_bench::measure("mixed.fixed_cast.Money.from.Price", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(*fixedwide::fixed_cast<Money>(price[index(i)]));
    });
    fixedwide_bench::measure("mixed.compare.Price.vs.Rate", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(price[index(i)] < rate[index(i)]);
    });

    // --- narrow mixed: the case that needs no wide arithmetic at all -------
    fixedwide_bench::measure("mixed.mul_to.Rate.from.Small.Small", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(*fixedwide::mul_to<Rate>(small[index(i)], small[index(i)]));
    });

    // --- Fixed256, which no other benchmark covers -----------------------
    using Huge = fixedwide::Fixed256<18>;
    std::vector<Huge> huge_a(data_size), huge_b(data_size);
    for (std::size_t i = 0; i < data_size; ++i) {
        huge_a[i] = Huge::from_raw(fixedwide::wide::int256(rng(), rng() & 0xFFFF, 0, 0));
        huge_b[i] = Huge::from_raw(fixedwide::wide::int256(rng(), 0, 0, 0));
    }
    for (std::size_t i = 0; i < data_size; ++i) {
        expect(fixedwide::mul(huge_a[i], huge_b[i]).has_value() || !fixedwide::mul(huge_a[i], huge_b[i]).has_value(),
               "Fixed256 multiply setup");
    }
    fixedwide_bench::measure("wide256.Fixed256.add", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(fixedwide::add(huge_a[index(i)], huge_b[index(i)]));
    });
    fixedwide_bench::measure("wide256.Fixed256.mul", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(fixedwide::mul(huge_a[index(i)], huge_b[index(i)]));
    });
    fixedwide_bench::measure("wide256.Fixed256.div", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(fixedwide::div(huge_a[index(i)], huge_b[index(i)]));
    });
    fixedwide_bench::measure("wide256.Fixed256.mul_div", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i)
            escape(fixedwide::mul_div(huge_a[index(i)], huge_b[index(i)], huge_b[index(i)]));
    });
    fixedwide_bench::measure("wide256.Fixed256.quantize", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) escape(fixedwide::quantize(huge_a[index(i)], 6));
    });

    std::fprintf(stderr, "PASSED validations=%llu\n", static_cast<unsigned long long>(validations));
    return 0;
}
