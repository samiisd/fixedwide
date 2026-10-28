// Opt-in instruction fixture, using scripts/icount.sh's workload/N protocol.
// Each call completes 400 input rows. The comparison script records per-call
// and per-row counts; publication additionally completes 100 positions/call.
// Rational validation and final output consumption are independent of N and
// cancel in I(2N)-I(N). Kernels live in separate translation units without LTO.
#include <fixedwide/batch.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace fw = fixedwide;
using Money = fw::Fixed128<12>;
using Price = fw::Fixed64<10>;
using Status = fw::batch::Result;
using boost::multiprecision::cpp_int;

namespace fixedwide_batch_codegen {
Status scalar_multiply(Money*, const Money*, std::size_t, Price) noexcept;
Status prepared_scalar_multiply(Money*, const Money*, std::size_t, Price) noexcept;
Status scalar_fx(Money*, const Money*, std::size_t, Money) noexcept;
Status prepared_scalar_fx(Money*, const Money*, std::size_t, Money) noexcept;
Status scalar_delta(const Money*, Money*, Money*, const std::uint32_t*, std::size_t, Price) noexcept;
Status prepared_scalar_delta(const Money*, Money*, Money*, const std::uint32_t*, std::size_t, Price) noexcept;
} // namespace fixedwide_batch_codegen
namespace fixedwide_fallback_codegen {
Status public_anchored(const Money*, const Money*, const Price*, const std::uint32_t*, Money*, Money*, std::size_t,
                       Price, Money) noexcept;
Status prepared_anchored(const Money*, const Money*, const Price*, const std::uint32_t*, Money*, Money*, std::size_t,
                         Price, Money) noexcept;
Status public_publication(const Money*, const Price*, Money*, std::size_t, Price, Money) noexcept;
Status prepared_publication(const Money*, const Price*, Money*, std::size_t, Price, Money) noexcept;
Status public_rounded_price(Money*, const Money*, std::size_t, Price) noexcept;
Status prepared_rounded_price(Money*, const Money*, std::size_t, Price) noexcept;
} // namespace fixedwide_fallback_codegen

