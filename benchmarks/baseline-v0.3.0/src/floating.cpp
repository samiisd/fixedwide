#include <fixedwide/floating.hpp>
#include "detail.hpp"
#include <bit>
#include <limits>

namespace fixedwide {
inline namespace FIXEDWIDE_SCALE_NAMESPACE {
namespace {
std::expected<i128, ArithmeticError> from_binary64(double value, Rounding rounding, u128 positive_limit) noexcept {
    static_assert(sizeof(double) == 8 && std::numeric_limits<double>::is_iec559);
    const auto bits = std::bit_cast<std::uint64_t>(value);
    const auto exponent_bits = static_cast<unsigned>((bits >> 52) & 0x7ff);
    if (exponent_bits == 0x7ff) return std::unexpected(ArithmeticError::invalid_value);
    const bool negative = (bits >> 63) != 0;
    const std::uint64_t significand = (bits & ((std::uint64_t{1} << 52) - 1)) |
                                    (exponent_bits == 0 ? 0 : (std::uint64_t{1} << 52));
    if (significand == 0) return i128{0};
    // 53 significand bits + at most 40 scale bits: always fits unsigned 128.
    u128 scaled = u128{significand} * scale;
    const int exponent = exponent_bits == 0 ? -1074 : static_cast<int>(exponent_bits) - 1023 - 52;
    const u128 limit = positive_limit + static_cast<unsigned>(negative);
    if (exponent >= 0) {
        if (exponent >= 128 || scaled > (limit >> exponent)) return std::unexpected(ArithmeticError::overflow);
        return detail::apply_sign(scaled << exponent, negative);
    }
    const int shift = -exponent;
    if (shift >= 128) {
        // The scaled significand has <=93 bits, so this is strictly below half.
        return detail::finish({0, 1}, 4, negative, rounding, positive_limit);
    }
    const u128 denominator = u128{1} << shift;
    return detail::finish({scaled >> shift, scaled & (denominator - 1)}, denominator, negative, rounding, positive_limit);
}
}
std::expected<FP64, ArithmeticError> from_double64(double value, Rounding rounding) noexcept {
    const auto result = from_binary64(value, rounding, INT64_MAX);
    if (!result) return std::unexpected(result.error());
    return FP64::from_raw(static_cast<std::int64_t>(*result));
}
std::expected<FP128, ArithmeticError> from_double128(double value, Rounding rounding) noexcept {
    const auto result = from_binary64(value, rounding, static_cast<u128>(i128_max));
    if (!result) return std::unexpected(result.error());
    return FP128::from_raw(*result);
}
double to_double(FP64 value) noexcept { return static_cast<double>(value.raw()) / static_cast<double>(scale); }
double to_double(FP128 value) noexcept { return static_cast<double>(value.raw()) / static_cast<double>(scale); }
} // inline namespace
} // namespace fixedwide
