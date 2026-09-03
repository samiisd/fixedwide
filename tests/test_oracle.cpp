#include <fixedwide/all.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <iostream>
#include <random>
#include <cassert>

using namespace fixedwide;
namespace mp = boost::multiprecision;

#define ALWAYS_CHECK(cond) \
    do { \
        if (!(cond)) { \
            std::cerr << "ORACLE CHECK FAILED: " #cond << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
            std::abort(); \
        } \
    } while (0)

#define ALWAYS_CHECK_EQ(a, b) \
    do { \
        if (!((a) == (b))) { \
            std::cerr << "ORACLE CHECK FAILED: " #a " == " #b \
                      << " (" << (a) << " vs " << (b) << ") at " << __FILE__ << ":" << __LINE__ << "\n"; \
            std::abort(); \
        } \
    } while (0)

// Exact rational oracle with all 6 rounding modes
struct OracleResult {
    bool ok{false};
    mp::cpp_int value{0};
    ArithmeticError error{ArithmeticError::overflow};
};

OracleResult round_rational(mp::cpp_int P, mp::cpp_int Q, Rounding rounding,
                            mp::cpp_int min_val, mp::cpp_int max_val) {
    if (Q == 0) return {false, 0, ArithmeticError::division_by_zero};
    if (Q < 0) { P = -P; Q = -Q; }

    mp::cpp_int q = P / Q;
    mp::cpp_int r = P % Q; // signed remainder, same sign as P

    if (r != 0) {
        if (rounding == Rounding::exact) {
            return {false, 0, ArithmeticError::inexact};
        }
        bool increment = false;
        bool negative = P < 0;
        mp::cpp_int abs_r = negative ? -r : r;

        switch (rounding) {
        case Rounding::toward_zero: break;
        case Rounding::floor: increment = negative; break;
        case Rounding::ceil: increment = !negative; break;
        case Rounding::nearest_away:
            increment = (abs_r * 2 >= Q);
            break;
        case Rounding::nearest_even: {
            mp::cpp_int rem2 = abs_r * 2;
            if (rem2 > Q) {
                increment = true;
            } else if (rem2 == Q) {
                // Halfway tie: round to even
                mp::cpp_int abs_q = q < 0 ? -q : q;
                increment = ((abs_q % 2) != 0);
            }
            break;
        }
        case Rounding::exact: break;
        }

        if (increment) {
            if (negative) q -= 1;
            else q += 1;
        }
    }

    if (q < min_val || q > max_val) {
        return {false, 0, ArithmeticError::overflow};
    }
    return {true, q, ArithmeticError::overflow};
}

mp::cpp_int pow10_mp(unsigned d) {
    mp::cpp_int res = 1;
    for (unsigned i = 0; i < d; ++i) res *= 10;
    return res;
}

template<class F>
void test_same_domain_oracle(int iterations) {
    std::mt19937_64 rng(42);

    mp::cpp_int min_val, max_val;
    if constexpr (F::bits == 8) { min_val = INT8_MIN; max_val = INT8_MAX; }
    else if constexpr (F::bits == 16) { min_val = INT16_MIN; max_val = INT16_MAX; }
    else if constexpr (F::bits == 32) { min_val = INT32_MIN; max_val = INT32_MAX; }
    else if constexpr (F::bits == 64) { min_val = INT64_MIN; max_val = INT64_MAX; }
    else if constexpr (F::bits == 128) {
        min_val = - (mp::cpp_int(1) << 127);
        max_val = (mp::cpp_int(1) << 127) - 1;
    } else {
        min_val = - (mp::cpp_int(1) << 255);
        max_val = (mp::cpp_int(1) << 255) - 1;
    }

    std::int64_t low_bound = (F::bits == 8) ? -128 : (F::bits == 16 ? -32768 : -1'000'000LL);
    std::int64_t high_bound = (F::bits == 8) ? 127 : (F::bits == 16 ? 32767 : 1'000'000LL);
    std::uniform_int_distribution<std::int64_t> dist(low_bound, high_bound);

    const Rounding modes[] = {
        Rounding::toward_zero,
        Rounding::floor,
        Rounding::ceil,
        Rounding::nearest_even,
        Rounding::nearest_away,
        Rounding::exact
    };

    mp::cpp_int scale = pow10_mp(F::fractional_digits);

    for (int i = 0; i < iterations; ++i) {
        std::int64_t raw_a = dist(rng);
        std::int64_t raw_b = dist(rng);
        if (raw_b == 0) raw_b = 1;

        auto a = F::from_raw(static_cast<typename F::raw_type>(raw_a));
        auto b = F::from_raw(static_cast<typename F::raw_type>(raw_b));

        for (Rounding rm : modes) {
            // Test mul: (raw_a * raw_b) / scale
            mp::cpp_int num_mul = mp::cpp_int(raw_a) * raw_b;
            auto oracle_mul = round_rational(num_mul, scale, rm, min_val, max_val);
            auto fw_mul = mul(a, b, rm);

            ALWAYS_CHECK_EQ(oracle_mul.ok, fw_mul.has_value());
            if (oracle_mul.ok) {
                mp::cpp_int fw_val;
                if constexpr (F::bits <= 64) fw_val = fw_mul->raw();
                else if constexpr (F::bits == 128) {
                    bool neg = fw_mul->raw().is_negative();
                    auto mag = magnitude(fw_mul->raw());
                    fw_val = (mp::cpp_int(mag.high) << 64) | mag.low;
                    if (neg) fw_val = -fw_val;
                } else {
                    bool neg = fw_mul->raw().is_negative();
                    auto mag = magnitude(fw_mul->raw());
                    fw_val = 0;
                    for (int k = 3; k >= 0; --k) fw_val = (fw_val << 64) | mag.limbs[k];
                    if (neg) fw_val = -fw_val;
                }
                ALWAYS_CHECK_EQ(oracle_mul.value, fw_val);
            }

            // Test div: (raw_a * scale) / raw_b
            mp::cpp_int num_div = mp::cpp_int(raw_a) * scale;
            auto oracle_div = round_rational(num_div, raw_b, rm, min_val, max_val);
            auto fw_div = div(a, b, rm);

            ALWAYS_CHECK_EQ(oracle_div.ok, fw_div.has_value());
            if (oracle_div.ok) {
                mp::cpp_int fw_val;
                if constexpr (F::bits <= 64) fw_val = fw_div->raw();
                else if constexpr (F::bits == 128) {
                    bool neg = fw_div->raw().is_negative();
                    auto mag = magnitude(fw_div->raw());
                    fw_val = (mp::cpp_int(mag.high) << 64) | mag.low;
                    if (neg) fw_val = -fw_val;
                } else {
                    bool neg = fw_div->raw().is_negative();
                    auto mag = magnitude(fw_div->raw());
                    fw_val = 0;
                    for (int k = 3; k >= 0; --k) fw_val = (fw_val << 64) | mag.limbs[k];
                    if (neg) fw_val = -fw_val;
                }
                ALWAYS_CHECK_EQ(oracle_div.value, fw_val);
            }
        }
    }
}