namespace {
constexpr std::size_t rows = 400;
constexpr std::int64_t grid = 10'000'000'000LL;
constexpr std::int64_t base_mark = 1'000'000'000'000LL;
constexpr std::int64_t fx_raw = 1'000'000'001'234LL;

enum class Operation { product, delta, fx, rounded_price, anchored, publication };
struct Case {
    std::string name;
    std::string_view shape;
    Operation operation;
    bool prepared;
};

std::vector<Case> cases() {
    std::vector<Case> result;
    const auto add = [&](std::string_view prefix, Operation operation, std::string_view shape) {
        for (const bool prepared : {false, true})
            result.push_back({std::string(prefix) + "." + std::string(shape) + (prepared ? ".prepared" : ".public"),
                              shape, operation, prepared});
    };
    for (const auto shape : {"q32", "q63plus1", "q80", "q90", "mixed5", "offgrid"}) {
        add("product", Operation::product, shape);
        add("delta", Operation::delta, shape);
    }
    add("fx", Operation::fx, "ordinary");
    for (const auto shape : {"exact", "inexact"}) add("rounded_price", Operation::rounded_price, shape);
    for (const auto shape : {"q32", "q90", "mixed5"}) {
        add("anchored", Operation::anchored, shape);
        add("publication", Operation::publication, shape);
    }
    return result;
}

[[noreturn]] void fail(const char* reason) {
    std::fprintf(stderr, "fallback fixture validation failed: %s\n", reason);
    std::exit(2);
}

Money money(cpp_int value) {
    if (value < -(cpp_int{1} << 127) || value >= (cpp_int{1} << 127)) fail("fixture range");
    if (value < 0) value += cpp_int{1} << 128;
    return Money::from_raw(fw::wide::int128{static_cast<std::uint64_t>(value & ((cpp_int{1} << 64) - 1)),
                                            static_cast<std::uint64_t>(value >> 64)});
}

cpp_int integer(Money value) {
    const auto raw = value.raw();
    cpp_int result = (cpp_int{raw.high} << 64) | raw.low;
    if ((raw.high >> 63) != 0) result -= cpp_int{1} << 128;
    return result;
}

cpp_int rational(cpp_int numerator, std::uint64_t denominator, bool rounded) {
    const bool negative = numerator < 0;
    if (negative) numerator = -numerator;
    cpp_int quotient = numerator / denominator;
    const cpp_int remainder = numerator % denominator;
    if (!rounded && remainder != 0) fail("inexact fixture");
    if (rounded && (remainder * 2 > denominator || (remainder * 2 == denominator && (quotient & 1) != 0))) ++quotient;
    const cpp_int limit = (cpp_int{1} << 127) - (negative ? 0 : 1);
    if (quotient > limit) fail("oracle output range");
    return negative ? -quotient : quotient;
}

struct Fixture {
    std::array<Money, rows> slopes{}, residuals{}, values{}, totals{};
    std::array<Price, rows> anchors{};
    std::array<std::uint32_t, rows> destinations{};
    std::int64_t factor = 37;
};

bool composed(const Case& test) {
    return test.operation == Operation::anchored || test.operation == Operation::publication;
}

Fixture fixture(const Case& test) {
    Fixture data;
    if (test.shape == "offgrid") data.factor = grid;
    if (test.operation == Operation::fx) data.factor = fx_raw;
    for (std::size_t row = 0; row < rows; ++row) {
        cpp_int coefficient = 1'000'000 + row;
        if (test.shape == "q63plus1") coefficient += cpp_int{1} << 63;
        if (test.shape == "q80" || test.shape == "offgrid") coefficient += cpp_int{1} << 80;
        // The composed fixture replaces row zero with a rounding-boundary
        // entry below. Row four keeps its sparse-wide population at 20/400.
        if (test.shape == "q90" || (test.shape == "mixed5" && (row % 20 == 0 || (composed(test) && row == 4))) ||
            test.operation == Operation::rounded_price)
            coefficient += cpp_int{1} << 90;
        cpp_int raw = coefficient * grid;
        if (test.shape == "offgrid") raw += 1024; // low ten bits clear, certificate still misses
        if (test.shape == "inexact") raw += 1;
        if (composed(test)) raw += row % 3 + 1;
        if (test.operation == Operation::fx) raw = cpp_int{1'234'567'890'123LL} + row;
        if ((row & 1U) != 0) raw = -raw;
        const bool hole = composed(test) && row % 20 == 19;
        data.slopes[row] = money(hole ? cpp_int{0} : raw);
        data.residuals[row] = money(hole ? cpp_int{0} : cpp_int{row % 127});
        data.anchors[row] = Price::from_raw(base_mark + static_cast<std::int64_t>(row % 31) - 15);
        data.destinations[row] =
            static_cast<std::uint32_t>(composed(test) ? (hole ? rows / 8 : row / 8) : (row * 7) % 32);
    }
    if (composed(test)) {
        // Individual nearest-even rounding matters: two 0.005 slopes at one
        // raw price unit each produce zero. At 25 units each produces 12.
        for (std::size_t row = 0; row < 4; ++row) {
            data.slopes[row] = money(row < 2 ? cpp_int{5'000'000'000LL} : cpp_int{0});
            data.residuals[row] = Money{};
            data.anchors[row] = Price::from_raw(base_mark + 36);
        }
    }
    return data;
}

Status invoke(const Case& test, Fixture& data, std::uint64_t quote) {
    namespace old = fixedwide_batch_codegen;
    namespace full = fixedwide_fallback_codegen;
    const auto factor = (quote & 1U) == 0 ? data.factor : -data.factor;
    const auto mark = Price::from_raw(base_mark + ((quote & 1U) == 0 ? 37 : 61));
    switch (test.operation) {
    case Operation::product:
        return (test.prepared ? old::prepared_scalar_multiply : old::scalar_multiply)(
            data.values.data(), data.slopes.data(), rows, Price::from_raw(factor));
    case Operation::delta:
        return (test.prepared ? old::prepared_scalar_delta
                              : old::scalar_delta)(data.slopes.data(), data.values.data(), data.totals.data(),
                                                   data.destinations.data(), rows, Price::from_raw(factor));
    case Operation::fx:
        return (test.prepared ? old::prepared_scalar_fx : old::scalar_fx)(data.values.data(), data.slopes.data(), rows,
                                                                          Money::from_raw(fw::wide::int128{factor}));
    case Operation::rounded_price:
        return (test.prepared ? full::prepared_rounded_price : full::public_rounded_price)(
            data.values.data(), data.slopes.data(), rows, Price::from_raw(factor));
    case Operation::anchored:
        return (test.prepared ? full::prepared_anchored : full::public_anchored)(
            data.slopes.data(), data.residuals.data(), data.anchors.data(), data.destinations.data(),
            data.values.data(), data.totals.data(), rows, mark, Money::from_raw(fw::wide::int128{fx_raw}));
    case Operation::publication:
        return (test.prepared ? full::prepared_publication
                              : full::public_publication)(data.slopes.data(), data.anchors.data(), data.values.data(),
                                                          rows, mark, Money::from_raw(fw::wide::int128{fx_raw}));
    }
    fail("invalid operation");
}

void validate(const Case& test, Fixture& data) {
    std::array<cpp_int, rows> expected_values{}, expected_totals{};
    for (std::uint64_t quote = 0; quote < 2; ++quote) {
        if (!invoke(test, data, quote)) fail("API refusal");
        const auto factor = (quote & 1U) == 0 ? data.factor : -data.factor;
        const auto mark = base_mark + ((quote & 1U) == 0 ? 37 : 61);
        if (test.operation == Operation::publication) {
            for (std::size_t position = 0; position < rows / 4; ++position) {
                cpp_int sum = 0;
                for (std::size_t lane = 0; lane < 4; ++lane) {
                    const auto row = position * 4 + lane;
                    sum += rational(integer(data.slopes[row]) * (mark - data.anchors[row].raw()), grid, true);
                }
                expected_values[position] = rational(sum * fx_raw, base_mark, true);
            }
        } else {
            for (std::size_t row = 0; row < rows; ++row) {
                cpp_int next;
                if (test.operation == Operation::anchored) {
                    const auto entry =
                        rational(integer(data.slopes[row]) * (mark - data.anchors[row].raw()), grid, true);
                    next = rational((entry + integer(data.residuals[row])) * fx_raw, base_mark, true);
                    expected_totals[data.destinations[row]] -= expected_values[row];
                    expected_totals[data.destinations[row]] += next;
                } else {
                    const bool rounded = test.operation == Operation::fx || test.operation == Operation::rounded_price;
                    next = rational(integer(data.slopes[row]) * factor,
                                    test.operation == Operation::fx ? base_mark : grid, rounded);
                    if (test.operation == Operation::delta) {
                        expected_totals[data.destinations[row]] += next;
                        next += expected_values[row];
                    }
                }
                expected_values[row] = next;
            }
        }
        for (std::size_t row = 0; row < rows; ++row) {
            if (integer(data.values[row]) != expected_values[row]) fail("row value or untouched suffix");
            if (integer(data.totals[row]) != expected_totals[row]) fail("ordered balance or sink");
        }
    }
}
} // namespace

int main(int argc, char** argv) {
    const auto workloads = cases();
    if (argc == 2 && std::string_view{argv[1]} == "--list") {
        for (const auto& test : workloads) std::puts(test.name.c_str());
        return 0;
    }
    if (argc != 3) return 1;
    std::uint64_t iterations = 0;
    const std::string_view number{argv[2]};
    const auto parsed = std::from_chars(number.data(), number.data() + number.size(), iterations);
    if (parsed.ec != std::errc{} || parsed.ptr != number.data() + number.size() || iterations == 0 ||
        (iterations & 1U) != 0 || iterations > 1'000'000)
        return 1;
    for (const auto& test : workloads) {
        if (test.name != argv[1]) continue;
        auto data = fixture(test);
        validate(test, data);
        // Every fixture has period two. Retain the independently validated
        // state and check the whole result after the even-N repetition loop;
        // a checksum alone could miss compensating corruption. This fixed
        // work is the same in the N and 2N processes.
        const auto expected_values = data.values;
        const auto expected_totals = data.totals;
        for (std::uint64_t quote = 0; quote < iterations; ++quote)
            if (!invoke(test, data, quote)) fail("repeated API refusal");
        if (data.values != expected_values || data.totals != expected_totals)
            fail("repeated calls changed the validated period-two state");
        std::uint64_t digest = 0;
        for (std::size_t row = 0; row < rows; ++row)
            digest ^= data.values[row].raw().low ^ data.values[row].raw().high ^ data.totals[row].raw().low ^
                      data.totals[row].raw().high;
        std::printf("completed_calls=%llu completed_rows_per_call=%zu completed_positions_per_call=%zu digest=%llu\n",
                    static_cast<unsigned long long>(iterations), rows,
                    test.operation == Operation::publication ? rows / 4 : 0, static_cast<unsigned long long>(digest));
        return 0;
    }
    return 1;
}
