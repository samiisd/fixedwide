// Native qualification for the bounded benchmark-only vector candidates.
// Unsupported backends are named explicitly and are never called.
#include <fixedwide/batch.hpp>
#include "../benchmarks/zen5_simd.hpp"
#include "../benchmarks/zen5_ifma.hpp"
#include "../benchmarks/zen5_kernels.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include "check.hpp"
#include <array>
#include <atomic>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <new>
#include <optional>
#include <random>
#include <vector>

#if defined(FIXEDWIDE_HAS_X86_64_ASM) && defined(FIXEDWIDE_HAS_MIXED_NATIVE) && !defined(_MSC_VER)
#include <cpuid.h>

static std::atomic<std::uint64_t> allocations{0};

void* operator new(std::size_t size) {
    void* result = std::malloc(size == 0 ? 1 : size);
    if (result == nullptr) throw std::bad_alloc{};
    allocations.fetch_add(1, std::memory_order_relaxed);
    return result;
}
void* operator new[](std::size_t size) {
    return ::operator new(size);
}
void operator delete(void* pointer) noexcept {
    std::free(pointer);
}
void operator delete[](void* pointer) noexcept {
    std::free(pointer);
}
void operator delete(void* pointer, std::size_t) noexcept {
    std::free(pointer);
}
void operator delete[](void* pointer, std::size_t) noexcept {
    std::free(pointer);
}

namespace {
namespace fw = fixedwide;
namespace candidate = fixedwide_zen5;
using boost::multiprecision::cpp_int;
using Money = fw::Fixed128<12>;
using Tile = unsigned (*)(const Money*, std::int64_t, Money*) noexcept;

enum class Coefficients { avx2, q52, q64, q94 };

struct Backend {
    const char* name;
    unsigned lanes;
    Coefficients coefficients;
    Tile tile;
    bool supported;
};

std::uint64_t vector_state() {
    unsigned eax, ebx, ecx, edx;
    if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx) ||
        (ecx & (bit_XSAVE | bit_OSXSAVE | bit_AVX)) != (bit_XSAVE | bit_OSXSAVE | bit_AVX))
        return 0;
    __asm__("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
    return (std::uint64_t{edx} << 32) | eax;
}

[[gnu::target("avx2")]] unsigned avx2_tile(const Money* input, std::int64_t factor, Money* output) noexcept {
    namespace simd = fw::detail::batch::simd;
    const simd::Factor prepared{factor};
    const simd::Tile result = prepared.signed32() ? simd::evaluate<10, simd::Kind::signed32>(input, prepared)
                              : prepared.magnitude <= UINT32_MAX
                                  ? simd::evaluate<10, simd::Kind::unsigned32>(input, prepared)
                                  : simd::evaluate<10, simd::Kind::unsigned64>(input, prepared);
    simd::store(output, result.values);
    return result.accepted;
}

std::array<Backend, 7> backends() {
    __builtin_cpu_init();
    const auto state = vector_state();
    const bool avx = (state & 0x6u) == 0x6u;
    const bool avx512 = (state & 0xe6u) == 0xe6u && __builtin_cpu_supports("avx2") && __builtin_cpu_supports("avx512f");
    const bool vl = avx512 && __builtin_cpu_supports("avx512vl");
    const bool dq = avx512 && __builtin_cpu_supports("avx512dq");
    const bool ifma = avx512 && __builtin_cpu_supports("avx512ifma");
    std::printf("Zen 5 candidate correctness host XCR0=0x%llx (timing is not measured here)\n",
                static_cast<unsigned long long>(state));
    return {{{"avx2", 4, Coefficients::avx2, avx2_tile, avx && __builtin_cpu_supports("avx2")},
             {"avx512vl64", 4, Coefficients::q64, candidate::evex256::tile<false>, dq && vl},
             {"avx51264", 8, Coefficients::q64, candidate::evex512::tile<false>, dq},
             {"avx512vl94", 4, Coefficients::q94, candidate::evex256::tile<true>, dq && vl},
             {"avx51294", 8, Coefficients::q94, candidate::evex512::tile<true>, dq},
             {"ifma256", 4, Coefficients::q52, candidate::ifma256_tile<Money>, ifma && vl},
             {"ifma512", 8, Coefficients::q52, candidate::ifma512_tile<Money>, ifma}}};
}

