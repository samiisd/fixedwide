#include <fixedwide/floating.hpp>
#include "detail.hpp"
#include "limbs.hpp"
#include <cmath>

namespace fixedwide::detail {

namespace {

template<typename Float>
std::expected<wide::int256, ArithmeticError>
float_to_raw(Float value, unsigned decimals, Rounding rounding, std::size_t bits) noexcept {
    if (std::isnan(value) || std::isinf(value)) {
        return std::unexpected(ArithmeticError::invalid_value);
    }
    if (value == Float(0.0)) {
        return wide::int256{};
    }

    bool negative = std::signbit(value);
    int exp = 0;
    Float m = 0;
    if constexpr (std::is_same_v<Float, long double>) {
        m = std::abs(std::frexpl(value, &exp));
    } else {
        m = std::abs(std::frexp(value, &exp));
    }

    constexpr int sig_bits = (sizeof(Float) == sizeof(float)) ? 24 : (sizeof(Float) == sizeof(double)) ? 53 : 64;
    Float scaled_m = 0;
    if constexpr (std::is_same_v<Float, long double>) {
        scaled_m = std::ldexpl(m, sig_bits);
    } else {
        scaled_m = std::ldexp(m, sig_bits);
    }

    std::uint64_t significand = static_cast<std::uint64_t>(scaled_m);
    int pwr = exp - sig_bits;

    if (pwr > 300) {
        return std::unexpected(ArithmeticError::overflow);
    }

    u1024_limbs num = u1024_limbs(significand) * pow10_limbs(decimals);
    u1024_limbs den(1ULL);

    if (pwr >= 0) {
        num = num << static_cast<unsigned>(pwr);
    } else {
        unsigned neg_pwr = static_cast<unsigned>(-pwr);
        if (neg_pwr >= 1024) {
            num = u1024_limbs(0ULL);
        } else {
            den = den << neg_pwr;
        }
    }

    auto divres = divmod_knuth(num, den);
    auto lim256 = detail::limit_magnitude_u256(bits, negative);
    u1024_limbs limit{};
    for (int i = 0; i < 4; ++i) limit.limbs[i] = lim256.limbs[i];

    if (divres.quotient > limit) {
        return std::unexpected(ArithmeticError::overflow);
    }

    auto rounded = round_magnitude(divres.quotient, divres.remainder, den, negative, rounding, limit);
    if (!rounded) return std::unexpected(rounded.error());

    wide::uint256 uq(rounded->limbs[0], rounded->limbs[1], rounded->limbs[2], rounded->limbs[3]);
    if (negative) {
        wide::uint256 neg_uq = ~uq + wide::uint256(1ULL);
        return wide::int256(neg_uq.limbs[0], neg_uq.limbs[1], neg_uq.limbs[2], neg_uq.limbs[3]);
    }
    return wide::int256(uq.limbs[0], uq.limbs[1], uq.limbs[2], uq.limbs[3]);
}

template<typename Float>
Float raw_to_float(wide::int256 raw, unsigned decimals) noexcept {
    bool neg = raw.is_negative();
    auto mag = magnitude(raw);
    Float d = static_cast<Float>(mag.limbs[0]) +
              static_cast<Float>(mag.limbs[1]) * std::ldexp(Float(1.0), 64) +
              static_cast<Float>(mag.limbs[2]) * std::ldexp(Float(1.0), 128) +
              static_cast<Float>(mag.limbs[3]) * std::ldexp(Float(1.0), 192);
    Float scale = std::pow(Float(10.0), static_cast<Float>(decimals));
    Float res = d / scale;
    return neg ? -res : res;
}

} // namespace

std::expected<wide::int256, ArithmeticError>
from_float_kernel(float value, unsigned decimals, Rounding rounding, std::size_t bits) noexcept {
    return float_to_raw(value, decimals, rounding, bits);
}

std::expected<wide::int256, ArithmeticError>
from_float_kernel(double value, unsigned decimals, Rounding rounding, std::size_t bits) noexcept {
    return float_to_raw(value, decimals, rounding, bits);
}

std::expected<wide::int256, ArithmeticError>
from_float_kernel(long double value, unsigned decimals, Rounding rounding, std::size_t bits) noexcept {
    return float_to_raw(value, decimals, rounding, bits);
}

float to_float_kernel(wide::int256 raw, unsigned decimals, float) noexcept {
    return raw_to_float<float>(raw, decimals);
}

double to_float_kernel(wide::int256 raw, unsigned decimals, double) noexcept {
    return raw_to_float<double>(raw, decimals);
}

long double to_float_kernel(wide::int256 raw, unsigned decimals, long double) noexcept {
    return raw_to_float<long double>(raw, decimals);
}

} // namespace fixedwide::detail
