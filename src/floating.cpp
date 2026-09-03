#include <fixedwide/floating.hpp>
#include "detail.hpp"
#include <cmath>

namespace fixedwide::detail {

std::expected<wide::int256, ArithmeticError>
from_float_kernel(double value, unsigned decimals, Rounding rounding, std::size_t bits) noexcept {
    if (std::isnan(value) || std::isinf(value)) return std::unexpected(ArithmeticError::invalid_value);
    double scale = std::pow(10.0, static_cast<double>(decimals));
    double scaled = value * scale;

    double rounded_val = 0.0;
    switch (rounding) {
    case Rounding::toward_zero:
        rounded_val = std::trunc(scaled);
        break;
    case Rounding::floor:
        rounded_val = std::floor(scaled);
        break;
    case Rounding::ceil:
        rounded_val = std::ceil(scaled);
        break;
    case Rounding::nearest_away:
        rounded_val = std::round(scaled);
        break;
    case Rounding::nearest_even: {
        double t = std::trunc(scaled);
        double diff = std::abs(scaled - t);
        if (diff == 0.5) {
            double half = t / 2.0;
            rounded_val = (half == std::trunc(half)) ? t : (scaled > 0 ? t + 1.0 : t - 1.0);
        } else {
            rounded_val = std::round(scaled);
        }
        break;
    }
    case Rounding::exact:
        if (scaled != std::trunc(scaled)) return std::unexpected(ArithmeticError::inexact);
        rounded_val = scaled;
        break;
    }

    double max_lim, min_lim;
    if (bits == 8)   { max_lim = INT8_MAX; min_lim = INT8_MIN; }
    else if (bits == 16)  { max_lim = INT16_MAX; min_lim = INT16_MIN; }
    else if (bits == 32)  { max_lim = INT32_MAX; min_lim = INT32_MIN; }
    else if (bits == 64)  { max_lim = static_cast<double>(INT64_MAX); min_lim = static_cast<double>(INT64_MIN); }
    else if (bits == 128) { max_lim = 1.7e38; min_lim = -1.7e38; }
    else { max_lim = 1e76; min_lim = -1e76; }

    if (rounded_val > max_lim || rounded_val < min_lim) {
        return std::unexpected(ArithmeticError::overflow);
    }

    if (bits <= 64) {
        return wide::int256(static_cast<std::int64_t>(rounded_val));
    }

    // Wide conversion
    bool neg = rounded_val < 0;
    double m = std::abs(rounded_val);
    std::uint64_t l0 = static_cast<std::uint64_t>(std::fmod(m, 18446744073709551616.0));
    m = std::floor(m / 18446744073709551616.0);
    std::uint64_t l1 = static_cast<std::uint64_t>(std::fmod(m, 18446744073709551616.0));
    m = std::floor(m / 18446744073709551616.0);
    std::uint64_t l2 = static_cast<std::uint64_t>(std::fmod(m, 18446744073709551616.0));
    m = std::floor(m / 18446744073709551616.0);
    std::uint64_t l3 = static_cast<std::uint64_t>(std::fmod(m, 18446744073709551616.0));

    wide::int256 res(l0, l1, l2, l3);
    if (neg) return -res;
    return res;
}

double to_float_kernel(wide::int256 raw, unsigned decimals) noexcept {
    bool neg = raw.is_negative();
    auto mag = magnitude(raw);
    double d = static_cast<double>(mag.limbs[0]) +
               static_cast<double>(mag.limbs[1]) * 18446744073709551616.0 +
               static_cast<double>(mag.limbs[2]) * 18446744073709551616.0 * 18446744073709551616.0 +
               static_cast<double>(mag.limbs[3]) * 18446744073709551616.0 * 18446744073709551616.0 * 18446744073709551616.0;
    double scale = std::pow(10.0, static_cast<double>(decimals));
    double res = d / scale;
    return neg ? -res : res;
}

} // namespace fixedwide::detail