void test_mixed_oracle(int iterations) {
    std::mt19937_64 rng(12345);
    std::uniform_int_distribution<std::int64_t> dist(-50'000, 50'000);

    using A = Fixed32<4>;
    using B = Fixed64<8>;
    using Dest = Fixed128<12>;

    mp::cpp_int min_val = - (mp::cpp_int(1) << 127);
    mp::cpp_int max_val = (mp::cpp_int(1) << 127) - 1;

    for (int i = 0; i < iterations; ++i) {
        std::int64_t ra = dist(rng);
        std::int64_t rb = dist(rng);
        if (rb == 0) rb = 1;

        auto a = A::from_raw(static_cast<std::int32_t>(ra));
        auto b = B::from_raw(rb);

        // Mixed comparison test: ra / 10^4 <=> rb / 10^8
        mp::cpp_int left = mp::cpp_int(ra) * pow10_mp(8);
        mp::cpp_int right = mp::cpp_int(rb) * pow10_mp(4);
        bool eq_oracle = (left == right);
        bool lt_oracle = (left < right);
        bool gt_oracle = (left > right);

        ALWAYS_CHECK_EQ(a == b, eq_oracle);
        ALWAYS_CHECK_EQ(a < b, lt_oracle);
        ALWAYS_CHECK_EQ(a > b, gt_oracle);

        // Mixed mul_to<Dest>: (ra / 10^4) * (rb / 10^8) * 10^12 = ra * rb
        mp::cpp_int num = mp::cpp_int(ra) * rb;
        auto oracle_mul = round_rational(num, 1, Rounding::nearest_even, min_val, max_val);
        auto fw_mul = mul_to<Dest>(a, b, Rounding::nearest_even);

        ALWAYS_CHECK_EQ(oracle_mul.ok, fw_mul.has_value());
        if (oracle_mul.ok) {
            bool neg = fw_mul->raw().is_negative();
            auto mag = magnitude(fw_mul->raw());
            mp::cpp_int fw_val = (mp::cpp_int(mag.high) << 64) | mag.low;
            if (neg) fw_val = -fw_val;
            ALWAYS_CHECK_EQ(oracle_mul.value, fw_val);
        }
    }
}

int main() {
    std::cout << "Running Boost.Multiprecision differential oracle tests...\n";
    test_same_domain_oracle<Fixed8<2>>(2000);
    std::cout << "Fixed8 oracle passed.\n";
    test_same_domain_oracle<Fixed16<4>>(2000);
    std::cout << "Fixed16 oracle passed.\n";
    test_same_domain_oracle<Fixed32<9>>(2000);
    std::cout << "Fixed32 oracle passed.\n";
    test_same_domain_oracle<Fixed64<12>>(2000);
    std::cout << "Fixed64 oracle passed.\n";
    test_same_domain_oracle<Fixed128<12>>(2000);
    std::cout << "Fixed128 oracle passed.\n";
    test_same_domain_oracle<Fixed256<18>>(1000);
    std::cout << "Fixed256 oracle passed.\n";

    test_mixed_oracle(2000);
    std::cout << "Mixed oracle passed.\n";

    std::cout << "ALL DIFFERENTIAL ORACLE TESTS PASSED PERFECTLY!\n";
    return 0;
}
