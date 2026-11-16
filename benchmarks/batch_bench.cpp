// Retained SoA inputs, complete arithmetic loops, and identical scalar/batch
// refusal boundaries. Timing is Google Benchmark's responsibility. This is a
// library benchmark, not a model of the complete HFT owner or its publication.
#include <fixedwide/batch.hpp>
#include <fixedwide/literal.hpp>
#include <benchmark/benchmark.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace fw = fixedwide;
using Money = fw::Fixed128<12>;
using Price = fw::Fixed64<10>;
using Wide = fw::Fixed256<12>;
using Raw = fw::wide::int128;
using Status = fw::batch::Result;
using ArithmeticStatus = std::expected<void, fw::ArithmeticError>;
template<class T>
using ArithmeticResult = std::expected<T, fw::ArithmeticError>;

namespace {
enum class Implementation { scalar, optimized_scalar, batch4 };
constexpr auto nearest = fw::Rounding::nearest_even;
constexpr auto exact = fw::Rounding::exact;
constexpr std::array implementations{Implementation::scalar, Implementation::optimized_scalar, Implementation::batch4};
constexpr std::array sizes{std::size_t{64}, std::size_t{125'000}};
constexpr Money sentinel = Money::from_raw(Raw{-987654321});
// Named constants also work around older Clang immediate-invocation bugs in
// aggregate default member initializers. Keep parsing entirely at compile time.
constexpr Price reference_mark{"100"};
constexpr Price alternate_mark{"100.00037"};
constexpr Money conversion_rate{"1.234567890123"};
std::size_t validated_rows = 0;

[[noreturn]] void fail(const std::string& message) {
    std::fprintf(stderr, "batch benchmark validation failed: %s\n", message.c_str());
    std::exit(1);
}

const char* name(Implementation implementation) {
    switch (implementation) {
    case Implementation::scalar: return "scalar";
    case Implementation::optimized_scalar: return "optimized_scalar";
    case Implementation::batch4: return "batch4";
    }
    return "unknown";
}

template<class T>
std::span<const T> view(const std::vector<T>& values) {
    return {values.data(), values.size()};
}

Status failure(std::size_t index, fw::ArithmeticError error) {
    return std::unexpected(fw::batch::Error{index, error});
}

bool same_status(const Status& left, const Status& right) {
    return left.has_value() == right.has_value() &&
           (left || (left.error().index == right.error().index && left.error().error == right.error().error));
}

void require_equal(const std::string& workload, const Status& reference, const Status& result,
                   const std::vector<Money>& expected, const std::vector<Money>& actual) {
    if (!same_status(reference, result)) {
        if (!reference)
            std::fprintf(stderr, "reference error: row %zu, cause %u\n", reference.error().index,
                         static_cast<unsigned>(reference.error().error));
        if (!result)
            std::fprintf(stderr, "actual error: row %zu, cause %u\n", result.error().index,
                         static_cast<unsigned>(result.error().error));
        fail(workload + " status mismatch");
    }
    if (expected != actual) {
        for (std::size_t row = 0; row < expected.size(); ++row) {
            if (expected[row] == actual[row]) continue;
            const auto wanted = expected[row].raw();
            const auto received = actual[row].raw();
            std::fprintf(stderr, "row %zu: expected raw 0x%016llx%016llx, actual raw 0x%016llx%016llx\n", row,
                         static_cast<unsigned long long>(wanted.high), static_cast<unsigned long long>(wanted.low),
                         static_cast<unsigned long long>(received.high), static_cast<unsigned long long>(received.low));
            break;
        }
        fail(workload + " value mismatch");
    }
    validated_rows += actual.size();
}

// A classification of the unscaled raw product, determined
// outside timing. It is not a hardware counter or instrumentation of the new
// implementation. The 128 x <=128 raw product always fits signed 256 bits.
template<class B>
bool product_outside_i128(Money a, B b) {
    const auto product = fw::detail::to_int256_raw(a.raw()) * fw::detail::to_int256_raw(b.raw());
    const std::uint64_t extension = (product.limbs[1] >> 63) != 0 ? ~std::uint64_t{0} : 0;
    return product.limbs[2] != extension || product.limbs[3] != extension;
}

// Untimed dispatch classification for this benchmark's Money x {Price,Money}
// -> Money products. This counts which required-prefix lanes delegate to the
// scalar API; it does not insert counters into the measured kernel.
// A prepared broadcast can cancel decimal factors before testing limb width.
template<bool Broadcast, class B>
bool delegates_scalar_multiply(Money a, B b) {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    using namespace fw::detail::batch;
    const auto lhs = native_magnitude(a.raw());
    const auto rhs = native_magnitude(b.raw());
    if constexpr (Broadcast) {
        constexpr auto scale = static_cast<std::uint64_t>(fw::detail::mixed_native::pow10_v<B::fractional_digits>);
        const auto common = std::gcd(static_cast<std::uint64_t>(rhs % scale), scale);
        if (((rhs / common) >> 64) == 0) return false;
    }
    return (lhs >> 64) != 0 && (rhs >> 64) != 0;
#else
    (void)a;
    (void)b;
    return true;
#endif
}

template<class B>
bool native_multiply_refuses(Money a, B b, fw::Rounding rounding) {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    return !fw::detail::mixed_native::try_mul<Money::fractional_digits, B::fractional_digits, Money::bits,
                                              Money::fractional_digits>(a.raw(), b.raw(), rounding);
#else
    (void)a;
    (void)b;
    (void)rounding;
    return true;
#endif
}

// Independent, untimed classification of lanes accepted by the four-row SIMD
// certificate. The timed kernel contains no instrumentation. A failed narrow
// certificate always means scalar fallback, never an arithmetic refusal.
template<class B>
bool simd_coefficient_fits(Money value, B factor) {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    if constexpr (B::bits <= 64 && B::fractional_digits > 0 && B::fractional_digits <= 19) {
        using I = __int128;
        constexpr I denominator = fw::detail::mixed_native::pow10_v<B::fractional_digits>;
        const I raw = static_cast<I>(value.raw());
        const I q = raw / denominator;
        if (q * denominator != raw) return false;
        if constexpr (B::fractional_digits == 10) {
            if (factor.raw() >= INT32_MIN && factor.raw() <= INT32_MAX) return q >= INT32_MIN && q <= INT32_MAX;
        }
        return q >= -I{UINT32_MAX} && q <= I{UINT32_MAX};
    }
#else
    (void)value;
    (void)factor;
#endif
    return false;
}

struct Counts {
    std::size_t input_rows{};
    std::size_t completed_rows{};
    std::size_t completed_operations{};
    std::size_t modeled_bytes{};
    std::size_t outside_i128_products{};
    std::size_t failures{};
    std::size_t positions{};
    std::size_t classified_product_rows{};
    std::size_t delegated_product_rows{};
    std::size_t native_refused_products{};
    std::size_t simd_product_rows{};
};

void counters(benchmark::State& state, Counts counts, Implementation implementation) {
    const auto iterations = state.iterations();
    state.SetItemsProcessed(iterations * static_cast<std::int64_t>(counts.completed_rows));
    state.counters["input_rows"] = static_cast<double>(counts.input_rows);
    state.counters["completed_rows"] = static_cast<double>(counts.completed_rows);
    state.counters["completed_ops"] = static_cast<double>(counts.completed_operations);
    state.counters["refusals"] = static_cast<double>(counts.failures);
    const auto required_rows = counts.completed_rows + counts.failures;
    const auto full_block_rows = counts.input_rows - counts.input_rows % 4;
    const auto mapped_rows =
        implementation == Implementation::batch4 && counts.failures != 0 && required_rows <= full_block_rows
            ? std::min(counts.input_rows, ((required_rows + 3) / 4) * 4)
            : required_rows;
    state.counters["mapped_rows"] = static_cast<double>(mapped_rows);
    state.counters["speculative_rows"] = static_cast<double>(mapped_rows - required_rows);
    state.counters["raw_products_outside_i128"] = static_cast<double>(counts.outside_i128_products);
    if (counts.classified_product_rows != 0) {
        state.counters["classified_product_rows"] = static_cast<double>(counts.classified_product_rows);
        const auto delegated =
            implementation == Implementation::scalar ? counts.classified_product_rows : counts.delegated_product_rows;
        state.counters["scalar_api_product_rows"] = static_cast<double>(delegated);
        // This classifies the original native attempt, not executed general
        // kernel calls: the public fallback may finish with a coefficient
        // certificate or quotient/remainder split. It remains comparable
        // when this same fixture is compiled against an older revision.
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
        const auto refused = implementation == Implementation::scalar ? counts.native_refused_products : delegated;
#else
        const auto refused = delegated;
#endif
        state.counters["classified_native_refused_rows"] = static_cast<double>(refused);
        state.counters["simd_product_rows"] =
            implementation == Implementation::batch4 ? static_cast<double>(counts.simd_product_rows) : 0.0;
    }
    state.counters["modeled_bytes"] = static_cast<double>(counts.modeled_bytes);
    state.counters["extra_array_passes"] = 0.0;
    state.counters["heap_scratch_bytes"] = 0.0;
    state.counters["block_lanes"] = implementation == Implementation::batch4 ? 4.0 : 1.0;
    // Upper bound for the generic scheduler's lane results, not a measured
    // stack frame. SIMD can store directly or retain only four int64 products.
    state.counters["lane_result_bytes"] =
        implementation == Implementation::scalar
            ? 0.0
            : static_cast<double>((implementation == Implementation::batch4 ? 4 : 1) * sizeof(ArithmeticResult<Money>));
    if (counts.positions != 0) state.counters["completed_positions"] = static_cast<double>(counts.positions);
    if (counts.completed_rows != 0) {
        // kInvert turns the iteration-invariant rate into elapsed time/row.
        // Keep seconds here: Google Benchmark prints kInvert counters with a
        // seconds suffix. Reports can multiply the JSON counter by 1e9 for ns.
        state.counters["seconds_per_row"] =
            benchmark::Counter(static_cast<double>(counts.completed_rows),
                               benchmark::Counter::kIsIterationInvariantRate | benchmark::Counter::kInvert);
    }
    if (counts.completed_operations != 0) {
        state.counters["seconds_per_op"] =
            benchmark::Counter(static_cast<double>(counts.completed_operations),
                               benchmark::Counter::kIsIterationInvariantRate | benchmark::Counter::kInvert);
    }
}

template<class Callback>
void dispatch(Implementation implementation, Callback&& callback) {
    switch (implementation) {
    case Implementation::scalar: callback.template operator()<Implementation::scalar>(); break;
    case Implementation::optimized_scalar: callback.template operator()<Implementation::optimized_scalar>(); break;
    case Implementation::batch4: callback.template operator()<Implementation::batch4>(); break;
    }
}

template<class T>
void consume(const std::vector<T>& values) {
    // Escape the whole allocation, then clobber memory. Every retained output
    // is observable; timing does not add a checksum/reduction over the book.
    auto pointer = values.data();
    benchmark::DoNotOptimize(pointer);
    benchmark::ClobberMemory();
}

// Keep invariant operands runtime-dependent. GCC can lose initialization when
// Google Benchmark's mixed memory/register read-write constraint is inlined
// through a fixed-point constructor. A single register alternative on primitive
// raw words retains the intended barrier without changing the numerical value.
// This target already requires GCC or Clang.
template<class Word>
void runtime_word(Word& word) noexcept {
    __asm__ volatile("" : "+r"(word) : : "memory");
}

template<class Fixed>
Fixed runtime_value(Fixed value) noexcept {
    auto raw = value.raw();
    if constexpr (Fixed::bits <= 64) {
        runtime_word(raw);
    } else {
        static_assert(Fixed::bits == 128);
        auto low = raw.low;
        auto high = raw.high;
        runtime_word(low);
        runtime_word(high);
        raw = typename Fixed::raw_type{low, high};
    }
    return Fixed::from_raw(raw);
}

ArithmeticResult<Money> replace(Money total, Money previous, Money next) noexcept {
    const auto removed = fw::sub_to<Wide>(total, previous, exact);
    if (!removed) return std::unexpected(removed.error());
    const auto replaced = fw::add_to<Wide>(*removed, next, exact);
    if (!replaced) return std::unexpected(replaced.error());
    return fw::fixed_cast<Money>(*replaced, exact);
}

template<class B>
struct Products {
    std::vector<Money> left;
    std::vector<B> right;
    B broadcast{};
    fw::Rounding rounding{nearest};
};

template<Implementation Impl, bool Broadcast, class B>
Status products(const Products<B>& data, std::span<Money> output) noexcept {
    const auto count = output.size();
    B factor = data.broadcast;
    if constexpr (Broadcast) factor = runtime_value(factor);
    if constexpr (Impl == Implementation::scalar) {
        for (std::size_t i = 0; i < count; ++i) {
            const auto result = fw::mul_to<Money>(data.left[i], Broadcast ? factor : data.right[i], data.rounding);
            if (!result) return failure(i, result.error());
            output[i] = *result;
        }
        return {};
    } else if constexpr (Impl == Implementation::batch4) {
        if constexpr (Broadcast)
            return fw::batch::transform<4>(output, fw::batch::prepare_mul_to<Money>(factor, data.rounding),
                                           view(data.left));
        else
            return fw::batch::transform<4>(output, fw::batch::mul_to<Money>(data.rounding), view(data.left),
                                           view(data.right));
    } else {
        const auto commit = [&](std::size_t i, Money value) noexcept -> ArithmeticStatus {
            output[i] = value;
            return {};
        };
        if constexpr (Broadcast) {
            const auto multiply = fw::batch::prepare_mul_to<Money>(factor, data.rounding);
            return fw::batch::for_each<1>(count, multiply, commit, view(data.left));
        } else {
            const auto multiply = fw::batch::mul_to<Money>(data.rounding);
            return fw::batch::for_each<1>(count, multiply, commit, view(data.left), view(data.right));
        }
    }
}

template<bool Broadcast, class B>
void register_products(const std::string& workload, Products<B> data, bool expect_error = false) {
    const auto input = std::make_shared<const Products<B>>(std::move(data));
    const auto count = input->left.size();
    std::vector<Money> expected(count, sentinel);
    const auto reference = products<Implementation::scalar, Broadcast>(*input, expected);
    if (reference.has_value() == expect_error) fail(workload + " unexpected fixture result");
    const std::size_t completed = reference ? count : reference.error().index;
    const std::size_t attempted = completed + static_cast<std::size_t>(!reference);
    std::size_t wide_products = 0;
    std::size_t delegated_products = 0;
    std::size_t native_refused_products = 0;
    std::size_t simd_products = 0;
    using Plan = decltype(fw::batch::prepare_mul_to<Money>(input->broadcast, input->rounding));
    using Simd = fw::detail::batch::simd_dispatch<Plan, std::span<const Money>>;
    bool simd_enabled = false;
    if constexpr (Broadcast && Simd::available)
        simd_enabled = Simd::can_run(fw::batch::prepare_mul_to<Money>(input->broadcast, input->rounding), count);
    for (std::size_t i = 0; i < attempted; ++i) {
        wide_products += static_cast<std::size_t>(
            product_outside_i128(input->left[i], Broadcast ? input->broadcast : input->right[i]));
        delegated_products += static_cast<std::size_t>(
            delegates_scalar_multiply<Broadcast>(input->left[i], Broadcast ? input->broadcast : input->right[i]));
        native_refused_products += static_cast<std::size_t>(
            native_multiply_refuses(input->left[i], Broadcast ? input->broadcast : input->right[i], input->rounding));
        if (simd_enabled && i < count - count % 4)
            simd_products += static_cast<std::size_t>(simd_coefficient_fits(input->left[i], input->broadcast));
    }
    const Counts counts{count,
                        completed,
                        completed,
                        attempted * (sizeof(Money) + (Broadcast ? 0 : sizeof(B))) + completed * sizeof(Money),
                        wide_products,
                        static_cast<std::size_t>(!reference),
                        0,
                        attempted,
                        delegated_products,
                        native_refused_products,
                        simd_products};
    for (const auto implementation : implementations) {
        dispatch(implementation, [&]<Implementation Impl>() {
            std::vector<Money> actual(count, sentinel);
            const auto result = products<Impl, Broadcast>(*input, actual);
            require_equal(workload + "/" + name(Impl), reference, result, expected, actual);
            const std::string benchmark_name = workload + "/" + name(Impl) + "/" + std::to_string(count);
            benchmark::RegisterBenchmark(benchmark_name.c_str(), [input, counts](benchmark::State& state) {
                std::vector<Money> output(input->left.size(), sentinel);
                for (auto _ : state) {
                    auto timed_result = products<Impl, Broadcast>(*input, output);
                    benchmark::DoNotOptimize(timed_result);
                    consume(output);
                }
                counters(state, counts, Impl);
            })->Unit(benchmark::kNanosecond);
        });
    }
}

struct Sweep {
    std::vector<Money> slope, residual, initial_value, initial_total;
    std::vector<Price> anchor;
    std::vector<std::uint32_t> balance;
    Price dx{Price::from_raw(37)};
    std::array<Price, 2> marks{alternate_mark, reference_mark};
    Money fx{conversion_rate};
};

struct SweepState {
    std::vector<Money> value, total;
    explicit SweepState(const Sweep& input) : value(input.initial_value), total(input.initial_total) {}
};

template<Implementation Impl>
Status delta(const Sweep& data, SweepState& output, Price dx) noexcept {
    // Keep each quote's factor runtime-dependent, so setup cannot be hoisted
    // out of the timed loop merely because this bounded corpus alternates.
    dx = runtime_value(dx);
    const auto commit = [&](std::size_t row, Money contribution) noexcept -> ArithmeticStatus {
        const auto after = fw::add(output.value[row], contribution);
        if (!after) return std::unexpected(after.error());
        const auto balance = data.balance[row];
        const auto total = fw::add(output.total[balance], contribution);
        if (!total) return std::unexpected(total.error());
        output.value[row] = *after;
        output.total[balance] = *total;
        return {};
    };
    if constexpr (Impl == Implementation::scalar) {
        for (std::size_t row = 0; row < data.slope.size(); ++row) {
            const auto contribution = fw::mul_to<Money>(data.slope[row], dx, exact);
            if (!contribution) return failure(row, contribution.error());
            const auto committed = commit(row, *contribution);
            if (!committed) return failure(row, committed.error());
        }
        return {};
    } else {
        // Prepared once per quote, inside the measured kernel. There is no
        // persistent converted slope array and no output contribution pass.
        const auto multiply = fw::batch::prepare_mul_to<Money>(dx, exact);
        constexpr std::size_t block_size = Impl == Implementation::batch4 ? 4 : 1;
        return fw::batch::for_each<block_size>(data.slope.size(), multiply, commit, view(data.slope));
    }
}

template<Implementation Impl>
Status anchored(const Sweep& data, SweepState& output, Price mark) noexcept {
    auto fx_factor = runtime_value(data.fx);
    mark = runtime_value(mark);
    const auto commit = [&](std::size_t row, Money next) noexcept -> ArithmeticStatus {
        const auto balance = data.balance[row];
        const auto total = replace(output.total[balance], output.value[row], next);
        if (!total) return std::unexpected(total.error());
        output.value[row] = next;
        output.total[balance] = *total;
        return {};
    };
    if constexpr (Impl == Implementation::scalar) {
        for (std::size_t row = 0; row < data.slope.size(); ++row) {
            // Corpus construction certifies the raw price difference.
            const auto dx = Price::from_raw(mark.raw() - data.anchor[row].raw());
            const auto product = fw::mul_to<Money>(data.slope[row], dx, nearest);
            if (!product) return failure(row, product.error());
            const auto pre_fx = fw::add(*product, data.residual[row]);
            if (!pre_fx) return failure(row, pre_fx.error());
            const auto next = fw::mul_to<Money>(*pre_fx, fx_factor, nearest);
            if (!next) return failure(row, next.error());
            const auto committed = commit(row, *next);
            if (!committed) return failure(row, committed.error());
        }
        return {};
    } else {
        const auto multiply = fw::batch::mul_to<Money>(nearest);
        const auto convert_fx = fw::batch::prepare_mul_to<Money>(fx_factor, nearest);
        const auto map = [&](Money slope, Money residual, Price anchor) noexcept -> ArithmeticResult<Money> {
            const auto dx = Price::from_raw(mark.raw() - anchor.raw());
            const auto product = multiply(slope, dx);
            if (!product) return std::unexpected(product.error());
            const auto pre_fx = fw::add(*product, residual);
            if (!pre_fx) return std::unexpected(pre_fx.error());
            return convert_fx(*pre_fx);
        };
        constexpr std::size_t block_size = Impl == Implementation::batch4 ? 4 : 1;
        return fw::batch::for_each<block_size>(data.slope.size(), map, commit, view(data.slope), view(data.residual),
                                               view(data.anchor));
    }
}

Sweep make_sweep(std::size_t count, bool duplicates, bool holes, std::mt19937_64& rng) {
    Sweep data;
    data.slope.resize(count);
    data.residual.resize(count);
    data.anchor.resize(count);
    data.initial_value.resize(count);
    data.balance.resize(count);
    const auto balance_count = duplicates ? (count + 3) / 4 : count;
    data.initial_total.resize(balance_count + 1); // last balance is the owner-local sink
    for (std::size_t row = 0; row < count; ++row) {
        const bool hole = holes && row % 20 == 0;
        const auto units = static_cast<std::int64_t>(rng() % 20'000'001) - 10'000'000;
        // Every slope has at most two fractional digits, including signs.
        data.slope[row] = Money::from_raw(hole ? Raw{} : Raw{units} * Raw{10'000'000'000ULL});
        data.residual[row] =
            Money::from_raw(hole ? Raw{} : Raw{static_cast<std::int64_t>(rng() % 2'000'001) - 1'000'000});
        data.anchor[row] = Price::from_raw(1'000'000'000'000LL + static_cast<std::int64_t>(rng() % 200'001) - 100'000);
        data.balance[row] = static_cast<std::uint32_t>(hole ? balance_count : (duplicates ? row / 4 : row));
        data.initial_value[row] =
            Money::from_raw(hole ? Raw{} : Raw{static_cast<std::int64_t>(rng() % 2'000'001) - 1'000'000});
        const auto sum = fw::add(data.initial_total[data.balance[row]], data.initial_value[row]);
        if (!sum) fail("sweep setup overflow");
        data.initial_total[data.balance[row]] = *sum;
    }
    return data;
}

template<bool Anchored>
void register_sweep(const std::string& workload, Sweep data) {
    const auto input = std::make_shared<const Sweep>(std::move(data));
    const auto count = input->slope.size();
    const Counts counts{count, count, count * (Anchored ? 6 : 3), count * (Anchored ? 108 : 84), 0, 0};
    const auto run = [&]<Implementation Impl>(SweepState& state, std::size_t iteration) {
        if constexpr (Anchored)
            return anchored<Impl>(*input, state, input->marks[iteration & 1U]);
        else
            return delta<Impl>(*input, state, (iteration & 1U) == 0 ? input->dx : Price::from_raw(-input->dx.raw()));
    };
    for (const auto implementation : implementations) {
        dispatch(implementation, [&]<Implementation Impl>() {
            SweepState expected(*input), actual(*input);
            // Repeated quotes exercise duplicate destinations and replacement
            // against the previous quote, not merely against zeroed outputs.
            for (std::size_t iteration = 0; iteration < 3; ++iteration) {
                const auto reference = run.template operator()<Implementation::scalar>(expected, iteration);
                const auto result = run.template operator()<Impl>(actual, iteration);
                if (!reference) fail(workload + " unexpectedly refused");
                require_equal(workload, reference, result, expected.value, actual.value);
                require_equal(workload, reference, result, expected.total, actual.total);
                if (actual.total.back() != Money{}) fail(workload + " sink changed");
            }
            const std::string benchmark_name = workload + "/" + name(Impl) + "/" + std::to_string(count);
            benchmark::RegisterBenchmark(benchmark_name.c_str(), [input, counts](benchmark::State& state) {
                SweepState output(*input);
                std::size_t iteration = 0;
                for (auto _ : state) {
                    Status result;
                    if constexpr (Anchored)
                        result = anchored<Impl>(*input, output, input->marks[iteration & 1U]);
                    else
                        result = delta<Impl>(*input, output,
                                             (iteration & 1U) == 0 ? input->dx : Price::from_raw(-input->dx.raw()));
                    ++iteration;
                    benchmark::DoNotOptimize(result);
                    consume(output.value);
                    consume(output.total);
                }
                counters(state, counts, Impl);
            })->Unit(benchmark::kNanosecond);
        });
    }
}

struct Replacement {
    std::vector<Money> total, previous, next;
};

template<Implementation Impl>
Status replacement(const Replacement& input, std::span<Money> output) noexcept {
    if constexpr (Impl == Implementation::scalar) {
        for (std::size_t row = 0; row < output.size(); ++row) {
            const auto result = replace(input.total[row], input.previous[row], input.next[row]);
            if (!result) return failure(row, result.error());
            output[row] = *result;
        }
        return {};
    } else {
        const auto commit = [&](std::size_t row, Money value) noexcept -> ArithmeticStatus {
            output[row] = value;
            return {};
        };
        constexpr std::size_t block_size = Impl == Implementation::batch4 ? 4 : 1;
        return fw::batch::for_each<block_size>(output.size(), replace, commit, view(input.total), view(input.previous),
                                               view(input.next));
    }
}

void register_replacement(std::size_t count) {
    Replacement data;
    data.total.resize(count);
    data.previous.resize(count);
    data.next.resize(count);
    constexpr Money high{"100000000000000000000000000"};
    for (std::size_t row = 0; row < count; ++row) {
        const auto sign = (row & 1U) != 0 ? -high.raw() : high.raw();
        data.total[row] = Money::from_raw(sign);
        data.previous[row] = Money::from_raw(-sign);
        data.next[row] = Money::from_raw(-sign);
    }
    const auto input = std::make_shared<const Replacement>(std::move(data));
    std::vector<Money> expected(count, sentinel);
    const auto reference = replacement<Implementation::scalar>(*input, expected);
    if (!reference || expected != input->total) fail("wide replacement fixture");
    const Counts counts{count, count, count * 3, count * 64, 0, 0};
    for (const auto implementation : implementations) {
        dispatch(implementation, [&]<Implementation Impl>() {
            std::vector<Money> actual(count, sentinel);
            const auto result = replacement<Impl>(*input, actual);
            require_equal("replace.wide_cancellation", reference, result, expected, actual);
            const std::string benchmark_name =
                std::string("replace.wide_cancellation/") + name(Impl) + "/" + std::to_string(count);
            benchmark::RegisterBenchmark(benchmark_name.c_str(), [input, counts](benchmark::State& state) {
                std::vector<Money> output(input->total.size(), sentinel);
                for (auto _ : state) {
                    auto timed_result = replacement<Impl>(*input, output);
                    benchmark::DoNotOptimize(timed_result);
                    consume(output);
                }
                counters(state, counts, Impl);
            })->Unit(benchmark::kNanosecond);
        });
    }
}

struct Publication {
    std::vector<Money> slope;
    std::vector<Price> execution;
    std::vector<std::uint8_t> last_entry;
    std::size_t positions{};
    Price mark{reference_mark};
    Money fx{conversion_rate};
};

template<Implementation Impl>
Status publication(const Publication& input, std::span<Money> output) noexcept {
    Money sum;
    std::size_t position = 0;
    auto fx_factor = runtime_value(input.fx);
    const auto fx = [fx_factor] {
        if constexpr (Impl == Implementation::scalar) {
            return [fx_factor](Money value) noexcept { return fw::mul_to<Money>(value, fx_factor, nearest); };
        } else {
            return fw::batch::prepare_mul_to<Money>(fx_factor, nearest);
        }
    }();
    const auto commit = [&](std::size_t row, Money term) noexcept -> ArithmeticStatus {
        const auto next = fw::add(sum, term);
        if (!next) return std::unexpected(next.error());
        if (input.last_entry[row] != 0) {
            const auto converted = fx(*next);
            if (!converted) return std::unexpected(converted.error());
            output[position++] = *converted;
            sum = Money{};
        } else {
            sum = *next;
        }
        return {};
    };
    if constexpr (Impl == Implementation::scalar) {
        for (std::size_t row = 0; row < input.slope.size(); ++row) {
            const auto dx = Price::from_raw(input.mark.raw() - input.execution[row].raw());
            const auto term = fw::mul_to<Money>(input.slope[row], dx, nearest);
            if (!term) return failure(row, term.error());
            const auto committed = commit(row, *term);
            if (!committed) return failure(row, committed.error());
        }
        return {};
    } else {
        const auto multiply = fw::batch::mul_to<Money>(nearest);
        const auto map = [&](Money slope, Price execution) noexcept {
            return multiply(slope, Price::from_raw(input.mark.raw() - execution.raw()));
        };
        constexpr std::size_t block_size = Impl == Implementation::batch4 ? 4 : 1;
        return fw::batch::for_each<block_size>(input.slope.size(), map, commit, view(input.slope),
                                               view(input.execution));
    }
}

// A decimal-grid coefficient beyond signed64 plus a fractional remainder.
// The raw product with the price corpus exceeds signed128, while the scaled
// result and its independently rounded FX conversion remain representable.
Money wide_rounded_slope(std::size_t row) {
    const Raw coefficient = (Raw{1} << 90) + Raw{row & 1023U};
    const Raw raw = coefficient * Raw{10'000'000'000ULL} + Raw{1};
    return Money::from_raw((row & 1U) == 0 ? raw : -raw);
}

void register_publication(std::size_t count, std::mt19937_64& rng, bool wide = false) {
    const std::string workload = wide ? "publication.wide_rounded" : "publication.segmented";
    Publication data;
    data.slope.resize(count);
    data.execution.resize(count);
    data.last_entry.resize(count);
    std::size_t remaining = 2;
    for (std::size_t row = 0; row < count; ++row) {
        data.slope[row] =
            Money::from_raw(Raw{static_cast<std::int64_t>(rng() % 2'000'000'001) - 1'000'000'000} * Raw{1000});
        if (wide) data.slope[row] = wide_rounded_slope(row);
        data.execution[row] = Price::from_raw(data.mark.raw() + static_cast<std::int64_t>(rng() % 200'001) - 100'000);
        --remaining;
        if (remaining == 0 || row + 1 == count) {
            data.last_entry[row] = 1;
            ++data.positions;
            remaining = 2 + data.positions % 4;
        }
    }
    // The first position is the two independent half-quantum terms: round
    // each to zero before summing. Combining slopes would instead produce 1.
    if (count >= 2) {
        data.slope[0] = data.slope[1] = Money{"0.005"};
        data.execution[0] = data.execution[1] = Price::from_raw(data.mark.raw() - 1);
    }
    const auto input = std::make_shared<const Publication>(std::move(data));
    const Counts counts{
        count, count,           count * 2 + input->positions, count * 25 + input->positions * sizeof(Money), 0,
        0,     input->positions};
    std::vector<Money> expected(input->positions, sentinel);
    const auto reference = publication<Implementation::scalar>(*input, expected);
    if (!reference || expected.front() != Money{}) fail("publication fixture");
    for (const auto implementation : implementations) {
        dispatch(implementation, [&]<Implementation Impl>() {
            std::vector<Money> actual(input->positions, sentinel);
            const auto result = publication<Impl>(*input, actual);
            require_equal(workload, reference, result, expected, actual);
            const std::string benchmark_name = workload + "/" + name(Impl) + "/" + std::to_string(count);
            benchmark::RegisterBenchmark(benchmark_name.c_str(), [input, counts](benchmark::State& state) {
                std::vector<Money> output(input->positions, sentinel);
                for (auto _ : state) {
                    auto timed_result = publication<Impl>(*input, output);
                    benchmark::DoNotOptimize(timed_result);
                    consume(output);
                }
                counters(state, counts, Impl);
            })->Unit(benchmark::kNanosecond);
        });
    }
}

void register_fixtures() {
    std::mt19937_64 rng(0xB47C'2026);
    // Extra cases must not change the established controls' retained corpus.
    std::mt19937_64 wide_rng(0xB47C'1286);
    for (const auto count : sizes) {
        Products<Price> price;
        Products<Money> fx;
        price.left.resize(count);
        price.right.resize(count);
        price.broadcast = Price::from_raw(37);
        price.rounding = exact;
        fx.left.resize(count);
        fx.right.resize(count);
        fx.broadcast = Money{"1.234567890123"};
        for (std::size_t row = 0; row < count; ++row) {
            const auto units = static_cast<std::int64_t>(rng() % 20'000'001) - 10'000'000;
            price.left[row] = Money::from_raw(Raw{units} * Raw{10'000'000'000ULL});
            price.right[row] = Price::from_raw(static_cast<std::int64_t>(rng() % 20'000'001) - 10'000'000);
            fx.left[row] = Money::from_raw(
                Raw{static_cast<std::int64_t>(rng() % 2'000'000'000'001ULL) - 1'000'000'000'000LL} * Raw{1000});
            fx.right[row] = Money::from_raw(Raw{rng() % 4'800'000'000'001ULL + 200'000'000'000ULL});
        }
        register_products<true>("mul_price.broadcast_exact", price);
        register_products<false>("mul_price.varying_exact", price);
        auto wide_price = price;
        wide_price.broadcast = Price::from_raw((std::int64_t{1} << 43) + 17);
        register_products<true>("mul_price.grid32_wide_factor", std::move(wide_price));
        auto wide_coefficient = price;
        for (std::size_t row = 0; row < count; ++row) {
            const Raw coefficient = (Raw{1} << 50) + Raw{row & 1023U};
            const Raw raw = coefficient * Raw{10'000'000'000ULL};
            wide_coefficient.left[row] = Money::from_raw((row & 1U) != 0 ? -raw : raw);
        }
        register_products<true>("mul_price.grid64_scalar_fallback", wide_coefficient);
        for (std::size_t row = 0; row < count; ++row) {
            const Raw coefficient = (Raw{1} << 90) + Raw{row & 1023U};
            const Raw raw = coefficient * Raw{10'000'000'000ULL};
            wide_coefficient.left[row] = Money::from_raw((row & 1U) != 0 ? -raw : raw);
        }
        register_products<true>("mul_price.wide_coefficient_fallback", std::move(wide_coefficient));
        auto wide_rounded = price;
        wide_rounded.rounding = nearest;
        for (std::size_t row = 0; row < count; ++row) {
            wide_rounded.left[row] = wide_rounded_slope(row);
            wide_rounded.right[row] = Price::from_raw(37 + static_cast<std::int64_t>(row % 5) * 2);
        }
        register_products<true>("mul_price.wide_rounded_broadcast", wide_rounded);
        register_products<false>("mul_price.wide_rounded_varying", wide_rounded);
        auto restored_exact = wide_rounded;
        restored_exact.rounding = exact;
        restored_exact.broadcast = Price::from_raw(10'000'000'000LL);
        for (std::size_t row = 0; row < count; ++row) {
            // Low ten bits are clear, but the source is not on the decimal
            // grid. The factor restores exactness after certificate rejection.
            const Raw coefficient = (Raw{1} << 80) + Raw{row & 1023U};
            const Raw raw = coefficient * Raw{10'000'000'000ULL} + Raw{1024};
            restored_exact.left[row] = Money::from_raw((row & 1U) == 0 ? raw : -raw);
        }
        register_products<true>("mul_price.offgrid_factor_cancels", std::move(restored_exact));
        register_products<true>("mul_fx.broadcast_nearest", fx);
        register_products<false>("mul_fx.varying_nearest", fx);

        // An inexact lane sits inside a block, after an accepted prefix. The
        // untouched suffix remains sentinel, and throughput uses only the
        // completed prefix rather than pretending the entire array succeeded.
        price.left[count / 2 + 1] = Money::from_raw(Raw{1});
        register_products<true>("mul_price.refused_exact", price, true);

        price.rounding = nearest;
        for (std::size_t row = 0; row < count; ++row) {
            const auto low = rng();
            const Raw magnitude{low, (std::uint64_t{1} << 26) + (rng() & 1023U)};
            price.left[row] = Money::from_raw((row & 1U) != 0 ? -magnitude : magnitude);
            price.right[row] = Price::from_raw(static_cast<std::int64_t>((std::uint64_t{1} << 45) + (rng() & 65535U)));
            if (!product_outside_i128(price.left[row], price.right[row])) fail("wide product fixture did not widen");
        }
        register_products<false>("mul_price.wide_success", price);

        // Both raw magnitudes exceed uint64. The final scale12 result still
        // fits Money, so this exercises successful general scalar fallback
        // after the specialized batch multiply declines. Choose a broadcast
        // relatively prime to 10^12 so preparation cannot reduce it to a limb.
        fx.broadcast = Money::from_raw(Raw{3, 4});
        for (std::size_t row = 0; row < count; ++row) {
            const Raw left{rng(), 16 + (rng() & 15U)};
            fx.left[row] = Money::from_raw((row & 1U) != 0 ? -left : left);
            fx.right[row] = Money::from_raw(Raw{rng(), 4 + (rng() & 3U)});
            if (!product_outside_i128(fx.left[row], fx.right[row])) fail("wide FX fixture did not widen");
        }
        register_products<true>("mul_fx.wide_broadcast_success", fx);
        register_products<false>("mul_fx.wide_varying_success", fx);

        register_sweep<false>("delta.unique.holes0", make_sweep(count, false, false, rng));
        register_sweep<false>("delta.duplicates.holes5", make_sweep(count, true, true, rng));
        auto mixed_sweep = make_sweep(count, true, true, rng);
        for (std::size_t row = 1; row < count; row += 20) {
            const Raw coefficient = (Raw{1} << 90) + Raw{row & 1023U};
            const Raw raw = coefficient * Raw{10'000'000'000ULL};
            mixed_sweep.slope[row] = Money::from_raw((row & 2U) != 0 ? -raw : raw);
        }
        register_sweep<false>("delta.mixed5pct_wide.duplicates.holes5", std::move(mixed_sweep));
        auto grid_sweep = make_sweep(count, true, true, rng);
        grid_sweep.dx = Price{"0.0001"};
        register_sweep<false>("delta.grid.duplicates.holes5", std::move(grid_sweep));
        register_sweep<true>("anchored_replace.duplicates.holes5", make_sweep(count, true, true, rng));
        auto wide_anchored = make_sweep(count, true, true, wide_rng);
        for (std::size_t row = 0; row < count; ++row)
            if (row % 20 != 0) wide_anchored.slope[row] = wide_rounded_slope(row);
        register_sweep<true>("anchored_replace.wide_rounded.duplicates.holes5", std::move(wide_anchored));
        register_replacement(count);
        register_publication(count, rng);
        register_publication(count, wide_rng, true);
    }
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
    benchmark::AddCustomContext("compiler", __VERSION__);
    benchmark::AddCustomContext("backend", backend);
    benchmark::AddCustomContext("seed", "0xB47C2026");
    benchmark::AddCustomContext("wide_seed", "0xB47C1286");
    benchmark::AddCustomContext("baseline", "public scalar operations from the same source revision");
    benchmark::AddCustomContext(
        "model", "complete arithmetic loops; modeled bytes exclude cache effects and speculative failed-block lanes");
#if defined(FIXEDWIDE_GOOGLE_BENCHMARK_VERSION)
    benchmark::AddCustomContext("google_benchmark", FIXEDWIDE_GOOGLE_BENCHMARK_VERSION);
#endif
    register_fixtures();
    benchmark::AddCustomContext("validated_output_rows", std::to_string(validated_rows));
    std::fprintf(
        stderr, "validated %zu output rows (values, errors, prefix writes, repeated quotes, duplicate/sink balances)\n",
        validated_rows);
    if (!validate_only) benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
}