Money from_integer(cpp_int value) {
    if (value < 0) value += cpp_int{1} << 128;
    const cpp_int mask = (cpp_int{1} << 64) - 1;
    return Money::from_raw(
        fw::wide::int128{static_cast<std::uint64_t>(value & mask), static_cast<std::uint64_t>(value >> 64)});
}

cpp_int integer(Money value) {
    const auto raw = value.raw();
    cpp_int result = (cpp_int{raw.high} << 64) | raw.low;
    if ((raw.high >> 63) != 0) result -= cpp_int{1} << 128;
    return result;
}

bool accepts(Coefficients coefficients, const cpp_int& value, std::int64_t factor) {
    constexpr std::uint64_t divisor = 10'000'000'000ULL;
    if (value % divisor != 0) return false;
    const cpp_int quotient = value / divisor;
    const cpp_int magnitude = quotient < 0 ? -quotient : quotient;
    if (coefficients == Coefficients::avx2) {
        if (factor >= INT32_MIN && factor <= INT32_MAX) {
            if (quotient < INT32_MIN || quotient > INT32_MAX) return false;
        } else if (magnitude > UINT32_MAX) {
            return false;
        }
    } else if (coefficients == Coefficients::q52) {
        if (magnitude >= (cpp_int{1} << 52)) return false;
    } else if (coefficients == Coefficients::q64) {
        if (magnitude >= (cpp_int{1} << 64)) return false;
    }
    const cpp_int product = quotient * factor;
    const cpp_int limit = cpp_int{1} << 127;
    return product >= -limit && product < limit;
}

// The eight-byte prefix prevents vector alignment while retaining Money's
// natural alignment. Guards also detect a full-tile store past its width.
struct alignas(64) Buffer {
    std::uint64_t prefix = 0xC011EC7;
    std::array<Money, 8> values;
    std::uint64_t suffix = 0xF17ED;
};
static_assert(offsetof(Buffer, values) % 32 != 0);

