// Independent integer/rational oracle for the batch multiplication paths.
// No fixedwide arithmetic is used to derive the expected value or error.
#include <fixedwide/batch.hpp>
#include <fixedwide/literal.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include "check.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <random>
#include <span>
#include <type_traits>
#include <vector>

namespace fw = fixedwide;
namespace batch = fw::batch;
using boost::multiprecision::cpp_int;
using Error = fw::ArithmeticError;
using Money = fw::Fixed128<12>;
using Price = fw::Fixed64<10>;

namespace {

constexpr std::array modes{fw::Rounding::toward_zero,  fw::Rounding::floor,        fw::Rounding::ceil,
                           fw::Rounding::nearest_even, fw::Rounding::nearest_away, fw::Rounding::exact};

cpp_int power_of_ten(unsigned exponent) {
    cpp_int value = 1;
    for (unsigned i = 0; i < exponent; ++i) value *= 10;
    return value;
}

template<class T>
cpp_int raw_integer(T value) {
    if constexpr (T::bits <= 64) {
        return cpp_int{value.raw()};
    } else if constexpr (T::bits == 128) {
        const auto raw = value.raw();
        cpp_int integer = (cpp_int{raw.high} << 64) | raw.low;
        if ((raw.high >> 63) != 0) integer -= cpp_int{1} << 128;
        return integer;
    } else {
        const auto raw = value.raw();
        cpp_int integer = 0;
        for (std::size_t i = 4; i-- > 0;) integer = (integer << 64) | raw.limbs[i];
        if ((raw.limbs[3] >> 63) != 0) integer -= cpp_int{1} << 256;
        return integer;
    }
}

struct OracleResult {
    cpp_int value{};
    Error error{};
    bool ok{};
};

OracleResult rounded(cpp_int numerator, const cpp_int& denominator, std::size_t bits, fw::Rounding mode) {
    const bool negative = numerator < 0;
    if (negative) numerator = -numerator;
    cpp_int quotient = numerator / denominator;
    const cpp_int remainder = numerator % denominator;
    const cpp_int limit = (cpp_int{1} << (bits - 1)) - (negative ? 0 : 1);
    if (quotient > limit) return {{}, Error::overflow, false};
    if (remainder != 0) {
        if (mode == fw::Rounding::exact) return {{}, Error::inexact, false};
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
        if (quotient > limit) return {{}, Error::overflow, false};
    }
    if (negative) quotient = -quotient;
    return {quotient, {}, true};
}

template<class Dest, class A, class B>
OracleResult multiply(A a, B b, fw::Rounding mode) {
    constexpr auto source_scale = A::fractional_digits + B::fractional_digits;
    cpp_int numerator = raw_integer(a) * raw_integer(b);
    cpp_int denominator = 1;
    if constexpr (Dest::fractional_digits >= source_scale)
        numerator *= power_of_ten(Dest::fractional_digits - source_scale);
    else
        denominator = power_of_ten(source_scale - Dest::fractional_digits);
    return rounded(numerator, denominator, Dest::bits, mode);
}

template<class T>
void verify(const std::expected<T, Error>& got, const OracleResult& expected) {
    CHECK(got.has_value() == expected.ok);
    if (expected.ok)
        CHECK(raw_integer(*got) == expected.value);
    else
        CHECK(got.error() == expected.error);
}

template<class Dest, class A, class B>
void check_product(A a, B b) {
    const std::array<A, 1> lhs{a};
    const std::array<B, 1> rhs{b};
    for (auto mode : modes) {
        const auto reference = multiply<Dest>(a, b, mode);
        verify(batch::prepare_mul_to<Dest>(b, mode)(a), reference);
        verify(batch::mul_to<Dest>(mode)(a, b), reference);
        verify(fw::mul_to<Dest>(a, b, mode), reference);
        std::array<Dest, 1> output{Dest::max()};
        const auto result = batch::mul_to<Dest>(std::span{output}, std::span{lhs}, std::span{rhs}, mode);
        CHECK(result.has_value() == reference.ok);
        if (reference.ok)
            CHECK(raw_integer(output[0]) == reference.value);
        else {
            CHECK(result.error().index == 0);
            CHECK(result.error().error == reference.error);
            CHECK(output[0] == Dest::max());
        }
    }
}

template<class T>
T random_value(std::mt19937_64& generator) {
    if constexpr (T::bits <= 64) {
        using Raw = typename T::raw_type;
        using Unsigned = std::make_unsigned_t<Raw>;
        return T::from_raw(std::bit_cast<Raw>(static_cast<Unsigned>(generator())));
    } else if constexpr (T::bits == 128) {
        return T::from_raw(fw::wide::int128{generator(), generator()});
    } else {
        return T::from_raw(fw::wide::int256{generator(), generator(), generator(), generator()});
    }
}

template<class T>
T small(std::int64_t value) {
    return T::from_raw(static_cast<typename T::raw_type>(value));
}

template<class Dest, class A, class B>
void sample(unsigned trials, std::uint64_t seed) {
    const std::array boundary_a{A{}, A::min(), A::max(), small<A>(1), small<A>(-1)};
    const std::array boundary_b{B{}, B::min(), B::max(), small<B>(1), small<B>(-1)};
    for (auto a : boundary_a)
        for (auto b : boundary_b) check_product<Dest>(a, b);
    std::mt19937_64 generator{seed};
    for (unsigned i = 0; i < trials; ++i) {
        const auto a = random_value<A>(generator);
        const auto b = random_value<B>(generator);
        check_product<Dest>(a, b);
        // Full-width random pairs often overflow. A single-limb partner also
        // exercises representable results and every nonzero remainder.
        check_product<Dest>(a, small<B>(static_cast<std::int64_t>(generator() % 200'001) - 100'000));
    }
}

void exact_delta_and_fx() {
    constexpr std::array slopes{Money{"1"},     Money{"100"},    Money{"100000"},  Money{"0.01"},
                                Money{"-1"},    Money{"-100"},   Money{"-100000"}, Money{"-0.01"},
                                Money{"0.005"}, Money{"-0.005"}, Money::min(),     Money::max()};
    constexpr std::array factors{Price::from_raw(1), Price::from_raw(-1), Price{"1"},    Price{"-1"},
                                 Price{"0.00001"},   Price{"0.03"},       Price{"0.05"}, Price{"1.234567891"},
                                 Price::min(),       Price::max()};
    for (auto a : slopes)
        for (auto b : factors) check_product<Money>(a, b);
    constexpr std::array fx{Money{"0"},   Money{"1"},      Money{"-1"},
                            Money{"0.1"}, Money{"1.2345"}, Money{"100000000000000000000"},
                            Money::min(), Money::max()};
    for (auto a : slopes)
        for (auto b : fx) check_product<Money>(a, b);

    const auto scale = fw::wide::int128{10'000'000'000LL};
    const auto high = fw::wide::int128::max();
    const auto half = scale >> 1;
    const auto largest_whole = high / 10'000'000'000LL;
    const std::array anchors{fw::wide::int128{0}, fw::wide::int128{1}, largest_whole - fw::wide::int128{1},
                             largest_whole - fw::wide::int128{2}};
    for (auto q : anchors) {
        for (std::int64_t offset : {-1, 0, 1}) {
            const auto raw = q * scale + half + fw::wide::int128{offset};
            check_product<Money>(Money::from_raw(raw), Price::from_raw(1));
            check_product<Money>(Money::from_raw(-raw), Price::from_raw(1));
        }
    }
}

Money positive_raw(const cpp_int& value) {
    const cpp_int mask = (cpp_int{1} << 64) - 1;
    return Money::from_raw(
        fw::wide::int128{static_cast<std::uint64_t>(value & mask), static_cast<std::uint64_t>(value >> 64)});
}

void prepared_divisor_boundaries() {
    using Factor = fw::Fixed128<19>;
    const cpp_int scale = power_of_ten(19);
    const cpp_int limb = cpp_int{1} << 64;
    const cpp_int max_raw = (cpp_int{1} << 127) - 1;
    const cpp_int mask = limb - 1;
    // Reducing a decimal broadcast can leave any divisor 2^a * 5^b, not
    // just another power of ten. Cover every family through the largest
    // supported native scale, including reciprocal corrections and d == 1.
    for (unsigned twos = 0; twos <= 19; ++twos) {
        cpp_int denominator = cpp_int{1} << twos;
        for (unsigned fives = 0; fives <= 19; ++fives, denominator *= 5) {
            const cpp_int common = scale / denominator;
            for (const cpp_int& numerator : std::array<cpp_int, 3>{1, 3, (cpp_int{1} << 63) + 1}) {
                const cpp_int factor_raw = common * numerator;
                CHECK(factor_raw <= max_raw);
                const auto factor = Factor::from_raw(fw::wide::int128{static_cast<std::uint64_t>(factor_raw & mask),
                                                                      static_cast<std::uint64_t>(factor_raw >> 64)});
                // Force the one-limb/two-limb boundary and values adjacent
                // to an exact quotient. The large reduced factor also uses
                // the unsigned half of a limb, above INT64_MAX.
                const cpp_int multiple = denominator * ((cpp_int{1} << 63) - 1);
                for (const cpp_int& magnitude :
                     std::array<cpp_int, 5>{limb - 1, limb, multiple - 1, multiple, multiple + 1}) {
                    const auto value = positive_raw(magnitude);
                    check_product<Money>(value, factor);
                    check_product<Money>(Money::from_raw(-value.raw()), factor);
                }
            }
        }
    }
}

void prepared_divisor_high_boundaries() {
#if defined(FIXEDWIDE_HAS_MIXED_NATIVE)
    using U128 = unsigned __int128;
    constexpr auto maximum = ~std::uint64_t{0};
    // Exercise the high-limb shortcut's exact transition. Include the full
    // unsigned128 domain: a rounded one-limb product can exceed signed128.
    for (const auto divisor : std::array<std::uint64_t, 7>{1, 2, 5, 10'000'000'000ULL, 1'000'000'000'000ULL,
                                                           10'000'000'000'000'000'000ULL, maximum}) {
        const fw::detail::batch::prepared_divisor divide{divisor};
        for (int offset : {-1, 0, 1}) {
            if (offset == 1 && divisor == maximum) continue;
            const auto high = offset < 0 ? divisor - 1 : (offset == 0 ? divisor : divisor + 1);
            for (const auto low : std::array<std::uint64_t, 3>{0, 1, maximum}) {
                const U128 numerator = (static_cast<U128>(high) << 64) | low;
                const auto actual = divide(numerator);
                CHECK(actual.quotient == numerator / divisor);
                CHECK(actual.remainder == numerator % divisor);
            }
        }
    }
#endif
}

template<class T>
T raw128(cpp_int value) {
    static_assert(T::bits == 128);
    CHECK(value >= -(cpp_int{1} << 127) && value < (cpp_int{1} << 127));
    if (value < 0) value += cpp_int{1} << 128;
    const cpp_int mask = (cpp_int{1} << 64) - 1;
    return T::from_raw(
        fw::wide::int128{static_cast<std::uint64_t>(value & mask), static_cast<std::uint64_t>(value >> 64)});
}

template<std::size_t BlockSize, class Dest, class A, class B>
void check_prepared_array(std::span<const A> input, B factor, fw::Rounding mode = fw::Rounding::exact) {
    const Dest sentinel = Dest::from_raw(77);
    std::vector<Dest> output(input.size(), sentinel);
    const auto result =
        batch::transform<BlockSize>(std::span{output}, batch::prepare_mul_to<Dest>(factor, mode), input);
    std::size_t first_error = input.size();
    for (std::size_t row = 0; row < input.size(); ++row) {
        const auto expected = multiply<Dest>(input[row], factor, mode);
        if (!expected.ok) {
            first_error = row;
            CHECK(!result);
            CHECK(result.error().index == row);
            CHECK(result.error().error == expected.error);
            break;
        }
        CHECK(raw_integer(output[row]) == expected.value);
    }
    if (first_error == input.size()) CHECK(result);
    for (std::size_t row = first_error; row < output.size(); ++row) CHECK(output[row] == sentinel);
}

template<unsigned Gap>
void prepared_array_scale_gap() {
    using A = fw::Fixed128<Gap>;
    using Dest = fw::Fixed128<0>;
    using Factor = fw::Fixed64<0>;
    const cpp_int denominator = power_of_ten(Gap);
    const cpp_int upper = (cpp_int{1} << 127) - 1;
    const cpp_int lower = -(cpp_int{1} << 127);
    const cpp_int signed32 = cpp_int{1} << 31;
    const cpp_int unsigned32 = (cpp_int{1} << 32) - 1;
    const cpp_int largest_grid = upper / denominator;
    std::vector<A> aligned;
    const auto append = [&](const cpp_int& q) {
        const cpp_int value = q * denominator;
        if (value >= lower && value <= upper) aligned.push_back(raw128<A>(value));
    };
    for (const cpp_int& q : std::array<cpp_int, 20>{0,
                                                    1,
                                                    -1,
                                                    -signed32 - 1,
                                                    -signed32,
                                                    -signed32 + 1,
                                                    signed32 - 1,
                                                    signed32,
                                                    unsigned32 - 1,
                                                    unsigned32,
                                                    unsigned32 + 1,
                                                    -unsigned32,
                                                    -unsigned32 - 1,
                                                    cpp_int{1} << 40,
                                                    -(cpp_int{1} << 40),
                                                    (cpp_int{1} << 63) - 1,
                                                    -(cpp_int{1} << 63),
                                                    largest_grid - 1,
                                                    largest_grid,
                                                    -largest_grid})
        append(q);
    // Reconstruction of q * 10^gap must carry between64-bit limbs correctly.
    // Include the neighbouring exact values on both sides of that carry.
    const cpp_int carry = (cpp_int{1} << 64) / denominator;
    if (carry <= unsigned32) {
        for (int offset : {-2, -1, 0, 1, 2}) {
            append(carry + offset);
            append(-carry + offset);
        }
    }
    constexpr std::array<std::int64_t, 12> factors{
        0, 1, -1, 2, -3, 37, -37, INT32_MIN, INT32_MAX, (std::int64_t{1} << 43) + 17, INT64_MIN, INT64_MAX};
    for (const auto p : factors) {
        const auto factor = Factor::from_raw(p);
        check_prepared_array<4, Dest>(std::span<const A>{aligned}, factor);
        check_prepared_array<8, Dest>(std::span<const A>{aligned}, factor);
    }

    // These values cannot all use a compact grid quotient. Some products are
    // nevertheless exact after factor cancellation; others round or overflow.
    // Move each value through every SIMD lane so early failures cannot hide it.
    for (const cpp_int& value : std::array<cpp_int, 9>{1, -1, denominator / 2, -denominator / 2, denominator / 2 - 1,
                                                       denominator / 5, denominator + denominator / 2, upper, lower}) {
        for (std::size_t lane = 0; lane < 4; ++lane) {
            std::array<A, 4> input;
            input.fill(raw128<A>(denominator));
            input[lane] = raw128<A>(value);
            for (auto mode : modes)
                for (const auto p : std::array<std::int64_t, 4>{2, 5, -37, INT64_MIN})
                    check_prepared_array<4, Dest>(std::span<const A>{input}, Factor::from_raw(p), mode);
        }
    }
}

template<std::size_t... Index>
void prepared_array_all_scales(std::index_sequence<Index...>) {
    (prepared_array_scale_gap<static_cast<unsigned>(Index + 1)>(), ...);
}

} // namespace

int main() {
    exact_delta_and_fx();
    prepared_divisor_boundaries();
    prepared_divisor_high_boundaries();
    prepared_array_all_scales(std::make_index_sequence<19>{});
    // The same public blocked call must retain scalar behaviour outside the
    // SIMD scale window; these are compile-time fallback configurations.
    prepared_array_scale_gap<0>();
    prepared_array_scale_gap<20>();
    sample<Money, Money, Price>(1200, 1);
    sample<Money, Price, Money>(300, 2);
    sample<Money, Money, Money>(900, 3);
    sample<fw::Fixed8<0>, fw::Fixed128<1>, fw::Fixed64<0>>(100, 4);
    sample<fw::Fixed16<2>, fw::Fixed32<4>, fw::Fixed64<8>>(100, 12);
    sample<fw::Fixed32<4>, fw::Fixed64<8>, fw::Fixed16<2>>(100, 13);
    sample<fw::Fixed128<0>, fw::Fixed128<19>, fw::Fixed64<0>>(200, 5);
    sample<fw::Fixed128<0>, fw::Fixed128<0>, fw::Fixed128<19>>(200, 14);
    sample<fw::Fixed128<0>, fw::Fixed128<20>, fw::Fixed64<0>>(200, 6);
    sample<fw::Fixed128<0>, fw::Fixed128<38>, fw::Fixed128<38>>(100, 7);
    sample<fw::Fixed256<24>, Money, Money>(150, 8);
    sample<fw::Fixed256<36>, fw::Fixed256<24>, Money>(100, 9);
    sample<fw::Fixed256<76>, fw::Fixed128<0>, fw::Fixed64<0>>(100, 10);
    sample<fw::Fixed256<0>, fw::Fixed256<76>, fw::Fixed256<76>>(100, 11);

    // Exhaust all raw inputs of the smallest fixed-point multiplication. This
    // covers the complete signed domain, including every tie and endpoint.
    for (int a = -128; a <= 127; ++a)
        for (int b = -128; b <= 127; ++b)
            check_product<fw::Fixed8<1>>(small<fw::Fixed8<1>>(a), small<fw::Fixed8<1>>(b));
    std::printf("batch oracle: %llu checks passed\n", static_cast<unsigned long long>(checks));
}
