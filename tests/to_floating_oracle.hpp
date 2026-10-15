#pragma once
#include "floating_test_bits.hpp"
#include <fixedwide/fixed.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <cstring>

namespace to_floating_test {
using namespace fixedwide;
using boost::multiprecision::cpp_int;
inline cpp_int pow10(unsigned d) {
    cpp_int value = 1;
    while (d-- != 0) value *= 10;
    return value;
}

// Independent oracle: binary search the rational's exponent, divide arbitrary
// precision integers at the FINAL output spacing, and encode the rounded result.
// No production decoder, reciprocal, normalization or FP arithmetic is used.
template<typename Float>
Float oracle(cpp_int raw, unsigned decimals) {
    const bool negative = raw < 0;
    if (negative) raw = -raw;
    constexpr int precision = std::numeric_limits<Float>::digits;
    constexpr int minimum = std::numeric_limits<Float>::min_exponent - 1;
    const cpp_int denominator = pow10(decimals);
    int low = -256, high = 256;
    while (low + 1 < high) {
        const int mid = low + (high - low) / 2;
        const bool at_least = mid >= 0 ? raw >= (denominator << mid) : (raw << -mid) >= denominator;
        if (at_least)
            low = mid;
        else
            high = mid;
    }
    const int exponent = std::max(low, minimum);
    const int shift = precision - 1 - exponent;
    cpp_int n = raw, d = denominator;
    if (shift >= 0)
        n <<= shift;
    else
        d <<= -shift;
    cpp_int q = n / d;
    const cpp_int remainder = n % d;
    if (2 * remainder > d || (2 * remainder == d && (q & 1) != 0)) ++q;
    unsigned field = q < (cpp_int{1} << (precision - 1))
                         ? 0u
                         : static_cast<unsigned>(exponent + std::numeric_limits<Float>::max_exponent - 1);
    if (q == (cpp_int{1} << precision)) {
        q >>= 1;
        ++field;
    }
    const cpp_int mask = (cpp_int{1} << 64) - 1;
    Float result;
    floating_test::store(result, negative, field,
                         wide::uint128{static_cast<std::uint64_t>(q & mask), static_cast<std::uint64_t>(q >> 64)});
    return result;
}

template<typename Float>
bool same(Float a, Float b) {
    // x87's trailing ABI padding is not part of its floating value.
    constexpr auto count = std::numeric_limits<Float>::digits == 64 ? 10 : sizeof(Float);
    return std::memcmp(&a, &b, count) == 0;
}

template<typename Target>
Target from_integer(cpp_int raw) {
    if constexpr (Target::bits <= 64)
        return Target::from_raw(static_cast<typename Target::raw_type>(raw));
    else {
        if (raw < 0) raw += cpp_int{1} << 128;
        const cpp_int mask = (cpp_int{1} << 64) - 1;
        return Target::from_raw(
            wide::int128{static_cast<std::uint64_t>(raw & mask), static_cast<std::uint64_t>(raw >> 64)});
    }
}

} // namespace to_floating_test