void check_tile(const Backend& backend, const std::array<cpp_int, 8>& values, std::int64_t factor) {
    Buffer input{}, output{};
    const auto sentinel = Money::min();
    for (unsigned lane = 0; lane < 8; ++lane) {
        input.values[lane] = from_integer(values[lane]);
        output.values[lane] = sentinel;
    }
    const auto original = input.values;
    const unsigned mask = backend.tile(input.values.data(), factor, output.values.data());
    CHECK((mask >> backend.lanes) == 0);
    CHECK(input.values == original);
    for (unsigned lane = 0; lane < backend.lanes; ++lane) {
        const bool accepted = accepts(backend.coefficients, values[lane], factor);
        CHECK(((mask >> lane) & 1u) == static_cast<unsigned>(accepted));
        // Rejected output is deliberately ignored: only the mask certifies it.
        if (accepted) CHECK(integer(output.values[lane]) == values[lane] / 10'000'000'000ULL * factor);
    }
    for (unsigned lane = backend.lanes; lane < 8; ++lane) CHECK(output.values[lane] == sentinel);
    CHECK(input.prefix == 0xC011EC7 && input.suffix == 0xF17ED);
    CHECK(output.prefix == 0xC011EC7 && output.suffix == 0xF17ED);

    const auto inplace_mask = backend.tile(input.values.data(), factor, input.values.data());
    CHECK(inplace_mask == mask);
    for (unsigned lane = 0; lane < backend.lanes; ++lane)
        if ((mask & (1u << lane)) != 0) CHECK(input.values[lane] == output.values[lane]);
    for (unsigned lane = backend.lanes; lane < 8; ++lane) CHECK(input.values[lane] == original[lane]);
    CHECK(input.prefix == 0xC011EC7 && input.suffix == 0xF17ED);
}

std::vector<cpp_int> inputs() {
    const cpp_int limit = cpp_int{1} << 127;
    const cpp_int divisor = 10'000'000'000ULL;
    const cpp_int maximum = (limit - 1) / divisor;
    std::vector<cpp_int> values{-limit, -limit + 1, -1, 0, 1, limit - 2, limit - 1};
    const auto add = [&](const cpp_int& coefficient) {
        for (int sign : {-1, 1}) {
            for (int delta : {-1024, -1, 0, 1, 1024}) {
                const cpp_int value = coefficient * divisor * sign + delta;
                if (value >= -limit && value < limit) values.push_back(value);
            }
        }
    };
    add(0);
    add(1);
    add(maximum);
    for (unsigned bits : {31u, 32u, 51u, 52u, 53u, 54u, 63u, 64u, 80u, 93u, 94u, 95u})
        for (int delta : {-1, 0, 1}) add((cpp_int{1} << bits) + delta);
    std::mt19937_64 random(0x51ADCE471F1ED);
    for (unsigned i = 0; i < 800; ++i) {
        const auto high = random(), low = random();
        cpp_int value = (cpp_int{high} << 64) | low;
        if ((value & limit) != 0) value -= limit * 2;
        if (i % 3 == 0) value = value / divisor * divisor;
        if (i % 3 == 1) value = (value / divisor * divisor) + 1024;
        if (value >= -limit && value < limit) values.push_back(value);
    }
    return values;
}

void check_backend(const Backend& backend, const std::vector<cpp_int>& values) {
    constexpr std::array<std::int64_t, 14> factors{
        INT64_MIN, INT64_MIN + 1,   -4'294'967'295LL, -2'147'483'649LL, INT32_MIN,        -1,       0, 1,
        INT32_MAX, 2'147'483'648LL, 4'294'967'295LL,  10'000'000'000LL, 20'000'000'000LL, INT64_MAX};
    for (const auto factor : factors) {
        for (std::size_t row = 0; row < values.size(); row += backend.lanes) {
            std::array<cpp_int, 8> tile;
            for (unsigned lane = 0; lane < 8; ++lane) tile[lane] = values[(row + lane) % values.size()];
            check_tile(backend, tile, factor);
        }
    }
    // Exhaust every accepted/rejected mask, with a real off-grid value in
    // each rejected lane rather than simulated eligibility counters.
    for (unsigned mask = 0; mask < (1u << backend.lanes); ++mask) {
        std::array<cpp_int, 8> tile;
        for (unsigned lane = 0; lane < 8; ++lane) {
            tile[lane] = cpp_int{lane + 1} * 10'000'000'000ULL;
            if ((mask & (1u << lane)) == 0) ++tile[lane];
        }
        check_tile(backend, tile, -37);
    }
}

struct Expected {
    std::vector<cpp_int> values, balances;
    std::optional<fw::batch::Error> error;
    std::size_t completed = 0;
};

Expected oracle(candidate::Kernel kernel, const std::vector<Money>& slopes, std::int64_t factor,
                const std::vector<Money>& values, const std::vector<std::size_t>& destinations,
                const std::vector<Money>& balances) {
    Expected result;
    for (auto value : values) result.values.push_back(integer(value));
    for (auto value : balances) result.balances.push_back(integer(value));
    const cpp_int limit = cpp_int{1} << 127;
    const auto fits = [&](const cpp_int& value) { return value >= -limit && value < limit; };
    const auto fail = [&](std::size_t row, fw::ArithmeticError error) { result.error = fw::batch::Error{row, error}; };
    for (std::size_t row = 0; row < slopes.size(); ++row) {
        const cpp_int numerator = integer(slopes[row]) * factor;
        const cpp_int product = numerator / 10'000'000'000ULL;
        if (!fits(product)) {
            fail(row, fw::ArithmeticError::overflow);
            break;
        }
        if (numerator % 10'000'000'000ULL != 0) {
            fail(row, fw::ArithmeticError::inexact);
            break;
        }
        if (kernel == candidate::Kernel::product) {
            result.values[row] = product;
        } else {
            const cpp_int next = result.values[row] + product;
            if (!fits(next)) {
                fail(row, fw::ArithmeticError::overflow);
                break;
            }
            const auto destination = destinations[row];
            if (destination != candidate::sink) {
                const cpp_int next_balance = result.balances[destination] + product;
                if (!fits(next_balance)) {
                    fail(row, fw::ArithmeticError::overflow);
                    break;
                }
                result.balances[destination] = next_balance;
            }
            result.values[row] = next;
        }
        ++result.completed;
    }
    return result;
}

void check_call(candidate::Backend backend, candidate::Kernel kernel, const std::vector<Money>& slopes,
                std::int64_t factor, const std::vector<Money>& values, const std::vector<std::size_t>& destinations,
                const std::vector<Money>& balances, bool inplace = false) {
    const auto& initial = inplace ? slopes : values;
    const auto expected = oracle(kernel, slopes, factor, initial, destinations, balances);
    for (bool instrumented : {false, true}) {
        auto actual_values = initial;
        auto actual_balances = balances;
        const std::span<const Money> source = inplace ? std::span<const Money>{actual_values} : std::span{slopes};
        candidate::Diagnostics diagnostics;
        const auto before = allocations.load(std::memory_order_relaxed);
        const auto status = candidate::execute(backend, kernel, source, factor, actual_values, destinations,
                                               actual_balances, instrumented ? &diagnostics : nullptr);
        CHECK(allocations.load(std::memory_order_relaxed) == before);
        CHECK(status.has_value() == !expected.error.has_value());
        if (expected.error) {
            CHECK(status.error().index == expected.error->index);
            CHECK(status.error().error == expected.error->error);
        }
        for (std::size_t row = 0; row < actual_values.size(); ++row)
            CHECK(integer(actual_values[row]) == expected.values[row]);
        for (std::size_t row = 0; row < actual_balances.size(); ++row)
            CHECK(integer(actual_balances[row]) == expected.balances[row]);
        if (instrumented) {
            CHECK(diagnostics.completed_rows == expected.completed);
            const auto published = diagnostics.commit_calls + diagnostics.direct_store_rows;
            CHECK(published >= diagnostics.completed_rows);
            CHECK(published <= diagnostics.completed_rows + 1);
            if (kernel == candidate::Kernel::delta) CHECK(diagnostics.direct_store_rows == 0);
            CHECK(diagnostics.simd_commits <= diagnostics.completed_rows);
            CHECK(diagnostics.simd_accepted <= diagnostics.simd_evaluated);
            CHECK(diagnostics.simd_evaluated == diagnostics.tile_calls * candidate::block_lanes(backend));
            if (candidate::block_lanes(backend) > 1)
                CHECK(diagnostics.scalar_maps == diagnostics.fallback_maps + diagnostics.tail_maps);
            else
                CHECK(diagnostics.simd_evaluated == 0);
        }
    }
}

void check_wrappers(candidate::Backend backend) {
    constexpr std::uint64_t divisor = 10'000'000'000ULL;
    for (unsigned size = 0; size <= 19; ++size) {
        std::vector<Money> slopes, values;
        std::vector<std::size_t> destinations;
        std::vector<Money> balances(3, from_integer(-100));
        for (unsigned row = 0; row < size; ++row) {
            const cpp_int coefficient = row % 5 == 0 ? (cpp_int{1} << 80) + row : cpp_int{row + 1};
            slopes.push_back(from_integer(coefficient * divisor));
            values.push_back(from_integer(row * 3));
            destinations.push_back(row % 5 == 2 ? candidate::sink : row % 3);
        }
        for (const auto kernel : {candidate::Kernel::product, candidate::Kernel::delta}) {
            // Every short-call/tail length, q32/q80 mixtures, duplicate and sink
            // destinations, and exact same-row input/output aliasing.
            for (const auto factor : {std::int64_t{1}, std::int64_t{-37}, INT64_MIN, INT64_MAX}) {
                check_call(backend, kernel, slopes, factor, values, destinations, balances);
                check_call(backend, kernel, slopes, factor, values, destinations, balances, true);
            }
            auto offgrid = slopes;
            for (auto& value : offgrid) value = from_integer(integer(value) + 1);
            // All rejected lanes still succeed through exact factor cancellation.
            check_call(backend, kernel, offgrid, divisor, values, destinations, balances);
            for (unsigned failure = 0; failure < size; ++failure) {
                auto failing = slopes;
                failing[failure] = from_integer(1);
                if (failure + 1 < size) failing[failure + 1] = from_integer(1);
                check_call(backend, kernel, failing, 1, values, destinations, balances);
                check_call(backend, kernel, failing, 1, values, destinations, balances, true);
            }
        }
        for (unsigned failure = 0; failure < size; ++failure) {
            // An earlier commit failure outranks a later speculative map miss.
            std::vector<Money> failing(size, from_integer(divisor));
            auto overflow_values = values;
            overflow_values[failure] = Money::max();
            if (failure + 1 < size) failing[failure + 1] = from_integer(1);
            check_call(backend, candidate::Kernel::delta, failing, 1, overflow_values, destinations, balances);

            // A balance failure must not publish even the current row's value.
            std::vector<std::size_t> only_one(size, candidate::sink);
            only_one[failure] = 0;
            std::vector<Money> overflowing_balance{Money::max()};
            check_call(backend, candidate::Kernel::delta, failing, 1, values, only_one, overflowing_balance);
        }
        if (size >= 4) {
            std::vector<Money> same(size, from_integer(divisor));
            same[3] = from_integer(1);
            std::vector<std::size_t> duplicates(size, 0);
            std::vector<Money> nearly_full{from_integer((cpp_int{1} << 127) - 3)};
            // Two ordered commits reach max; the third fails before row 3's map.
            check_call(backend, candidate::Kernel::delta, same, 1, values, duplicates, nearly_full);
        }
        std::vector<std::size_t> unique(size);
        std::vector<Money> unique_balances(size, from_integer(-11));
        for (unsigned row = 0; row < size; ++row) unique[row] = row;
        check_call(backend, candidate::Kernel::delta, slopes, 1, values, unique, unique_balances);
    }
}
} // namespace
#endif

