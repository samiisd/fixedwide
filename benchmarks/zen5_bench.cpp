// Complete-call experiments over the same retained inputs. This is a library
// arithmetic fixture, not a production HFT owner or a multicore benchmark.
#include "zen5_kernels.hpp"
#include <benchmark/benchmark.h>
#include <algorithm>
#include <charconv>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace z5 = fixedwide_zen5;
using Money = z5::Money;
using Raw = fixedwide::wide::int128;
using Status = z5::Status;

namespace {
constexpr std::int64_t decimal_grid = 10'000'000'000LL;
constexpr Money sentinel = Money::from_raw(Raw{-987654321});
std::size_t validated_rows = 0;

[[noreturn]] void fail(const std::string& message) {
    std::fprintf(stderr, "Zen 5 benchmark validation failed: %s\n", message.c_str());
    std::exit(1);
}

struct Fixture {
    std::string name;
    z5::Kernel operation;
    std::int64_t factor = 1'000'000;
    std::vector<Money> slopes, initial_values, initial_balances;
    std::vector<std::size_t> destinations;
};

enum class Shape { q31, q50, q63p1, q80, q90, mixed_wide, offgrid, wide_factor, refusal };

const char* shape_name(Shape shape) {
    switch (shape) {
    case Shape::q31: return "q31";
    case Shape::q50: return "q50";
    case Shape::q63p1: return "q63p1";
    case Shape::q80: return "q80";
    case Shape::q90: return "q90";
    case Shape::mixed_wide: return "mixed5pct_q80";
    case Shape::offgrid: return "all_offgrid_factor_cancels";
    case Shape::wide_factor: return "q50_factor63";
    case Shape::refusal: return "refused_prefix";
    }
    return "unknown";
}

Fixture make_fixture(std::size_t count, Shape shape, z5::Kernel operation, bool duplicates = false,
                     bool holes = false) {
    Fixture result;
    result.operation = operation;
    result.name = operation == z5::Kernel::product ? "product/"
                  : duplicates                     ? "delta_duplicates_holes5/"
                                                   : "delta_unique/";
    result.name += shape_name(shape);
    result.slopes.resize(count);
    result.initial_values.assign(count, operation == z5::Kernel::product ? sentinel : Money{});
    if (operation == z5::Kernel::delta) {
        result.destinations.resize(count);
        result.initial_balances.resize(duplicates ? (count + 7) / 8 : count);
    }
    if (shape == Shape::offgrid) result.factor = decimal_grid;
    if (shape == Shape::wide_factor) result.factor = (std::int64_t{1} << 62) + 1;
    if (shape == Shape::refusal) result.factor = 1;
    for (std::size_t row = 0; row < count; ++row) {
        unsigned shift = 30;
        if (shape == Shape::q50 || shape == Shape::wide_factor || shape == Shape::offgrid) shift = 49;
        if (shape == Shape::q63p1) shift = 63;
        if (shape == Shape::q80 || (shape == Shape::mixed_wide && row % 20 == 1)) shift = 80;
        if (shape == Shape::q90) shift = 90;
        const Raw coefficient = (Raw{1} << shift) + Raw{(row & 1023U) + 1};
        Raw raw = coefficient * Raw{decimal_grid};
        // The low ten bits are zero, so the full modular certificate must
        // perform its inverse/range rejection rather than a cheap bit test.
        if (shape == Shape::offgrid) raw += Raw{1024};
        result.slopes[row] = Money::from_raw((row & 1U) == 0 ? raw : -raw);
        if (operation == z5::Kernel::delta)
            result.destinations[row] = holes && row % 20 == 19 ? z5::sink : duplicates ? row / 8 : row;
    }
    if (shape == Shape::refusal && count != 0)
        result.slopes[std::min(count - 1, count / 2 + 1)] = Money::from_raw(Raw{1});
    return result;
}

bool same_status(const Status& left, const Status& right) {
    return left.has_value() == right.has_value() && (left || left.error() == right.error());
}

Status run(const Fixture& fixture, z5::Backend backend, std::int64_t factor, std::vector<Money>& values,
           std::vector<Money>& balances, z5::Diagnostics* counts = nullptr) {
    return z5::execute(backend, fixture.operation, fixture.slopes, factor, values, fixture.destinations, balances,
                       counts);
}

void check(const Fixture& fixture, z5::Backend backend, const Status& reference, const Status& actual,
           const std::vector<Money>& expected_values, const std::vector<Money>& actual_values,
           const std::vector<Money>& expected_balances, const std::vector<Money>& actual_balances) {
    if (!same_status(reference, actual) || expected_values != actual_values || expected_balances != actual_balances)
        fail(fixture.name + "/" + z5::backend_name(backend) + "/" + std::to_string(fixture.slopes.size()) +
             " result, first error, or accepted prefix mismatch");
    validated_rows += actual_values.size();
}

void add_counters(benchmark::State& state, const Fixture& fixture, z5::Backend backend, const Status& status,
                  const z5::Diagnostics& diagnostics) {
    const auto completed = status ? fixture.slopes.size() : status.error().index;
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(completed));
    state.counters["input_rows"] = static_cast<double>(fixture.slopes.size());
    state.counters["completed_rows"] = static_cast<double>(completed);
    state.counters["error_index"] = status ? -1.0 : static_cast<double>(status.error().index);
    state.counters["refusals"] = status ? 0.0 : 1.0;
    state.counters["block_lanes"] = z5::block_lanes(backend);
    state.counters["diagnostic_tile_calls"] = static_cast<double>(diagnostics.tile_calls);
    state.counters["diagnostic_simd_evaluated"] = static_cast<double>(diagnostics.simd_evaluated);
    state.counters["diagnostic_simd_accepted"] = static_cast<double>(diagnostics.simd_accepted);
    state.counters["diagnostic_simd_commits"] = static_cast<double>(diagnostics.simd_commits);
    state.counters["diagnostic_scalar_maps"] = static_cast<double>(diagnostics.scalar_maps);
    state.counters["diagnostic_fallback_maps"] = static_cast<double>(diagnostics.fallback_maps);
    state.counters["diagnostic_tail_maps"] = static_cast<double>(diagnostics.tail_maps);
    state.counters["diagnostic_commit_calls"] = static_cast<double>(diagnostics.commit_calls);
    state.counters["diagnostic_direct_store_rows"] = static_cast<double>(diagnostics.direct_store_rows);
    state.counters["heap_scratch_bytes"] = 0.0;
    if (completed != 0)
        state.counters["seconds_per_row"] =
            benchmark::Counter(static_cast<double>(completed),
                               benchmark::Counter::kIsIterationInvariantRate | benchmark::Counter::kInvert);
}

