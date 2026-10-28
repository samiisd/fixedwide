// Assembly/vectorization probes, intentionally without a main or benchmark
// dependency. Compile this translation unit with -O3 -DNDEBUG -S and the
// consumer's ISA flags. Neither restrict nor extra alignment is assumed.
#include <fixedwide/batch.hpp>
#include <cstddef>
#include <cstdint>
#include <span>

#if defined(__GNUC__) || defined(__clang__)
#define FIXEDWIDE_BATCH_PROBE_NOINLINE [[gnu::noinline]]
#elif defined(_MSC_VER)
#define FIXEDWIDE_BATCH_PROBE_NOINLINE __declspec(noinline)
#else
#define FIXEDWIDE_BATCH_PROBE_NOINLINE
#endif

namespace fixedwide_batch_codegen {
namespace fw = fixedwide;
using Money = fw::Fixed128<12>;
using Price = fw::Fixed64<10>;
using Small = fw::Fixed32<4>;
using Status = fw::batch::Result;
using ArithmeticStatus = std::expected<void, fw::ArithmeticError>;
constexpr auto exact = fw::Rounding::exact;

// All multiplication probes have the same accepted-prefix output contract.
// dx is a runtime broadcast, including its sign and full supported range.
FIXEDWIDE_BATCH_PROBE_NOINLINE
Status scalar_multiply(Money* output, const Money* slope, std::size_t count, Price dx) noexcept {
    for (std::size_t row = 0; row < count; ++row) {
        const auto product = fw::mul_to<Money>(slope[row], dx, exact);
        if (!product) return std::unexpected(fw::batch::Error{row, product.error()});
        output[row] = *product;
    }
    return {};
}

FIXEDWIDE_BATCH_PROBE_NOINLINE
Status prepared_scalar_multiply(Money* output, const Money* slope, std::size_t count, Price dx) noexcept {
    const auto multiply = fw::batch::prepare_mul_to<Money>(dx, exact);
    return fw::batch::transform<1>(std::span{output, count}, multiply, std::span{slope, count});
}

FIXEDWIDE_BATCH_PROBE_NOINLINE
Status blocked_multiply(Money* output, const Money* slope, std::size_t count, Price dx) noexcept {
    const auto multiply = fw::batch::prepare_mul_to<Money>(dx, exact);
    return fw::batch::transform<4>(std::span{output, count}, multiply, std::span{slope, count});
}

// Rounded FX probes exercise the unsigned64 x unsigned64 -> unsigned128
// shortcut and retain the full-width fallback when neither operand fits a
// magnitude limb. Factor setup remains outside the generated row loop.
FIXEDWIDE_BATCH_PROBE_NOINLINE
Status scalar_fx(Money* output, const Money* amount, std::size_t count, Money fx) noexcept {
    for (std::size_t row = 0; row < count; ++row) {
        const auto result = fw::mul_to<Money>(amount[row], fx, fw::Rounding::nearest_even);
        if (!result) return std::unexpected(fw::batch::Error{row, result.error()});
        output[row] = *result;
    }
    return {};
}

FIXEDWIDE_BATCH_PROBE_NOINLINE
Status prepared_scalar_fx(Money* output, const Money* amount, std::size_t count, Money fx) noexcept {
    const auto multiply = fw::batch::prepare_mul_to<Money>(fx, fw::Rounding::nearest_even);
    return fw::batch::transform<1>(std::span{output, count}, multiply, std::span{amount, count});
}

FIXEDWIDE_BATCH_PROBE_NOINLINE
Status blocked_fx(Money* output, const Money* amount, std::size_t count, Money fx) noexcept {
    const auto multiply = fw::batch::prepare_mul_to<Money>(fx, fw::Rounding::nearest_even);
    return fw::batch::transform<4>(std::span{output, count}, multiply, std::span{amount, count});
}

FIXEDWIDE_BATCH_PROBE_NOINLINE
Status varying_fx(Money* output, const Money* amount, const Money* rate, std::size_t count) noexcept {
    return fw::batch::transform<1>(std::span{output, count}, fw::batch::mul_to<Money>(fw::Rounding::nearest_even),
                                   std::span{amount, count}, std::span{rate, count});
}

// Each balance index must address the total array. Indices may repeat in any
// order. Slope/index fields must not alias the mutable value/total fields, and
// value and total are distinct fields. No uniqueness or iota index assumption.
// The two additions are checked before either output of the row is written.
FIXEDWIDE_BATCH_PROBE_NOINLINE
Status scalar_delta(const Money* slope, Money* value, Money* total, const std::uint32_t* balance, std::size_t count,
                    Price dx) noexcept {
    for (std::size_t row = 0; row < count; ++row) {
        const auto contribution = fw::mul_to<Money>(slope[row], dx, exact);
        if (!contribution) return std::unexpected(fw::batch::Error{row, contribution.error()});
        const auto after = fw::add(value[row], *contribution);
        if (!after) return std::unexpected(fw::batch::Error{row, after.error()});
        const auto destination = balance[row];
        const auto sum = fw::add(total[destination], *contribution);
        if (!sum) return std::unexpected(fw::batch::Error{row, sum.error()});
        value[row] = *after;
        total[destination] = *sum;
    }
    return {};
}

template<std::size_t BlockSize>
Status prepared_delta(const Money* slope, Money* value, Money* total, const std::uint32_t* balance, std::size_t count,
                      Price dx) noexcept {
    const auto multiply = fw::batch::prepare_mul_to<Money>(dx, exact);
    const auto commit = [&](std::size_t row, Money contribution) noexcept -> ArithmeticStatus {
        const auto after = fw::add(value[row], contribution);
        if (!after) return std::unexpected(after.error());
        const auto destination = balance[row];
        const auto sum = fw::add(total[destination], contribution);
        if (!sum) return std::unexpected(sum.error());
        value[row] = *after;
        total[destination] = *sum;
        return {};
    };
    return fw::batch::for_each<BlockSize>(count, multiply, commit, std::span{slope, count});
}

FIXEDWIDE_BATCH_PROBE_NOINLINE
Status prepared_scalar_delta(const Money* slope, Money* value, Money* total, const std::uint32_t* balance,
                             std::size_t count, Price dx) noexcept {
    return prepared_delta<1>(slope, value, total, balance, count, dx);
}

FIXEDWIDE_BATCH_PROBE_NOINLINE
Status blocked_delta(const Money* slope, Money* value, Money* total, const std::uint32_t* balance, std::size_t count,
                     Price dx) noexcept {
    return prepared_delta<4>(slope, value, total, balance, count, dx);
}

// These comparison/select probes help distinguish SIMD arithmetic from XMM
// register copies of a 128-bit value. Any vectorized Small comparison says
// nothing about whether the multiply or wide carry chains vectorized.
FIXEDWIDE_BATCH_PROBE_NOINLINE
Status select_min32(Small* output, const Small* left, const Small* right, std::size_t count) noexcept {
    return fw::batch::transform<4>(
        std::span{output, count}, [](Small a, Small b) noexcept { return a < b ? a : b; }, std::span{left, count},
        std::span{right, count});
}

FIXEDWIDE_BATCH_PROBE_NOINLINE
Status select_min128(Money* output, const Money* left, const Money* right, std::size_t count) noexcept {
    return fw::batch::transform<4>(
        std::span{output, count}, [](Money a, Money b) noexcept { return a < b ? a : b; }, std::span{left, count},
        std::span{right, count});
}
} // namespace fixedwide_batch_codegen

#undef FIXEDWIDE_BATCH_PROBE_NOINLINE
