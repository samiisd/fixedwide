#include <fixedwide/all.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <iostream>
#include <random>
#include <cassert>
#include <vector>

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
mp::cpp_int to_cpp_int(typename F::raw_type r) {
    if constexpr (F::bits <= 64) {
        return mp::cpp_int(r);
    } else if constexpr (F::bits == 128) {
        bool neg = r.is_negative();
        auto mag = magnitude(r);
        mp::cpp_int val = (mp::cpp_int(mag.high) << 64) | mag.low;
        return neg ? -val : val;
    } else {
        bool neg = r.is_negative();
        auto mag = magnitude(r);
        mp::cpp_int val = 0;
        for (int k = 3; k >= 0; --k) val = (val << 64) | mag.limbs[k];
        return neg ? -val : val;
    }
}

template<class F>
void run_oracle_pair(F a, F b, const mp::cpp_int& min_val, const mp::cpp_int& max_val, const mp::cpp_int& scale) {
    const Rounding modes[] = {
        Rounding::toward_zero,
        Rounding::floor,
        Rounding::ceil,
        Rounding::nearest_even,
        Rounding::nearest_away,
        Rounding::exact
    };

    mp::cpp_int raw_a = to_cpp_int<F>(a.raw());
    mp::cpp_int raw_b = to_cpp_int<F>(b.raw());

    for (Rounding rm : modes) {
        // Test mul: (raw_a * raw_b) / scale
        mp::cpp_int num_mul = raw_a * raw_b;
        auto oracle_mul = round_rational(num_mul, scale, rm, min_val, max_val);
        auto fw_mul = mul(a, b, rm);

        ALWAYS_CHECK_EQ(oracle_mul.ok, fw_mul.has_value());
        if (oracle_mul.ok) {
            ALWAYS_CHECK_EQ(oracle_mul.value, to_cpp_int<F>(fw_mul->raw()));
        }

        // Test div: (raw_a * scale) / raw_b
        if (raw_b != 0) {
            mp::cpp_int num_div = raw_a * scale;
            auto oracle_div = round_rational(num_div, raw_b, rm, min_val, max_val);
            auto fw_div = div(a, b, rm);

            ALWAYS_CHECK_EQ(oracle_div.ok, fw_div.has_value());
            if (oracle_div.ok) {
                ALWAYS_CHECK_EQ(oracle_div.value, to_cpp_int<F>(fw_div->raw()));
            }
        }
    }
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

    mp::cpp_int scale = pow10_mp(F::fractional_digits);

    // 1. Test deterministic signed minima, boundaries, and exact tie points
    std::vector<typename F::raw_type> corners;
    corners.push_back(F::min().raw());
    corners.push_back(F::max().raw());
    corners.push_back(F::from_raw(static_cast<typename F::raw_type>(0)).raw());
    corners.push_back(F::from_raw(static_cast<typename F::raw_type>(1)).raw());
    corners.push_back(F::from_raw(static_cast<typename F::raw_type>(-1)).raw());
    corners.push_back(F::scale());

    for (auto c1 : corners) {
        for (auto c2 : corners) {
            run_oracle_pair(F::from_raw(c1), F::from_raw(c2), min_val, max_val, scale);
        }
    }

    // 2. Randomized sampling: small, medium, and full-width
    std::int64_t small_bound = (F::bits == 8) ? 127 : (F::bits == 16 ? 32767 : 1'000'000LL);
    std::uniform_int_distribution<std::int64_t> dist_small(-small_bound, small_bound);

    for (int i = 0; i < iterations; ++i) {
        typename F::raw_type raw_a{}, raw_b{};
        if (i % 3 == 0 || F::bits <= 32) {
            // Small range to exercise successful multiplications and divisions
            std::int64_t sa = dist_small(rng);
            std::int64_t sb = dist_small(rng);
            if (sb == 0) sb = 1;
            raw_a = static_cast<typename F::raw_type>(sa);
            raw_b = static_cast<typename F::raw_type>(sb);
        } else if (i % 3 == 1) {
            // Exact tie generator: construct raw_a such that raw_a * scale % raw_b == raw_b / 2
            std::int64_t base = dist_small(rng);
            std::int64_t divisor = (dist_small(rng) % 1000) * 2; // ensure even divisor
            if (divisor == 0) divisor = 2;
            std::int64_t tie_a = base * divisor + (divisor / 2);
            raw_a = static_cast<typename F::raw_type>(tie_a);
            raw_b = static_cast<typename F::raw_type>(divisor);
        } else {
            // Full-width randomized bits for 64, 128, and 256 bits
            if constexpr (F::bits == 64) {
                raw_a = static_cast<std::int64_t>(rng());
                raw_b = static_cast<std::int64_t>(rng());
                if (raw_b == 0) raw_b = 1;
            } else if constexpr (F::bits == 128) {
                std::uint64_t lo_a = rng(), hi_a = rng();
                std::uint64_t lo_b = rng(), hi_b = rng();
                raw_a = wide::int128(lo_a, hi_a);
                raw_b = wide::int128(lo_b, hi_b);
                if (raw_b.is_zero()) raw_b = wide::int128(1ULL);
            } else if constexpr (F::bits == 256) {
                raw_a = wide::int256(rng(), rng(), rng(), rng());
                raw_b = wide::int256(rng(), rng(), rng(), rng());
                if (raw_b.is_zero()) raw_b = wide::int256(1ULL);
            }
        }

        run_oracle_pair(F::from_raw(raw_a), F::from_raw(raw_b), min_val, max_val, scale);
    }
}

