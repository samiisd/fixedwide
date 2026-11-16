// Complete kernels for the opt-in fallback instruction comparison.
// Compile this translation unit separately from the driver, without LTO.
#include <fixedwide/batch.hpp>
#include <cstdint>
#include <span>
namespace fixedwide_fallback_codegen {
namespace fw = fixedwide;
using Money = fw::Fixed128<12>;
using Price = fw::Fixed64<10>;
using Wide = fw::Fixed256<12>;
using Status = fw::batch::Result;
using ArithmeticStatus = std::expected<void, fw::ArithmeticError>;
constexpr auto nearest = fw::Rounding::nearest_even;
constexpr auto exact = fw::Rounding::exact;
Status failure(std::size_t row, fw::ArithmeticError error) noexcept {
    return std::unexpected(fw::batch::Error{row, error});
}

template<bool Prepared>
Status anchored(const Money* slopes, const Money* residuals, const Price* anchors, const std::uint32_t* indices,
                Money* previous, Money* totals, std::size_t count, Price mark, Money fx) noexcept {
    const auto product = fw::batch::mul_to<Money>(nearest);
    const auto convert = fw::batch::prepare_mul_to<Money>(fx, nearest);
    const auto map = [&](Money slope, Money residual,
                         Price anchor) noexcept -> std::expected<Money, fw::ArithmeticError> {
        // The retained fixture proves subtraction fits its signed price raw type.
        const auto dx = Price::from_raw(mark.raw() - anchor.raw());
        const auto first = [&] {
            if constexpr (Prepared)
                return product(slope, dx);
            else
                return fw::mul_to<Money>(slope, dx, nearest);
        }();
        if (!first) return std::unexpected(first.error());
        const auto subtotal = fw::add(*first, residual);
        if (!subtotal) return std::unexpected(subtotal.error());
        if constexpr (Prepared)
            return convert(*subtotal);
        else
            return fw::mul_to<Money>(*subtotal, fx, nearest);
    };
    const auto commit = [&](std::size_t row, Money next) noexcept -> ArithmeticStatus {
        const auto index = indices[row];
        const auto removed = fw::sub_to<Wide>(totals[index], previous[row], exact);
        if (!removed) return std::unexpected(removed.error());
        const auto replaced = fw::add_to<Wide>(*removed, next, exact);
        if (!replaced) return std::unexpected(replaced.error());
        const auto narrowed = fw::fixed_cast<Money>(*replaced, exact);
        if (!narrowed) return std::unexpected(narrowed.error());
        previous[row] = next;
        totals[index] = *narrowed;
        return {};
    };
    if constexpr (Prepared)
        return fw::batch::for_each<1>(count, map, commit, std::span{slopes, count}, std::span{residuals, count},
                                      std::span{anchors, count});
    else {
        for (std::size_t row = 0; row < count; ++row) {
            const auto mapped = map(slopes[row], residuals[row], anchors[row]);
            if (!mapped) return failure(row, mapped.error());
            const auto applied = commit(row, *mapped);
            if (!applied) return failure(row, applied.error());
        }
        return {};
    }
}

template<bool Prepared>
Status publication(const Money* slopes, const Price* anchors, Money* output, std::size_t count, Price mark,
                   Money fx) noexcept {
    const auto product = fw::batch::mul_to<Money>(nearest);
    const auto convert = fw::batch::prepare_mul_to<Money>(fx, nearest);
    // Four entry products are individually rounded, then added in their original
    // order. FX is rounded once per position after its entry sum.
    for (std::size_t position = 0; position < count / 4; ++position) {
        Money sum{};
        for (std::size_t lane = 0; lane < 4; ++lane) {
            const auto row = 4 * position + lane;
            const auto dx = Price::from_raw(mark.raw() - anchors[row].raw());
            const auto entry = [&] {
                if constexpr (Prepared)
                    return product(slopes[row], dx);
                else
                    return fw::mul_to<Money>(slopes[row], dx, nearest);
            }();
            if (!entry) return failure(row, entry.error());
            const auto added = fw::add(sum, *entry);
            if (!added) return failure(row, added.error());
            sum = *added;
        }
        const auto value = [&] {
            if constexpr (Prepared)
                return convert(sum);
            else
                return fw::mul_to<Money>(sum, fx, nearest);
        }();
        if (!value) return failure(4 * position + 3, value.error());
        output[position] = *value;
    }
    return {};
}
[[gnu::noinline]] Status public_anchored(const Money* a, const Money* b, const Price* c, const std::uint32_t* d,
                                         Money* e, Money* f, std::size_t n, Price m, Money x) noexcept {
    return anchored<false>(a, b, c, d, e, f, n, m, x);
}
[[gnu::noinline]] Status prepared_anchored(const Money* a, const Money* b, const Price* c, const std::uint32_t* d,
                                           Money* e, Money* f, std::size_t n, Price m, Money x) noexcept {
    return anchored<true>(a, b, c, d, e, f, n, m, x);
}
[[gnu::noinline]] Status public_publication(const Money* a, const Price* b, Money* c, std::size_t n, Price m,
                                            Money x) noexcept {
    return publication<false>(a, b, c, n, m, x);
}
[[gnu::noinline]] Status prepared_publication(const Money* a, const Price* b, Money* c, std::size_t n, Price m,
                                              Money x) noexcept {
    return publication<true>(a, b, c, n, m, x);
}

// A rounded price product whose raw numerator may exceed signed128.
template<bool Prepared>
Status rounded_price(Money* output, const Money* slopes, std::size_t count, Price dx) noexcept {
    const auto multiply = fw::batch::prepare_mul_to<Money>(dx, nearest);
    for (std::size_t row = 0; row < count; ++row) {
        const auto result = [&] {
            if constexpr (Prepared)
                return multiply(slopes[row]);
            else
                return fw::mul_to<Money>(slopes[row], dx, nearest);
        }();
        if (!result) return failure(row, result.error());
        output[row] = *result;
    }
    return {};
}
[[gnu::noinline]] Status public_rounded_price(Money* a, const Money* b, std::size_t n, Price x) noexcept {
    return rounded_price<false>(a, b, n, x);
}
[[gnu::noinline]] Status prepared_rounded_price(Money* a, const Money* b, std::size_t n, Price x) noexcept {
    return rounded_price<true>(a, b, n, x);
}
} // namespace fixedwide_fallback_codegen
