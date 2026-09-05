#pragma once

#include "check.hpp"
#include <fixedwide/arithmetic.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <cstdint>
#include <expected>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

// Independent signed-magnitude reference values. Do not use fixedwide arithmetic
// to compute expected results: the production and constexpr paths must both
// agree with Boost's arbitrary-precision integer operations.
namespace coverage_test {
using boost::multiprecision::cpp_int;
using fixedwide::ArithmeticError;
using fixedwide::Rounding;
inline constexpr std::array modes{Rounding::toward_zero,  Rounding::floor,        Rounding::ceil,
                                  Rounding::nearest_even, Rounding::nearest_away, Rounding::exact};
inline cpp_int power10(unsigned n) {
    cpp_int result = 1;
    while (n-- != 0) result *= 10;
    return result;
}
inline cpp_int bits(fixedwide::wide::uint128 x) {
    return (cpp_int{x.high} << 64) + x.low;
}
inline cpp_int bits(fixedwide::wide::int128 x) {
    return (cpp_int{x.high} << 64) + x.low;
}
inline cpp_int bits(fixedwide::wide::uint256 x) {
    cpp_int result = 0;
    for (unsigned i = 4; i-- != 0;) result = (result << 64) + x.limbs[i];
    return result;
}
inline cpp_int bits(fixedwide::wide::int256 x) {
    return bits(fixedwide::wide::uint256{x.limbs[0], x.limbs[1], x.limbs[2], x.limbs[3]});
}
template<class W>
cpp_int integer(W x) {
    if constexpr (std::is_integral_v<W>)
        return cpp_int{x};
    else {
        cpp_int result = bits(x);
        if constexpr (requires { x.is_negative(); }) {
            if (x.is_negative()) result -= cpp_int{1} << (sizeof(W) * 8);
        }
        return result;
    }
}
template<class W>
W from_integer(cpp_int value) {
    if constexpr (std::is_integral_v<W>)
        return value.convert_to<W>();
    else {
        const cpp_int mask = (cpp_int{1} << 64) - 1;
        value &= (cpp_int{1} << (sizeof(W) * 8)) - 1;
        const auto a = (value & mask).convert_to<std::uint64_t>();
        value >>= 64;
        const auto b = (value & mask).convert_to<std::uint64_t>();
        value >>= 64;
        if constexpr (sizeof(W) == 16)
            return W{a, b};
        else {
            const auto c = (value & mask).convert_to<std::uint64_t>();
            value >>= 64;
            return W{a, b, c, (value & mask).convert_to<std::uint64_t>()};
        }
    }
}
inline cpp_int random_bits(std::mt19937_64& rng, unsigned width) {
    cpp_int value = 0;
    for (unsigned i = 0; i < width; i += 64) value |= cpp_int{rng()} << i;
    return value & ((cpp_int{1} << width) - 1);
}
inline std::expected<cpp_int, ArithmeticError> rounded(cpp_int numerator, cpp_int denominator, Rounding mode,
                                                       unsigned width) {
    if (denominator == 0) return std::unexpected(ArithmeticError::division_by_zero);
    const bool negative = (numerator < 0) != (denominator < 0);
    if (numerator < 0) numerator = -numerator;
    if (denominator < 0) denominator = -denominator;
    cpp_int q = numerator / denominator, r = numerator % denominator;
    const cpp_int limit = (cpp_int{1} << (width - 1)) - (negative ? 0 : 1);
    if (q > limit) return std::unexpected(ArithmeticError::overflow);
    if (r != 0) {
        if (mode == Rounding::exact) return std::unexpected(ArithmeticError::inexact);
        if ((mode == Rounding::floor && negative) || (mode == Rounding::ceil && !negative) ||
            (mode == Rounding::nearest_away && 2 * r >= denominator) ||
            (mode == Rounding::nearest_even && (2 * r > denominator || (2 * r == denominator && (q & 1) != 0))))
            ++q;
    }
    if (q > limit) return std::unexpected(ArithmeticError::overflow);
    if (negative) q = -q;
    return q;
}
template<class T>
void agrees(const std::expected<T, ArithmeticError>& actual, const std::expected<cpp_int, ArithmeticError>& expected) {
    CHECK(actual.has_value() == expected.has_value());
    if (actual) {
        if constexpr (requires { actual->raw(); })
            CHECK(integer(actual->raw()) == *expected);
        else
            CHECK(integer(*actual) == *expected);
    } else
        CHECK(actual.error() == expected.error());
}
inline std::vector<cpp_int> boundaries(unsigned width) {
    const cpp_int limit = cpp_int{1} << (width - 1);
    std::vector<cpp_int> result{0, 1, -1, 2, -2, limit - 1, -limit, limit - 2, -limit + 1};
    for (unsigned bit = 8; bit < width; bit += 8) {
        const cpp_int v = cpp_int{1} << bit;
        result.insert(result.end(), {v, v - 1, -v, -v + 1});
    }
    return result;
}
} // namespace coverage_test