void test_mixed_oracle(int iterations) {
    std::mt19937_64 rng(12345);
    std::uniform_int_distribution<std::int64_t> dist(-50'000, 50'000);

    // Mixed domain 1: Fixed32<4> * Fixed64<8> -> Fixed128<12>
    using A1 = Fixed32<4>;
    using B1 = Fixed64<8>;
    using Dest1 = Fixed128<12>;
    mp::cpp_int min_val1 = - (mp::cpp_int(1) << 127);
    mp::cpp_int max_val1 = (mp::cpp_int(1) << 127) - 1;

    for (int i = 0; i < iterations; ++i) {
        std::int64_t ra = dist(rng);
        std::int64_t rb = dist(rng);
        if (rb == 0) rb = 1;

        auto a = A1::from_raw(static_cast<std::int32_t>(ra));
        auto b = B1::from_raw(rb);

        // Mixed comparison test: ra / 10^4 <=> rb / 10^8
        mp::cpp_int left = mp::cpp_int(ra) * pow10_mp(8);
        mp::cpp_int right = mp::cpp_int(rb) * pow10_mp(4);
        ALWAYS_CHECK_EQ(a == b, (left == right));
        ALWAYS_CHECK_EQ(a < b, (left < right));
        ALWAYS_CHECK_EQ(a > b, (left > right));

        // Mixed mul_to<Dest1>: (ra / 10^4) * (rb / 10^8) * 10^12 = ra * rb
        mp::cpp_int num = mp::cpp_int(ra) * rb;
        auto oracle_mul = round_rational(num, 1, Rounding::nearest_even, min_val1, max_val1);
        auto fw_mul = mul_to<Dest1>(a, b, Rounding::nearest_even);

        ALWAYS_CHECK_EQ(oracle_mul.ok, fw_mul.has_value());
        if (oracle_mul.ok) {
            ALWAYS_CHECK_EQ(oracle_mul.value, to_cpp_int<Dest1>(fw_mul->raw()));
        }
    }

    // Mixed domain 2: Fixed64<10> and Fixed128<12> -> Fixed256<18>
    using A2 = Fixed64<10>;
    using B2 = Fixed128<12>;
    using Dest2 = Fixed256<18>;
    mp::cpp_int min_val2 = - (mp::cpp_int(1) << 255);
    mp::cpp_int max_val2 = (mp::cpp_int(1) << 255) - 1;

    for (int i = 0; i < iterations; ++i) {
        std::int64_t ra = dist(rng);
        std::int64_t rb_lo = dist(rng);
        auto a = A2::from_raw(ra);
        auto b = B2::from_raw(wide::int128(rb_lo));

        // Mixed mul_to<Dest2>: (ra / 10^10) * (rb / 10^12) * 10^18 = (ra * rb) / 10^4
        mp::cpp_int num = mp::cpp_int(ra) * rb_lo;
        auto oracle_mul = round_rational(num, 10000, Rounding::nearest_even, min_val2, max_val2);
        auto fw_mul = mul_to<Dest2>(a, b, Rounding::nearest_even);

        ALWAYS_CHECK_EQ(oracle_mul.ok, fw_mul.has_value());
        if (oracle_mul.ok) {
            ALWAYS_CHECK_EQ(oracle_mul.value, to_cpp_int<Dest2>(fw_mul->raw()));
        }
    }
}

int main() {
    std::cout << "Running expanded Boost.Multiprecision differential oracle tests...\n";
    test_same_domain_oracle<Fixed8<2>>(1000);
    std::cout << "Fixed8 oracle passed.\n";
    test_same_domain_oracle<Fixed16<4>>(1000);
    std::cout << "Fixed16 oracle passed.\n";
    test_same_domain_oracle<Fixed32<9>>(1000);
    std::cout << "Fixed32 oracle passed.\n";
    test_same_domain_oracle<Fixed64<12>>(2000);
    std::cout << "Fixed64 oracle passed.\n";
    test_same_domain_oracle<Fixed128<12>>(2000);
    std::cout << "Fixed128 oracle passed.\n";
    test_same_domain_oracle<Fixed256<18>>(1000);
    std::cout << "Fixed256 oracle passed.\n";

    test_mixed_oracle(2000);
    std::cout << "Mixed oracle passed.\n";

    std::cout << "ALL EXPANDED DIFFERENTIAL ORACLE TESTS PASSED PERFECTLY!\n";
    return 0;
}
