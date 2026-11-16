#pragma once

// Reusable benchmark/test kernels. This header is not part of the installed
// library and none of its experimental backends participates in dispatch.
#include "zen5_simd.hpp"
#include "zen5_ifma.hpp"
#include <array>
#include <bit>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <span>
#include <string_view>
#if defined(FIXEDWIDE_HAS_X86_64_ASM) && defined(FIXEDWIDE_HAS_MIXED_NATIVE) && !defined(_MSC_VER)
#include <cpuid.h>
#endif

namespace fixedwide_zen5 {

enum class Backend {
    public_scalar,
    prepared_scalar,
    avx2,
    avx512vl64,
    avx51264,
    avx512vl94,
    avx51294,
    ifma256,
    ifma512
};
enum class Kernel { product, delta };
inline constexpr std::size_t sink = std::numeric_limits<std::size_t>::max();
inline constexpr std::array backends{Backend::public_scalar, Backend::prepared_scalar, Backend::avx2,
                                     Backend::avx512vl64,    Backend::avx51264,        Backend::avx512vl94,
                                     Backend::avx51294,      Backend::ifma256,         Backend::ifma512};

constexpr const char* backend_name(Backend backend) noexcept {
    switch (backend) {
    case Backend::public_scalar: return "public_scalar";
    case Backend::prepared_scalar: return "prepared_scalar";
    case Backend::avx2: return "avx2";
    case Backend::avx512vl64: return "avx512vl64";
    case Backend::avx51264: return "avx51264";
    case Backend::avx512vl94: return "avx512vl94";
    case Backend::avx51294: return "avx51294";
    case Backend::ifma256: return "ifma256";
    case Backend::ifma512: return "ifma512";
    }
    return "unknown";
}

constexpr unsigned block_lanes(Backend backend) noexcept {
    switch (backend) {
    case Backend::public_scalar:
    case Backend::prepared_scalar: return 1;
    case Backend::avx2:
    case Backend::avx512vl64:
    case Backend::avx512vl94:
    case Backend::ifma256: return 4;
    default: return 8;
    }
}

struct Hardware {
    std::uint64_t xcr0 = 0;
    bool popcnt = false;
    bool avx2 = false, avx512f = false, avx512dq = false, avx512vl = false, avx512ifma = false;
};

inline const Hardware& hardware() noexcept {
    static const Hardware result = [] {
        Hardware value;
#if defined(FIXEDWIDE_HAS_X86_64_ASM) && defined(FIXEDWIDE_HAS_MIXED_NATIVE) && !defined(_MSC_VER)
        __builtin_cpu_init();
        value.popcnt = __builtin_cpu_supports("popcnt");
        unsigned a, b, c, d;
        constexpr unsigned xsave_avx = (1u << 26) | (1u << 27) | (1u << 28);
        if (__get_cpuid(1, &a, &b, &c, &d) && (c & xsave_avx) == xsave_avx) {
            unsigned low, high;
            __asm__("xgetbv" : "=a"(low), "=d"(high) : "c"(0));
            value.xcr0 = (std::uint64_t{high} << 32) | low;
        }
        value.avx2 = (value.xcr0 & 6) == 6 && __builtin_cpu_supports("avx2");
        const bool zmm = (value.xcr0 & 0xe6) == 0xe6;
        value.avx512f = zmm && __builtin_cpu_supports("avx512f");
        value.avx512dq = zmm && __builtin_cpu_supports("avx512dq");
        value.avx512vl = zmm && __builtin_cpu_supports("avx512vl");
        value.avx512ifma = zmm && __builtin_cpu_supports("avx512ifma");
#endif
        return value;
    }();
    return result;
}

constexpr bool supported(Backend backend, const Hardware& cpu) noexcept {
    // Targeted diagnostic passes can lower std::popcount to POPCNT. Its CPUID
    // bit is independent of AVX2 and AVX-512, including on virtualized hosts.
    switch (backend) {
    case Backend::public_scalar:
    case Backend::prepared_scalar: return true;
    case Backend::avx2: return cpu.popcnt && cpu.avx2;
    case Backend::avx512vl64:
    case Backend::avx512vl94: return cpu.popcnt && cpu.avx2 && cpu.avx512f && cpu.avx512dq && cpu.avx512vl;
    case Backend::avx51264:
    case Backend::avx51294: return cpu.popcnt && cpu.avx2 && cpu.avx512f && cpu.avx512dq;
    case Backend::ifma256: return cpu.popcnt && cpu.avx2 && cpu.avx512f && cpu.avx512ifma && cpu.avx512vl;
    case Backend::ifma512: return cpu.popcnt && cpu.avx2 && cpu.avx512f && cpu.avx512ifma;
    }
    return false;
}

inline bool supported(Backend backend) noexcept {
    return supported(backend, hardware());
}

// These counts come from a separate instrumented execution, not estimated
// coefficient eligibility and not counters inserted into the timed loop.
struct Diagnostics {
    std::size_t tile_calls = 0, simd_evaluated = 0, simd_accepted = 0;
    std::size_t scalar_maps = 0, fallback_maps = 0, tail_maps = 0;
    std::size_t commit_calls = 0, direct_store_rows = 0, completed_rows = 0, simd_commits = 0;
};

namespace kernel_detail {
using ArithmeticStatus = std::expected<void, fixedwide::ArithmeticError>;
using Slopes = std::span<const Money>;
using Values = std::span<Money>;
using Destinations = std::span<const std::size_t>;

inline Status failure(std::size_t row, fixedwide::ArithmeticError error) noexcept {
    return std::unexpected(fixedwide::batch::Error{row, error});
}

template<Kernel Operation, bool Diagnostic>
struct Commit {
    Values values;
    Destinations destinations;
    Values balances;
    Diagnostics* diagnostics;

