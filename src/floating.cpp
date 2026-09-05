#include <fixedwide/floating.hpp>
#include "detail.hpp"
#include "limbs.hpp"
#include <algorithm>
#include <cmath>
#include <type_traits>
#include <limits>

namespace fixedwide::detail {

namespace {

template<typename Float>
std::expected<wide::int256, ArithmeticError> float_to_raw(Float value, unsigned decimals, Rounding rounding,
                                                          std::size_t bits) noexcept {
    if (std::isnan(value) || std::isinf(value)) {
        return std::unexpected(ArithmeticError::invalid_value);
    }
    if (value == Float(0.0)) {
        return wide::int256{};
    }

    bool negative = std::signbit(value);
    int exp = 0;
    // std::frexp and std::ldexp are overloaded for long double in C++, so the
    // C `frexpl`/`ldexpl` spellings are unnecessary -- and libstdc++ did not
    // put those two in namespace std until GCC 14, so naming them made the
    // library fail to compile on GCC 13 while claiming to support GCC.
    const Float m = std::abs(std::frexp(value, &exp));

    // Extract the full significand in chunks. A binary128 long double has
    // 113 bits, so silently limiting this to uint64_t loses real input bits.
    constexpr int sig_bits = std::numeric_limits<Float>::digits;
    u1024_limbs significand{};
    if constexpr (sig_bits <= 64) {
        significand = u1024_limbs(static_cast<std::uint64_t>(std::ldexp(m, sig_bits)));
    } else {
        Float remaining = m;
        for (int consumed = 0; consumed < sig_bits;) {
            const int count = std::min(64, sig_bits - consumed);
            remaining = std::ldexp(remaining, count);
            const auto chunk = static_cast<std::uint64_t>(remaining);
            significand = (significand << static_cast<unsigned>(count)) + u1024_limbs(chunk);
            remaining -= static_cast<Float>(chunk);
            consumed += count;
        }
    }
    const int pwr = exp - sig_bits;
    if (pwr > 300) return std::unexpected(ArithmeticError::overflow);

    u1024_limbs num = significand * pow10_limbs(decimals);
    u1024_limbs den(1ULL);

    if (pwr >= 0) {
        num = num << static_cast<unsigned>(pwr);
    } else {
        unsigned neg_pwr = static_cast<unsigned>(-pwr);
        if (neg_pwr >= 1024) {
            // The nonzero scaled magnitude is below half a raw unit:
            // at most 113 significand bits + 253 scale bits, divided by
            // at least 2^1024. Preserve an inexact, below-half remainder
            // so directed rounding and Rounding::exact still work.
            num = u1024_limbs(1ULL);
            den = u1024_limbs(4ULL);
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
    // float cannot represent 2^128: even a zero high limb used to evaluate
    // 0 * infinity and contaminate every result with NaN. Accumulate and
    // rescale in at least double, whose exponent range covers all 256 bits.
    using Work = std::conditional_t<std::is_same_v<Float, float>, double, Float>;
    Work d = static_cast<Work>(mag.limbs[0]) + static_cast<Work>(mag.limbs[1]) * std::ldexp(Work(1), 64) +
             static_cast<Work>(mag.limbs[2]) * std::ldexp(Work(1), 128) +
             static_cast<Work>(mag.limbs[3]) * std::ldexp(Work(1), 192);
    Work scale = std::pow(Work(10), static_cast<Work>(decimals));
    const Float res = static_cast<Float>(d / scale);
    return neg ? -res : res;
}

} // namespace

std::expected<wide::int256, ArithmeticError> from_float_kernel(float value, unsigned decimals, Rounding rounding,
                                                               std::size_t bits) noexcept {
    return float_to_raw(value, decimals, rounding, bits);
}

std::expected<wide::int256, ArithmeticError> from_float_kernel(double value, unsigned decimals, Rounding rounding,
                                                               std::size_t bits) noexcept {
    return float_to_raw(value, decimals, rounding, bits);
}

std::expected<wide::int256, ArithmeticError> from_float_kernel(long double value, unsigned decimals, Rounding rounding,
                                                               std::size_t bits) noexcept {
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
