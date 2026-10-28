// Independent rational checks for the full-width exact-grid certificate.
#include <fixedwide/batch.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include "check.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <random>
#include <utility>
#include <vector>

namespace fw = fixedwide;
using boost::multiprecision::cpp_int;
using Error = fw::ArithmeticError;

namespace {

constexpr std::array modes{fw::Rounding::toward_zero,  fw::Rounding::floor,        fw::Rounding::ceil,
                           fw::Rounding::nearest_even, fw::Rounding::nearest_away, fw::Rounding::exact};

cpp_int power_of_ten(unsigned exponent) {
    cpp_int result = 1;
    for (unsigned i = 0; i < exponent; ++i) result *= 10;
    return result;
}

fw::wide::int128 raw128(cpp_int value) {
    if (value < 0) value += cpp_int{1} << 128;
    const cpp_int mask = (cpp_int{1} << 64) - 1;
    return {static_cast<std::uint64_t>(value & mask), static_cast<std::uint64_t>(value >> 64)};
}

template<class T>
cpp_int integer(T value) {
    if constexpr (T::bits <= 64) {
        return cpp_int{value.raw()};
    } else {
        const auto raw = value.raw();
        cpp_int result = (cpp_int{raw.high} << 64) | raw.low;
        if ((raw.high >> 63) != 0) result -= cpp_int{1} << 128;
        return result;
    }
}

template<class Dest>
std::expected<cpp_int, Error> oracle(cpp_int numerator, const cpp_int& denominator, fw::Rounding mode) {
    const bool negative = numerator < 0;
    if (negative) numerator = -numerator;
    cpp_int quotient = numerator / denominator;
    const cpp_int remainder = numerator % denominator;
    const cpp_int limit = (cpp_int{1} << (Dest::bits - 1)) - (negative ? 0 : 1);
    if (quotient > limit) return std::unexpected(Error::overflow);
    if (remainder != 0) {
        if (mode == fw::Rounding::exact) return std::unexpected(Error::inexact);
        bool increment = false;
        switch (mode) {
        case fw::Rounding::toward_zero: break;
        case fw::Rounding::floor: increment = negative; break;
        case fw::Rounding::ceil: increment = !negative; break;
        case fw::Rounding::nearest_even:
            increment = remainder * 2 > denominator || (remainder * 2 == denominator && (quotient & 1) != 0);
            break;
        case fw::Rounding::nearest_away: increment = remainder * 2 >= denominator; break;
        case fw::Rounding::exact: break;
        }
        if (increment) ++quotient;
        if (quotient > limit) return std::unexpected(Error::overflow);
    }
    if (negative) quotient = -quotient;
    return quotient;
}

template<class Dest>
void verify(const std::expected<Dest, Error>& actual, const std::expected<cpp_int, Error>& expected) {
    CHECK(actual.has_value() == expected.has_value());
    if (expected)
        CHECK(integer(*actual) == *expected);
    else
        CHECK(actual.error() == expected.error());
}

template<class Dest, class Source, class Factor>
void product(const cpp_int& raw, std::int64_t factor) {
    constexpr unsigned gap = Source::fractional_digits + Factor::fractional_digits - Dest::fractional_digits;
    const auto lhs = Source::from_raw(raw128(raw));
    const auto rhs = Factor::from_raw(factor);
    for (const auto mode : modes) {
        const auto expected = oracle<Dest>(raw * factor, power_of_ten(gap), mode);
        verify(fw::mul_to<Dest>(lhs, rhs, mode), expected);
        verify(fw::mul_to<Dest>(rhs, lhs, mode), expected);
        verify(fw::batch::mul_to<Dest>(mode)(lhs, rhs), expected);
        verify(fw::batch::mul_to<Dest>(mode)(rhs, lhs), expected);
        verify(fw::batch::prepare_mul_to<Dest>(rhs, mode)(lhs), expected);
    }
}

template<class Dest, class Source, class Factor>
void boundaries() {
    constexpr unsigned gap = Source::fractional_digits + Factor::fractional_digits - Dest::fractional_digits;
    const cpp_int divisor = power_of_ten(gap);
    const cpp_int source_limit = cpp_int{1} << 127;
    const cpp_int maximum_coefficient = (source_limit - 1) / divisor;
    std::vector<cpp_int> coefficients{0, 1, -1, maximum_coefficient, -maximum_coefficient};
    for (unsigned bits : {7u, 15u, 31u, 32u, 63u, 64u, 93u, 94u, 95u}) {
        for (int offset : {-1, 0, 1}) {
            const cpp_int coefficient = (cpp_int{1} << bits) + offset;
            if (coefficient <= maximum_coefficient) {
                coefficients.push_back(coefficient);
                coefficients.push_back(-coefficient);
            }
        }
    }
    for (const auto& coefficient : coefficients) {
        const cpp_int scaled = coefficient * divisor;
        for (int offset : {-1, 0, 1}) {
            const cpp_int raw = scaled + offset;
            if (raw < -source_limit || raw >= source_limit) continue;
            for (std::int64_t factor :
                 {INT64_MIN, INT64_MIN + 1, std::int64_t{-2}, std::int64_t{-1}, std::int64_t{0}, std::int64_t{1},
                  std::int64_t{2}, std::int64_t{10'000'000'000}, std::int64_t{20'000'000'000}, INT64_MAX})
                product<Dest, Source, Factor>(raw, factor);
        }
    }
    product<Dest, Source, Factor>(-source_limit, INT64_MIN);
    product<Dest, Source, Factor>(source_limit - 1, INT64_MAX);
    product<Dest, Source, Factor>(-source_limit, 0);
    if constexpr (gap <= 18) {
        // A certificate miss must still allow the factor to restore exactness.
        product<Dest, Source, Factor>((cpp_int{1} << 80) + 1, static_cast<std::int64_t>(divisor));
        product<Dest, Source, Factor>((cpp_int{1} << 80) + 1024, static_cast<std::int64_t>(divisor));
    }
}

void randomized_products() {
    using Money = fw::Fixed128<12>;
    using Price = fw::Fixed64<10>;
    const cpp_int divisor = power_of_ten(10);
    const cpp_int limit = cpp_int{1} << 127;
    std::mt19937_64 rng(0xC04FF1C1E17);
    for (unsigned i = 0; i < 1200; ++i) {
        cpp_int raw = (cpp_int{rng()} << 64) | rng();
        if ((raw & limit) != 0) raw -= limit * 2;
        if (i % 3 != 0) raw = raw / divisor * divisor;
        const auto factor = std::bit_cast<std::int64_t>(rng());
        product<Money, Money, Price>(raw, i % 4 == 0 ? factor : factor % 1'000'000);
    }
}

#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
template<std::uint64_t Divisor>
void certificate() {
    namespace native = fw::detail::mixed_native;
    using native::i128;
    using native::u128;
    const auto check_certificate = [](i128 raw) {
        i128 quotient;
        const bool accepted = native::exact_quotient_i128<Divisor>(raw, quotient);
        const cpp_int value = integer(fw::Fixed128<0>::from_raw(static_cast<fw::wide::int128>(raw)));
        CHECK(accepted == (value % Divisor == 0));
        if (accepted)
            CHECK(integer(fw::Fixed128<0>::from_raw(static_cast<fw::wide::int128>(quotient))) == value / Divisor);
    };
    constexpr auto divisor = static_cast<i128>(Divisor);
    constexpr auto lower = native::dest_min<128>() / divisor;
    constexpr auto upper = native::dest_max<128>() / divisor;
    for (i128 q : {lower, lower + 1, i128{INT64_MIN} - 1, i128{INT64_MIN}, i128{-1}, i128{0}, i128{1}, i128{INT64_MAX},
                   i128{INT64_MAX} + 1, upper - 1, upper}) {
        if (q < lower || q > upper) continue;
        const i128 raw = q * divisor;
        check_certificate(raw);
        if (raw != native::dest_min<128>()) check_certificate(raw - 1);
        if (raw != native::dest_max<128>()) check_certificate(raw + 1);
    }
    check_certificate(native::dest_min<128>());
    check_certificate(native::dest_max<128>());
    std::mt19937_64 rng(0xB0A1D + Divisor);
    for (unsigned i = 0; i < 2000; ++i) {
        const i128 raw = std::bit_cast<i128>((u128{rng()} << 64) | rng());
        check_certificate(raw);
        check_certificate(raw / divisor * divisor);
    }
}

template<std::size_t... Gap>
void certificates(std::index_sequence<Gap...>) {
    (certificate<static_cast<std::uint64_t>(fw::detail::mixed_native::pow10_v<Gap + 1>)>(), ...);
    certificate<1>();
    certificate<UINT64_MAX>();
    static_assert([] {
        using namespace fw::detail::mixed_native;
        i128 quotient;
        constexpr i128 expected = -(i128{1} << 80) + 1;
        return exact_quotient_i128<10'000'000'000>(expected * 10'000'000'000, quotient) && quotient == expected;
    }());
}
#endif

} // namespace

int main() {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    certificates(std::make_index_sequence<19>{});
#endif
    boundaries<fw::Fixed128<12>, fw::Fixed128<12>, fw::Fixed64<10>>();
    boundaries<fw::Fixed128<0>, fw::Fixed128<1>, fw::Fixed64<0>>();
    boundaries<fw::Fixed128<0>, fw::Fixed128<19>, fw::Fixed64<0>>();
    boundaries<fw::Fixed64<0>, fw::Fixed128<10>, fw::Fixed64<0>>();
    boundaries<fw::Fixed32<0>, fw::Fixed128<10>, fw::Fixed64<0>>();
    boundaries<fw::Fixed16<0>, fw::Fixed128<10>, fw::Fixed64<0>>();
    boundaries<fw::Fixed8<0>, fw::Fixed128<10>, fw::Fixed64<0>>();
    randomized_products();
    std::printf("exact coefficient oracle: %llu checks passed\n", static_cast<unsigned long long>(checks));
}