    [[gnu::always_inline]] inline ArithmeticStatus operator()(std::size_t row, const Money& value,
                                                              bool simd = false) const noexcept {
        if constexpr (Diagnostic) ++diagnostics->commit_calls;
        if constexpr (Operation == Kernel::product) {
            values[row] = value;
        } else {
            const auto next_value = fixedwide::add(values[row], value);
            if (!next_value) return std::unexpected(next_value.error());
            const auto destination = destinations[row];
            if (destination != sink) {
                const auto next_balance = fixedwide::add(balances[destination], value);
                if (!next_balance) return std::unexpected(next_balance.error());
                balances[destination] = *next_balance;
            }
            values[row] = *next_value;
        }
        if constexpr (Diagnostic) {
            ++diagnostics->completed_rows;
            diagnostics->simd_commits += simd;
        }
        return {};
    }
};

template<Kernel Operation, bool Diagnostic, class Map>
[[gnu::always_inline]] inline Status tail(std::size_t row, Slopes slopes, Map& map,
                                          Commit<Operation, Diagnostic>& commit, Diagnostics* diagnostics,
                                          bool vector_backend) noexcept {
    for (; row < slopes.size(); ++row) {
        if constexpr (Diagnostic) {
            ++diagnostics->scalar_maps;
            diagnostics->tail_maps += vector_backend;
        }
        const auto mapped = map(slopes[row]);
        if (!mapped) return failure(row, mapped.error());
        const auto status = commit(row, *mapped);
        if (!status) return failure(row, status.error());
    }
    return {};
}

template<Kernel Operation, bool Diagnostic, class Map, std::size_t Width>
[[gnu::always_inline]] inline Status consume(std::size_t row, Slopes slopes, const std::array<Money, Width>& results,
                                             unsigned accepted, Map& map, Commit<Operation, Diagnostic>& commit,
                                             Diagnostics* diagnostics) noexcept {
    if constexpr (Diagnostic) {
        ++diagnostics->tile_calls;
        diagnostics->simd_evaluated += Width;
        diagnostics->simd_accepted += std::popcount(accepted);
    }
    if constexpr (Operation == Kernel::product) {
        if (accepted == (1u << Width) - 1) {
            std::memcpy(static_cast<void*>(commit.values.data() + row), results.data(), sizeof(results));
            if constexpr (Diagnostic) {
                diagnostics->direct_store_rows += Width;
                diagnostics->completed_rows += Width;
                diagnostics->simd_commits += Width;
            }
            return {};
        }
    }
    for (std::size_t lane = 0; lane < Width; ++lane) {
        Money value = results[lane];
        const bool simd = (accepted & (1u << lane)) != 0;
        if (!simd) {
            if constexpr (Diagnostic) {
                ++diagnostics->scalar_maps;
                ++diagnostics->fallback_maps;
            }
            const auto mapped = map(slopes[row + lane]);
            if (!mapped) return failure(row + lane, mapped.error());
            value = *mapped;
        }
        const auto status = commit(row + lane, value, simd);
        if (!status) return failure(row + lane, status.error());
    }
    return {};
}

template<Kernel Operation, bool Diagnostic, bool Prepared>
inline Status scalar(Slopes slopes, std::int64_t factor, Values values, Destinations destinations, Values balances,
                     Diagnostics* diagnostics) noexcept {
    Commit<Operation, Diagnostic> commit{values, destinations, balances, diagnostics};
    const Price price = Price::from_raw(factor);
    if constexpr (Prepared) {
        auto map = fixedwide::batch::prepare_mul_to<Money>(price, fixedwide::Rounding::exact);
        return tail(0, slopes, map, commit, diagnostics, false);
    } else {
        auto map = [price](Money value) noexcept {
            return fixedwide::mul_to<Money>(value, price, fixedwide::Rounding::exact);
        };
        return tail(0, slopes, map, commit, diagnostics, false);
    }
}

#if defined(FIXEDWIDE_HAS_X86_64_ASM) && defined(FIXEDWIDE_HAS_MIXED_NATIVE) && !defined(_MSC_VER)
namespace avx2_impl = fixedwide::detail::batch::simd;

template<Kernel Operation, bool Diagnostic, avx2_impl::Kind Method>
[[gnu::target("avx2")]] inline Status avx2_kind(Slopes slopes, std::int64_t factor, Values values,
                                                Destinations destinations, Values balances,
                                                Diagnostics* diagnostics) noexcept {
    auto map = fixedwide::batch::prepare_mul_to<Money>(Price::from_raw(factor), fixedwide::Rounding::exact);
    Commit<Operation, Diagnostic> commit{values, destinations, balances, diagnostics};
    const avx2_impl::Factor prepared{factor};
    if constexpr (!Diagnostic) {
        // The timed backend is the committed AVX2 implementation itself.
        if constexpr (Operation == Kernel::product)
            return avx2_impl::transform<10, Method, fixedwide::batch::Error>(map, values, slopes, prepared);
        else
            return avx2_impl::for_each<10, Method, fixedwide::batch::Error, Money>(map, commit, slopes, prepared);
    } else {
        // The diagnostic uses the identical tile evaluator and fallback map,
        // with counters and ordered commits. It is verified against timing code.
        std::size_t row = 0;
        for (; slopes.size() - row >= 4; row += 4) {
            const auto result = avx2_impl::evaluate<10, Method>(slopes.data() + row, prepared);
            std::array<Money, 4> tile;
            avx2_impl::store(tile.data(), result.values);
            const auto status = consume(row, slopes, tile, result.accepted, map, commit, diagnostics);
            if (!status) return status;
        }
        return tail(row, slopes, map, commit, diagnostics, true);
    }
}

template<Kernel Operation, bool Diagnostic>
inline Status avx2(Slopes slopes, std::int64_t factor, Values values, Destinations destinations, Values balances,
                   Diagnostics* diagnostics) noexcept {
    const avx2_impl::Factor prepared{factor};
    if (prepared.signed32())
        return avx2_kind<Operation, Diagnostic, avx2_impl::Kind::signed32>(slopes, factor, values, destinations,
                                                                           balances, diagnostics);
    if (prepared.magnitude <= UINT32_MAX)
        return avx2_kind<Operation, Diagnostic, avx2_impl::Kind::unsigned32>(slopes, factor, values, destinations,
                                                                             balances, diagnostics);
    return avx2_kind<Operation, Diagnostic, avx2_impl::Kind::unsigned64>(slopes, factor, values, destinations, balances,
                                                                         diagnostics);
}

// One target boundary per complete call. Keeping each entire loop under its
// ISA target lets the tile inline without an ABI crossing on every block.
#define FIXEDWIDE_ZEN5_KERNEL(Name, Target, Width, Evaluate)                                                           \
    template<Kernel Operation, bool Diagnostic>                                                                        \
    [[gnu::target(Target)]] inline Status Name(Slopes slopes, std::int64_t factor, Values values,                      \
                                               Destinations destinations, Values balances,                             \
                                               Diagnostics* diagnostics) noexcept {                                    \
        auto map = fixedwide::batch::prepare_mul_to<Money>(Price::from_raw(factor), fixedwide::Rounding::exact);       \
        Commit<Operation, Diagnostic> commit{values, destinations, balances, diagnostics};                             \
        std::size_t row = 0;                                                                                           \
        for (; slopes.size() - row >= Width; row += Width) {                                                           \
            std::array<Money, Width> tile;                                                                             \
            const unsigned accepted = Evaluate;                                                                        \
            const auto status = consume(row, slopes, tile, accepted, map, commit, diagnostics);                        \
            if (!status) return status;                                                                                \
        }                                                                                                              \
        return tail(row, slopes, map, commit, diagnostics, true);                                                      \
    }

FIXEDWIDE_ZEN5_KERNEL(avx512vl64, "avx512f,avx512dq,avx512vl", 4,
                      evex256::evaluate<false>(slopes.data() + row, factor, tile.data()))
FIXEDWIDE_ZEN5_KERNEL(avx51264, "avx512f,avx512dq", 8,
                      evex512::evaluate<false>(slopes.data() + row, factor, tile.data()))
FIXEDWIDE_ZEN5_KERNEL(avx512vl94, "avx512f,avx512dq,avx512vl", 4,
                      evex256::evaluate<true>(slopes.data() + row, factor, tile.data()))
FIXEDWIDE_ZEN5_KERNEL(avx51294, "avx512f,avx512dq", 8,
                      evex512::evaluate<true>(slopes.data() + row, factor, tile.data()))
FIXEDWIDE_ZEN5_KERNEL(ifma256, "avx512f,avx512ifma,avx512vl", 4, ifma256_tile(slopes.data() + row, factor, tile.data()))
FIXEDWIDE_ZEN5_KERNEL(ifma512, "avx512f,avx512ifma", 8, ifma512_tile(slopes.data() + row, factor, tile.data()))
#undef FIXEDWIDE_ZEN5_KERNEL
#endif

template<Kernel Operation, bool Diagnostic>
inline Status dispatch(Backend backend, Slopes slopes, std::int64_t factor, Values values, Destinations destinations,
                       Values balances, Diagnostics* diagnostics) noexcept {
    switch (backend) {
    case Backend::public_scalar:
        return scalar<Operation, Diagnostic, false>(slopes, factor, values, destinations, balances, diagnostics);
    case Backend::prepared_scalar:
        return scalar<Operation, Diagnostic, true>(slopes, factor, values, destinations, balances, diagnostics);
#if defined(FIXEDWIDE_HAS_X86_64_ASM) && defined(FIXEDWIDE_HAS_MIXED_NATIVE) && !defined(_MSC_VER)
#define FIXEDWIDE_ZEN5_CASE(Name)                                                                                      \
    case Backend::Name: return Name<Operation, Diagnostic>(slopes, factor, values, destinations, balances, diagnostics);
        FIXEDWIDE_ZEN5_CASE(avx2)
        FIXEDWIDE_ZEN5_CASE(avx512vl64)
        FIXEDWIDE_ZEN5_CASE(avx51264)
        FIXEDWIDE_ZEN5_CASE(avx512vl94)
        FIXEDWIDE_ZEN5_CASE(avx51294)
        FIXEDWIDE_ZEN5_CASE(ifma256)
        FIXEDWIDE_ZEN5_CASE(ifma512)
#undef FIXEDWIDE_ZEN5_CASE
#else
    default: break;
#endif
    }
    std::abort(); // Unsupported forcing must never be labelled as another ISA.
}
} // namespace kernel_detail

inline Status execute(Backend backend, Kernel operation, std::span<const Money> slopes, std::int64_t factor,
                      std::span<Money> values, std::span<const std::size_t> destinations = {},
                      std::span<Money> balances = {}, Diagnostics* diagnostics = nullptr) noexcept {
    if (!supported(backend)) std::abort();
    assert(values.size() == slopes.size());
    assert(operation == Kernel::product || destinations.size() == slopes.size());
    if (diagnostics) {
        *diagnostics = {};
        if (operation == Kernel::product)
            return kernel_detail::dispatch<Kernel::product, true>(backend, slopes, factor, values, destinations,
                                                                  balances, diagnostics);
        return kernel_detail::dispatch<Kernel::delta, true>(backend, slopes, factor, values, destinations, balances,
                                                            diagnostics);
    }
    if (operation == Kernel::product)
        return kernel_detail::dispatch<Kernel::product, false>(backend, slopes, factor, values, destinations, balances,
                                                               nullptr);
    return kernel_detail::dispatch<Kernel::delta, false>(backend, slopes, factor, values, destinations, balances,
                                                         nullptr);
}
} // namespace fixedwide_zen5
