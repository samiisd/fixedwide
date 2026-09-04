#include "baselines.hpp"
#include <fixedwide/arithmetic.hpp>
#include <fixedwide/chars.hpp>
#include <fixedwide/floating.hpp>
#include <fixedwide/hash.hpp>
#include <fixedwide/integer_chars.hpp>
#include <fixedwide/string.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>
#ifdef FIXEDWIDE_BENCH_BOOST
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#endif

namespace fw = fixedwide;
using fw::FP128;
using fw::FP64;
using fw::i128;
using fw::Rounding;
using fw::u128;
using fw::u256;
constexpr std::size_t dataset_size = 4096;
constexpr std::size_t mask = dataset_size - 1;
struct Row {
    std::int64_t a, b;
    i128 wide_a, wide_b, full_divisor, division_a;
    u256 numerator;
    u128 divisor;
    double floating;
};
std::array<Row, dataset_size> rows;
std::array<std::string, dataset_size> texts, bigtexts;
std::size_t iterations = 262144;
unsigned repetitions = 9;
std::string filter;

// The memory clobber prevents invariant input loads moving across iterations.
// Consume the entire arithmetic result through a limb checksum, not just its low
// word. Random generation, strings and oracle validation are OUTSIDE timed loops.
inline void escape(std::uint64_t value) {
    __asm__ __volatile__("" : : "g"(value) : "memory");
}
inline void escape(double value) {
    __asm__ __volatile__("" : : "m"(value) : "memory");
}
std::uint64_t digest(u128 value) {
    return static_cast<std::uint64_t>(value) ^ static_cast<std::uint64_t>(value >> 64);
}
std::uint64_t digest(i128 value) {
    return digest(static_cast<u128>(value));
}
std::uint64_t digest(u256 value) {
    return digest(static_cast<u128>(value)) ^ digest(static_cast<u128>(value >> 128));
}
template<class FP>
std::uint64_t checked(const std::expected<FP, fw::ArithmeticError>& value) {
    return value ? digest(static_cast<i128>(value->raw())) : UINT64_MAX - static_cast<unsigned>(value.error());
}
template<class FP>
std::uint64_t parsed(const std::expected<FP, fw::ParseError>& value) {
    return value ? digest(static_cast<i128>(value->raw())) : UINT64_MAX - static_cast<unsigned>(value.error());
}
std::uint64_t division_digest(const std::expected<fw::UnsignedDivision, fw::ArithmeticError>& value) {
    return value ? digest(value->quotient) ^ digest(value->remainder) : UINT64_MAX;
}
std::uint64_t legacy(bool ok, const baseline::old_i128& result) {
    return ok ? digest(static_cast<i128>(result)) : UINT64_MAX;
}