void register_fixture(Fixture fixture, const std::vector<z5::Backend>& selected) {
    auto shared = std::make_shared<const Fixture>(std::move(fixture));
    auto reference_values = shared->initial_values;
    auto reference_balances = shared->initial_balances;
    const auto reference =
        run(*shared, z5::Backend::public_scalar, shared->factor, reference_values, reference_balances);
    auto reverse_values = reference_values;
    auto reverse_balances = reference_balances;
    const auto reverse = run(*shared, z5::Backend::public_scalar, -shared->factor, reverse_values, reverse_balances);
    if (shared->operation == z5::Kernel::delta &&
        (reverse_values != shared->initial_values || reverse_balances != shared->initial_balances))
        fail(shared->name + " alternating quotes did not restore the initial state");

    for (const auto backend : selected) {
        auto values = shared->initial_values;
        auto balances = shared->initial_balances;
        const auto status = run(*shared, backend, shared->factor, values, balances);
        check(*shared, backend, reference, status, reference_values, values, reference_balances, balances);
        const auto reversed = run(*shared, backend, -shared->factor, values, balances);
        check(*shared, backend, reverse, reversed, reverse_values, values, reverse_balances, balances);

        values = shared->initial_values;
        balances = shared->initial_balances;
        z5::Diagnostics diagnostics;
        const auto diagnostic_status = run(*shared, backend, shared->factor, values, balances, &diagnostics);
        check(*shared, backend, reference, diagnostic_status, reference_values, values, reference_balances, balances);
        const auto completed = status ? shared->slopes.size() : status.error().index;
        if (diagnostics.completed_rows != completed) fail(shared->name + " diagnostic completed-row count");

        const auto name =
            "zen5/" + shared->name + "/" + z5::backend_name(backend) + "/" + std::to_string(shared->slopes.size());
        benchmark::RegisterBenchmark(name, [shared, backend, reference, diagnostics](benchmark::State& state) {
            // Allocations and diagnostic execution are outside timing. The
            // kernel itself prepares its broadcast plan on every complete call.
            auto values = shared->initial_values;
            auto balances = shared->initial_balances;
            std::int64_t factor = shared->factor;
            for (auto _ : state) {
                benchmark::DoNotOptimize(factor);
                auto status = run(*shared, backend, factor, values, balances);
                benchmark::DoNotOptimize(status);
                benchmark::ClobberMemory();
                factor = -factor;
            }
            benchmark::DoNotOptimize(values.data());
            benchmark::DoNotOptimize(balances.data());
            add_counters(state, *shared, backend, reference, diagnostics);
        })->Unit(benchmark::kNanosecond);
    }
}