int main() {
#if defined(FIXEDWIDE_HAS_X86_64_ASM) && defined(FIXEDWIDE_HAS_MIXED_NATIVE) && !defined(_MSC_VER)
    const auto values = inputs();
    unsigned executed = 0;
    for (const auto& backend : backends()) {
        if (!backend.supported) {
            std::printf("SKIP %s: required CPU features or OS vector state unavailable\n", backend.name);
            continue;
        }
        const auto previous = checks;
        check_backend(backend, values);
        std::printf("PASS %s: %llu checks (native instructions executed)\n", backend.name,
                    static_cast<unsigned long long>(checks - previous));
        ++executed;
    }
    for (const auto backend : candidate::backends) {
        if (!candidate::supported(backend)) continue;
        const auto previous = checks;
        check_wrappers(backend);
        std::printf("PASS %s complete calls: %llu checks\n", candidate::backend_name(backend),
                    static_cast<unsigned long long>(checks - previous));
    }
    if (executed == 0) return 77;
    std::printf("vector coefficient oracle: %llu checks passed\n", static_cast<unsigned long long>(checks));
    return 0;
#else
    std::puts("SKIP AVX2/AVX-512/IFMA candidates: requires native x86-64 GCC or Clang; portable backend retained");
    return 77;
#endif
}