template<class F>
void benchmark(const char* name, F function, unsigned iteration_divisor = 1) {
    if (!filter.empty() && std::string_view(name).find(filter) == std::string_view::npos) return;
    const auto count = std::max<std::size_t>(4096, iterations / iteration_divisor);
    for (std::size_t i = 0; i < dataset_size; ++i) escape(function(i));
    std::vector<double> samples;
    samples.reserve(repetitions);
    for (unsigned repeat = 0; repeat < repetitions; ++repeat) {
        const auto start = std::chrono::steady_clock::now();
        for (std::size_t i = 0; i < count; ++i) escape(function(i & mask));
        const auto end = std::chrono::steady_clock::now();
        samples.push_back(std::chrono::duration<double, std::nano>(end - start).count() / static_cast<double>(count));
    }
    auto sorted = samples;
    std::sort(sorted.begin(), sorted.end());
    const auto p95 = static_cast<std::size_t>((sorted.size() - 1) * 95 / 100);
    std::printf("%s,%zu,%u,%.6f,%.6f,%.6f,%.6f,\"", name, count, repetitions, sorted.front(), sorted[sorted.size() / 2],
                sorted[p95], sorted.back());
    for (unsigned i = 0; i < samples.size(); ++i) std::printf("%s%.6f", i == 0 ? "" : ";", samples[i]);
    std::printf("\"\n");
    std::fflush(stdout);
}
void setup() {
    std::mt19937_64 rng(0x4b209cd5);
    for (std::size_t i = 0; i < dataset_size; ++i) {
        auto& r = rows[i];
        r.a = static_cast<std::int64_t>(rng() % (1000 * fw::scale)) + fw::scale;
        r.b = static_cast<std::int64_t>(rng() % (10 * fw::scale)) + fw::scale;
        if (i % 4 == 0) r.a = -r.a;
        r.wide_a = (i128{1} << 84) + static_cast<i128>(rng());
        r.wide_b = (i128{1} << 64) + static_cast<i128>(rng());
        r.division_a = (i128{1} << 120) + static_cast<i128>(rng());
        r.full_divisor = (i128{1} << 96) + static_cast<i128>(rng());
        r.numerator = (static_cast<u256>(rng()) << 192) | (static_cast<u256>(rng()) << 128) |
                      (static_cast<u256>(rng()) << 64) | static_cast<u256>(rng());
        r.divisor = (u128{rng()} << 64) | rng() | 1;
        r.floating = static_cast<double>(r.a) / fw::scale;
        const auto s = fw::to_string(FP64::from_raw(r.a));
        if (!s) std::abort();
        texts[i] = *s;
        char buffer[80];
        auto size = fw::to_chars(buffer, sizeof(buffer), r.numerator);
        if (!size) std::abort();
        bigtexts[i] = std::string(buffer, *size);

        // Verify each legacy/new arithmetic comparison before collecting timings.
        baseline::old_i128 old;
        for (auto operands : {std::pair<i128, i128>{r.a, r.b}, {r.wide_a, r.wide_b}}) {
            const auto value =
                fw::mul(FP128::from_raw(operands.first), FP128::from_raw(operands.second), Rounding::toward_zero);
            if (!value || !baseline::legacy_mul(operands.first, operands.second, old) ||
                value->raw() != static_cast<i128>(old))
                std::abort();
        }
        for (i128 numerator : {r.wide_a, r.division_a})
            for (i128 divisor : {i128{r.b}, r.full_divisor}) {
                const auto value = fw::div(FP128::from_raw(numerator), FP128::from_raw(divisor), Rounding::toward_zero);
                if (!value || !baseline::legacy_div(numerator, divisor, old) || value->raw() != static_cast<i128>(old))
                    std::abort();
            }
        const auto result = fw::mul_div(FP128::from_raw(r.wide_a), FP128::from_raw(r.wide_b),
                                        FP128::from_raw(r.full_divisor), Rounding::toward_zero);
        if (!result || !baseline::legacy_mul_div(r.wide_a, r.wide_b, r.full_divisor, old) ||
            result->raw() != static_cast<i128>(old))
            std::abort();
        for (u128 divisor : {static_cast<u128>(fw::scale), r.divisor}) {
            auto fast = fw::divmod(r.numerator, divisor), native = baseline::native_divmod(r.numerator, divisor);
            if (!fast || !native || fast->quotient != native->quotient || fast->remainder != native->remainder)
                std::abort();
        }
    }
}
int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--iterations" && i + 1 < argc)
            iterations = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--repetitions" && i + 1 < argc)
            repetitions = static_cast<unsigned>(std::strtoul(argv[++i], nullptr, 10));
        else if (arg == "--filter" && i + 1 < argc)
            filter = argv[++i];
        else {
            std::fprintf(stderr, "usage: %s [--iterations N] [--repetitions N] [--filter substring]\n", argv[0]);
            return 2;
        }
    }
    if (iterations < 4096 || repetitions < 3 || repetitions > 1000) return 2;
    std::fprintf(stderr, "compiler=%s; scale=%u; dataset=%zu; iterations=%zu; repetitions=%u; seed=0x4b209cd5\n",
                 __VERSION__, fw::fractional_digits, dataset_size, iterations, repetitions);
    setup();
    std::puts("name,iterations,repetitions,min_ns,median_ns,p95_batch_ns,max_ns,samples_ns");
    benchmark("harness.read64", [](std::size_t i) { return static_cast<std::uint64_t>(rows[i].a); });
    benchmark("raw64.compare", [](std::size_t i) { return static_cast<std::uint64_t>(rows[i].a < rows[i].b); });
    benchmark("fp64.compare", [](std::size_t i) {
        return static_cast<std::uint64_t>(FP64::from_raw(rows[i].a) < FP64::from_raw(rows[i].b));
    });
    benchmark("raw64.add", [](std::size_t i) { return static_cast<std::uint64_t>(rows[i].a + rows[i].b); });
    benchmark("fp64.add_checked",
              [](std::size_t i) { return checked(fw::add(FP64::from_raw(rows[i].a), FP64::from_raw(rows[i].b))); });
    benchmark("fp64.sub_checked",
              [](std::size_t i) { return checked(fw::sub(FP64::from_raw(rows[i].a), FP64::from_raw(rows[i].b))); });
    benchmark("fp128.add_checked", [](std::size_t i) {
        return checked(fw::add(FP128::from_raw(rows[i].wide_a), FP128::from_raw(rows[i].wide_b)));
    });
    benchmark("raw128.add", [](std::size_t i) { return digest(rows[i].wide_a + rows[i].wide_b); });
    benchmark("fp128.compare", [](std::size_t i) {
        return static_cast<std::uint64_t>(FP128::from_raw(rows[i].wide_a) < FP128::from_raw(rows[i].wide_b));
    });
    benchmark("fp64.hash", [](std::size_t i) { return fw::hash_value(FP64::from_raw(rows[i].a)); });
    benchmark("fp128.hash", [](std::size_t i) { return fw::hash_value(FP128::from_raw(rows[i].wide_a)); });
    benchmark("fp64.widen", [](std::size_t i) { return digest(FP128(FP64::from_raw(rows[i].a)).raw()); });
    benchmark("fp128.narrow", [](std::size_t i) { return checked(fw::narrow(FP128::from_raw(rows[i].a))); });
    benchmark("fp64.mul.typical", [](std::size_t i) {
        return checked(fw::mul(FP64::from_raw(rows[i].a), FP64::from_raw(rows[i].b), Rounding::toward_zero));
    });
    benchmark("fp64.mul_wide.typical", [](std::size_t i) {
        return checked(fw::mul_wide(FP64::from_raw(rows[i].a), FP64::from_raw(rows[i].b), Rounding::toward_zero));
    });
    benchmark("fp128.mul.typical", [](std::size_t i) {
        return checked(fw::mul(FP128::from_raw(rows[i].a), FP128::from_raw(rows[i].b), Rounding::toward_zero));
    });
    benchmark("legacy128.mul.typical", [](std::size_t i) {
        baseline::old_i128 out;
        const bool ok = baseline::legacy_mul(rows[i].a, rows[i].b, out);
        return legacy(ok, out);
    });
    benchmark("fp64.mul.nearest_even", [](std::size_t i) {
        return checked(fw::mul(FP64::from_raw(rows[i].a), FP64::from_raw(rows[i].b), Rounding::nearest_even));
    });
    benchmark("fp128.mul.wide", [](std::size_t i) {
        return checked(
            fw::mul(FP128::from_raw(rows[i].wide_a), FP128::from_raw(rows[i].wide_b), Rounding::toward_zero));
    });
    benchmark("legacy128.mul.wide", [](std::size_t i) {
        baseline::old_i128 out;
        const bool ok = baseline::legacy_mul(rows[i].wide_a, rows[i].wide_b, out);
        return legacy(ok, out);
    });
    benchmark("fp64.div.typical", [](std::size_t i) {
        return checked(fw::div(FP64::from_raw(rows[i].a), FP64::from_raw(rows[i].b), Rounding::toward_zero));
    });
    benchmark("fp128.div.typical", [](std::size_t i) {
        return checked(fw::div(FP128::from_raw(rows[i].a), FP128::from_raw(rows[i].b), Rounding::toward_zero));
    });
    benchmark("legacy128.div.typical", [](std::size_t i) {
        baseline::old_i128 out;
        const bool ok = baseline::legacy_div(rows[i].a, rows[i].b, out);
        return legacy(ok, out);
    });
    benchmark("fp128.div.native_by64", [](std::size_t i) {
        return checked(fw::div(FP128::from_raw(rows[i].wide_a), FP128::from_raw(rows[i].b), Rounding::toward_zero));
    });
    benchmark("legacy128.div.native_by64", [](std::size_t i) {
        baseline::old_i128 out;
        const bool ok = baseline::legacy_div(rows[i].wide_a, rows[i].b, out);
        return legacy(ok, out);
    });
    benchmark("fp128.div.native_by128", [](std::size_t i) {
        return checked(
            fw::div(FP128::from_raw(rows[i].wide_a), FP128::from_raw(rows[i].full_divisor), Rounding::toward_zero));
    });
    benchmark("legacy128.div.native_by128", [](std::size_t i) {
        baseline::old_i128 out;
        const bool ok = baseline::legacy_div(rows[i].wide_a, rows[i].full_divisor, out);
        return legacy(ok, out);
    });
    benchmark("fp128.div.wide_by64", [](std::size_t i) {
        return checked(fw::div(FP128::from_raw(rows[i].division_a), FP128::from_raw(rows[i].b), Rounding::toward_zero));
    });
    benchmark("legacy128.div.wide_by64", [](std::size_t i) {
        baseline::old_i128 out;
        const bool ok = baseline::legacy_div(rows[i].division_a, rows[i].b, out);
        return legacy(ok, out);
    });
    benchmark("fp128.div.wide_by128", [](std::size_t i) {
        return checked(
            fw::div(FP128::from_raw(rows[i].division_a), FP128::from_raw(rows[i].full_divisor), Rounding::toward_zero));
    });
    benchmark("legacy128.div.wide_by128", [](std::size_t i) {
        baseline::old_i128 out;
        const bool ok = baseline::legacy_div(rows[i].division_a, rows[i].full_divisor, out);
        return legacy(ok, out);
    });
    benchmark("fp128.mul_div.wide", [](std::size_t i) {
        return checked(fw::mul_div(FP128::from_raw(rows[i].wide_a), FP128::from_raw(rows[i].wide_b),
                                   FP128::from_raw(rows[i].full_divisor), Rounding::toward_zero));
    });
    benchmark("legacy128.mul_div.wide", [](std::size_t i) {
        baseline::old_i128 out;
        const bool ok = baseline::legacy_mul_div(rows[i].wide_a, rows[i].wide_b, rows[i].full_divisor, out);
        return legacy(ok, out);
    });
    benchmark("bigint.div256_by64", [](std::size_t i) {
        return division_digest(fw::divmod(rows[i].numerator, static_cast<u128>(fw::scale)));
    });
    benchmark("native.div256_by64", [](std::size_t i) {
        return division_digest(baseline::native_divmod(rows[i].numerator, static_cast<u128>(fw::scale)));
    });
    benchmark("bigint.div256_by128",
              [](std::size_t i) { return division_digest(fw::divmod(rows[i].numerator, rows[i].divisor)); });
    benchmark("native.div256_by128", [](std::size_t i) {
        return division_digest(baseline::native_divmod(rows[i].numerator, rows[i].divisor));
    });
    benchmark("fp128.mul.overflow", [](std::size_t i) {
        return checked(fw::mul(fw::fp128_max, FP128::from_raw(rows[i].wide_a), Rounding::toward_zero));
    });
    benchmark("fp128.div.zero", [](std::size_t i) {
        return checked(fw::div(FP128::from_raw(rows[i].a), FP128{}, Rounding::toward_zero));
    });
    benchmark("fp64.quantize", [](std::size_t i) {
        return checked(fw::quantize(FP64::from_raw(rows[i].a), 4, Rounding::nearest_even));
    });
    benchmark("fp128.quantize", [](std::size_t i) {
        return checked(fw::quantize(FP128::from_raw(rows[i].wide_a), 4, Rounding::nearest_even));
    });
    benchmark("fp64.remainder", [](std::size_t i) {
        return checked(fw::remainder(FP64::from_raw(rows[i].a), FP64::from_raw(rows[i].b)));
    });
    benchmark("fp64.parse", [](std::size_t i) { return parsed(fw::parse64(texts[i])); });
    benchmark("fp128.parse", [](std::size_t i) { return parsed(fw::parse128(texts[i])); });
    benchmark("fp128.parse.scientific", [](std::size_t i) {
        return parsed(fw::parse128(i % 2 ? "-12345.678901e-3" : "7.345678912e+3", Rounding::nearest_even));
    });
    benchmark("fp128.parse.invalid", [](std::size_t i) { return parsed(fw::parse128(i % 2 ? "1e+" : "12.34x")); });
    benchmark("fp64.format.buffer", [](std::size_t i) {
        char b[48];
        auto n = fw::to_chars(b, sizeof(b), FP64::from_raw(rows[i].a));
        return n ? static_cast<std::uint64_t>(*n + b[0] + b[*n - 1]) : UINT64_MAX;
    });
    benchmark("fp128.format.buffer", [](std::size_t i) {
        char b[48];
        auto n = fw::to_chars(b, sizeof(b), FP128::from_raw(rows[i].wide_a));
        return n ? static_cast<std::uint64_t>(*n + b[0] + b[*n - 1]) : UINT64_MAX;
    });
    benchmark("fp128.format.string", [](std::size_t i) {
        auto s = fw::to_string(FP128::from_raw(rows[i].wide_a));
        return s ? static_cast<std::uint64_t>(s->size() + s->front() + s->back()) : UINT64_MAX;
    });
    benchmark("bigint.u256.format", [](std::size_t i) {
        char b[80];
        auto n = fw::to_chars(b, sizeof(b), rows[i].numerator);
        return n ? static_cast<std::uint64_t>(*n + b[0] + b[*n - 1]) : UINT64_MAX;
    });
    benchmark("bigint.u256.parse", [](std::size_t i) {
        auto v = fw::parse_u256(bigtexts[i]);
        return v ? digest(*v) : UINT64_MAX;
    });
    benchmark("fp64.from_double",
              [](std::size_t i) { return checked(fw::from_double64(rows[i].floating, Rounding::nearest_even)); });
    benchmark("fp128.from_double",
              [](std::size_t i) { return checked(fw::from_double128(rows[i].floating, Rounding::nearest_even)); });
    benchmark("fp64.to_double", [](std::size_t i) { return fw::to_double(FP64::from_raw(rows[i].a)); });
    benchmark("fp128.to_double", [](std::size_t i) { return fw::to_double(FP128::from_raw(rows[i].wide_a)); });
#ifdef FIXEDWIDE_BENCH_BOOST
    using Decimal = boost::multiprecision::cpp_dec_float_50;
    std::vector<Decimal> da, db;
    for (const auto& r : rows) {
        da.emplace_back(Decimal(r.a) / fw::scale);
        db.emplace_back(Decimal(r.b) / fw::scale);
    }
    // Context only: decimal floating point has DIFFERENT range/precision semantics.
    benchmark(
        "boost.dec_float50.mul",
        [&](std::size_t i) {
            const Decimal x = da[i] * db[i];
            __asm__ __volatile__("" : : "m"(x) : "memory");
            return std::uint64_t{0};
        },
        4);
    benchmark(
        "boost.dec_float50.div",
        [&](std::size_t i) {
            const Decimal x = da[i] / db[i];
            __asm__ __volatile__("" : : "m"(x) : "memory");
            return std::uint64_t{0};
        },
        4);
    benchmark(
        "boost.dec_float50.parse",
        [&](std::size_t i) {
            const Decimal x(texts[i]);
            __asm__ __volatile__("" : : "m"(x) : "memory");
            return std::uint64_t{0};
        },
        4);
#endif
}