void register_fixtures(const std::vector<std::size_t>& sizes, const std::vector<z5::Backend>& selected) {
    for (const auto count : sizes) {
        // Small-call latency has two representative coefficient shapes rather
        // than a full cross-product of every workload and array size.
        if (count < 64) {
            for (const auto shape : {Shape::q31, Shape::q80}) {
                register_fixture(make_fixture(count, shape, z5::Kernel::product), selected);
                register_fixture(make_fixture(count, shape, z5::Kernel::delta), selected);
            }
            continue;
        }
        for (const auto shape : {Shape::q31, Shape::q50, Shape::q63p1, Shape::q80, Shape::q90, Shape::mixed_wide,
                                 Shape::offgrid, Shape::wide_factor, Shape::refusal})
            register_fixture(make_fixture(count, shape, z5::Kernel::product), selected);
        for (const auto shape : {Shape::q31, Shape::q80}) {
            register_fixture(make_fixture(count, shape, z5::Kernel::delta), selected);
            register_fixture(make_fixture(count, shape, z5::Kernel::delta, true, true), selected);
        }
        register_fixture(make_fixture(count, Shape::mixed_wide, z5::Kernel::delta, true, true), selected);
        register_fixture(make_fixture(count, Shape::refusal, z5::Kernel::delta, true, true), selected);
    }
}

std::optional<std::vector<std::size_t>> parse_sizes(std::string_view text) {
    std::vector<std::size_t> result;
    while (!text.empty()) {
        const auto end = text.find(',');
        const auto item = text.substr(0, end);
        std::size_t value;
        const auto parsed = std::from_chars(item.data(), item.data() + item.size(), value);
        if (item.empty() || parsed.ec != std::errc{} || parsed.ptr != item.data() + item.size() || value > 1'000'000)
            return std::nullopt;
        result.push_back(value);
        if (end == std::string_view::npos) break;
        text.remove_prefix(end + 1);
        if (text.empty()) return std::nullopt;
    }
    if (result.empty()) return std::nullopt;
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}
} // namespace

int main(int argc, char** argv) {
    bool validate_only = false;
    std::string_view requested = "all";
    std::vector<std::size_t> sizes{1, 4, 8, 64, 125'000};
    int remaining = 1;
    for (int i = 1; i < argc; ++i) {
        const std::string_view argument{argv[i]};
        if (argument == "--validate-only")
            validate_only = true;
        else if (argument.starts_with("--backend="))
            requested = argument.substr(10);
        else if (argument.starts_with("--sizes=")) {
            auto parsed = parse_sizes(argument.substr(8));
            if (!parsed) {
                std::fprintf(stderr, "--sizes requires comma-separated integers in [0,1000000]\n");
                return 2;
            }
            sizes = std::move(*parsed);
        } else
            argv[remaining++] = argv[i];
    }
    argc = remaining;
    argv[argc] = nullptr;
    std::vector<z5::Backend> selected;
    for (const auto backend : z5::backends) {
        const bool available = z5::supported(backend);
        std::fprintf(stderr, "backend %s: %s (block %u)\n", z5::backend_name(backend),
                     available ? "supported" : "unsupported CPU/OS/build", z5::block_lanes(backend));
        if (requested == z5::backend_name(backend)) {
            if (!available) {
                std::fprintf(stderr, "forced backend is unsupported; refusing substitution\n");
                return 3;
            }
            selected.push_back(backend);
        } else if (requested == "all" && available)
            selected.push_back(backend);
    }
    if (selected.empty()) {
        std::fprintf(stderr, "unknown or empty --backend selection\n");
        return 2;
    }
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 2;
    benchmark::AddCustomContext("compiler", __VERSION__);
#if defined(_LIBCPP_VERSION)
    benchmark::AddCustomContext("standard_library", "libc++ " + std::to_string(_LIBCPP_VERSION));
#elif defined(__GLIBCXX__)
    benchmark::AddCustomContext("standard_library", "libstdc++ " + std::to_string(__GLIBCXX__));
#endif
    benchmark::AddCustomContext("xcr0", std::to_string(z5::hardware().xcr0));
    benchmark::AddCustomContext("forced_backend", std::string{requested});
    benchmark::AddCustomContext("baseline", "public scalar and prepared scalar from the same source revision");
    benchmark::AddCustomContext(
        "scope", "complete exact product/delta calls; preparation, packing, fallback and ordered stores included");
    benchmark::AddCustomContext("diagnostics",
                                "actual separately instrumented positive-quote execution; not timed counters");
    benchmark::AddCustomContext("quote_reuse", "alternating +factor/-factor; plan preparation included on every call");
    benchmark::AddCustomContext("experimental_dispatch",
                                "manually forced benchmark backends; library dispatch unchanged");
#if defined(FIXEDWIDE_GOOGLE_BENCHMARK_VERSION)
    benchmark::AddCustomContext("google_benchmark", FIXEDWIDE_GOOGLE_BENCHMARK_VERSION);
#endif
    register_fixtures(sizes, selected);
    benchmark::AddCustomContext("validated_rows", std::to_string(validated_rows));
    std::fprintf(stderr, "validated %zu output rows, both quote signs, ordered balances and diagnostic equivalence\n",
                 validated_rows);
    if (validate_only) {
        benchmark::Shutdown();
        return 0;
    }
    const auto matches = benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    if (matches == 0) {
        std::fprintf(stderr, "empty benchmark selection is an error\n");
        return 4;
    }
    return 0;
}
