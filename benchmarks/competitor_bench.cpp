#include <fixedwide/all.hpp>
#include <fpm/fixed.hpp>
#include <cnl/scaled_integer.h>
#include <boost/decimal.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>
#include <iostream>
#include <vector>
#include <random>
#include <charconv>

using namespace fixedwide;

inline void escape(void* p) {
    __asm__ __volatile__("" : : "g"(p) : "memory");
}
inline void escape_val(std::uint64_t v) {
    __asm__ __volatile__("" : : "r"(v) : "memory");
}

constexpr std::size_t N = 100000;
constexpr int WARMUP = 10;
constexpr int TRIALS = 5;

template<class Func>
double time_op(Func&& f) {
    for (int i = 0; i < WARMUP; ++i) f();
    double min_time = 1e18;
    for (int t = 0; t < TRIALS; ++t) {
        auto start = std::chrono::steady_clock::now();
        f();
        auto end = std::chrono::steady_clock::now();
        double dur_ns = std::chrono::duration<double, std::nano>(end - start).count() / N;
        if (dur_ns < min_time) min_time = dur_ns;
    }
    return min_time;
}

int main() {
    std::mt19937_64 rng(42);
    std::vector<std::int64_t> raw_a(N), raw_b(N);
    for (std::size_t i = 0; i < N; ++i) {
        raw_a[i] = (rng() % 100000000LL) + 1000000LL;
        raw_b[i] = (rng() % 50000000LL) + 1000000LL;
    }

    std::cout << "library,type,representation,operation,rounding,checked,ns_per_op\n";

    // 1. fixedwide Fixed64<12>
    {
        std::vector<Fixed64<12>> a(N), b(N), res(N);
        for (std::size_t i = 0; i < N; ++i) {
            a[i] = Fixed64<12>::from_raw(raw_a[i]);
            b[i] = Fixed64<12>::from_raw(raw_b[i]);
        }
        double t_mul_even = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = *mul(a[i], b[i], Rounding::nearest_even);
            escape(res.data());
        });
        double t_mul_zero = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = *mul(a[i], b[i], Rounding::toward_zero);
            escape(res.data());
        });
        double t_div_even = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = *div(a[i], b[i], Rounding::nearest_even);
            escape(res.data());
        });
        double t_div_zero = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = *div(a[i], b[i], Rounding::toward_zero);
            escape(res.data());
        });
        double t_add = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = *add(a[i], b[i]);
            escape(res.data());
        });
        std::cout << "fixedwide,Fixed64<12>,64-bit decimal,mul,nearest_even,yes," << t_mul_even << "\n";
        std::cout << "fixedwide,Fixed64<12>,64-bit decimal,mul,toward_zero,yes," << t_mul_zero << "\n";
        std::cout << "fixedwide,Fixed64<12>,64-bit decimal,div,nearest_even,yes," << t_div_even << "\n";
        std::cout << "fixedwide,Fixed64<12>,64-bit decimal,div,toward_zero,yes," << t_div_zero << "\n";
        std::cout << "fixedwide,Fixed64<12>,64-bit decimal,add,none,yes," << t_add << "\n";
    }

    // 2. fixedwide Fixed128<12>
    {
        std::vector<Fixed128<12>> a(N), b(N), res(N);
        for (std::size_t i = 0; i < N; ++i) {
            a[i] = Fixed128<12>::from_raw(wide::int128(raw_a[i]));
            b[i] = Fixed128<12>::from_raw(wide::int128(raw_b[i]));
        }
        double t_mul_even = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = *mul(a[i], b[i], Rounding::nearest_even);
            escape(res.data());
        });
        double t_mul_zero = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = *mul(a[i], b[i], Rounding::toward_zero);
            escape(res.data());
        });
        double t_div_even = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = *div(a[i], b[i], Rounding::nearest_even);
            escape(res.data());
        });
        double t_div_zero = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = *div(a[i], b[i], Rounding::toward_zero);
            escape(res.data());
        });
        double t_add = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = *add(a[i], b[i]);
            escape(res.data());
        });
        std::cout << "fixedwide,Fixed128<12>,128-bit decimal,mul,nearest_even,yes," << t_mul_even << "\n";
        std::cout << "fixedwide,Fixed128<12>,128-bit decimal,mul,toward_zero,yes," << t_mul_zero << "\n";
        std::cout << "fixedwide,Fixed128<12>,128-bit decimal,div,nearest_even,yes," << t_div_even << "\n";
        std::cout << "fixedwide,Fixed128<12>,128-bit decimal,div,toward_zero,yes," << t_div_zero << "\n";
        std::cout << "fixedwide,Fixed128<12>,128-bit decimal,add,none,yes," << t_add << "\n";
    }

    // 3. fpm::fixed (64-bit binary fixed point)
    {
        using Fpm64 = fpm::fixed<std::int64_t, __int128, 32>;
        std::vector<Fpm64> a(N), b(N), res(N);
        for (std::size_t i = 0; i < N; ++i) {
            a[i] = Fpm64::from_raw_value(raw_a[i]);
            b[i] = Fpm64::from_raw_value(raw_b[i]);
        }
        double t_mul = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] * b[i];
            escape(res.data());
        });
        double t_div = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] / b[i];
            escape(res.data());
        });
        double t_add = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] + b[i];
            escape(res.data());
        });
        std::cout << "fpm,fixed<i64;32>,64-bit binary,mul,trunc,no," << t_mul << "\n";
        std::cout << "fpm,fixed<i64;32>,64-bit binary,div,trunc,no," << t_div << "\n";
        std::cout << "fpm,fixed<i64;32>,64-bit binary,add,none,no," << t_add << "\n";
    }

    // 4. cnl::scaled_integer (64-bit binary & decimal)
    {
        using CnlBin = cnl::scaled_integer<std::int64_t, cnl::power<-32>>;
        std::vector<CnlBin> a(N), b(N), res(N);
        for (std::size_t i = 0; i < N; ++i) {
            a[i] = cnl::from_rep<CnlBin, std::int64_t>()(raw_a[i]);
            b[i] = cnl::from_rep<CnlBin, std::int64_t>()(raw_b[i]);
        }
        double t_mul = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] * b[i];
            escape(res.data());
        });
        double t_div = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] / b[i];
            escape(res.data());
        });
        double t_add = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] + b[i];
            escape(res.data());
        });
        std::cout << "cnl,scaled_integer<-32>,64-bit binary,mul,trunc,no," << t_mul << "\n";
        std::cout << "cnl,scaled_integer<-32>,64-bit binary,div,trunc,no," << t_div << "\n";
        std::cout << "cnl,scaled_integer<-32>,64-bit binary,add,none,no," << t_add << "\n";
    }

    // 5. boost::decimal::decimal64_t
    {
        std::vector<boost::decimal::decimal64_t> a(N), b(N), res(N);
        for (std::size_t i = 0; i < N; ++i) {
            a[i] = boost::decimal::decimal64_t(raw_a[i], -12);
            b[i] = boost::decimal::decimal64_t(raw_b[i], -12);
        }
        double t_mul = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] * b[i];
            escape(res.data());
        });
        double t_div = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] / b[i];
            escape(res.data());
        });
        double t_add = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] + b[i];
            escape(res.data());
        });
        std::cout << "boost.decimal,decimal64_t,IEEE-754 decimal float,mul,nearest_even,no," << t_mul << "\n";
        std::cout << "boost.decimal,decimal64_t,IEEE-754 decimal float,div,nearest_even,no," << t_div << "\n";
        std::cout << "boost.decimal,decimal64_t,IEEE-754 decimal float,add,nearest_even,no," << t_add << "\n";
    }

    // 6. Native IEEE-754 binary double
    {
        std::vector<double> a(N), b(N), res(N);
        for (std::size_t i = 0; i < N; ++i) {
            a[i] = static_cast<double>(raw_a[i]) * 1e-12;
            b[i] = static_cast<double>(raw_b[i]) * 1e-12;
        }
        double t_mul = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] * b[i];
            escape(res.data());
        });
        double t_div = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] / b[i];
            escape(res.data());
        });
        double t_add = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] + b[i];
            escape(res.data());
        });
        std::cout << "native,double,IEEE-754 binary float,mul,nearest_even,no," << t_mul << "\n";
        std::cout << "native,double,IEEE-754 binary float,div,nearest_even,no," << t_div << "\n";
        std::cout << "native,double,IEEE-754 binary float,add,nearest_even,no," << t_add << "\n";
    }

    // 7. boost::multiprecision::int128_t
    {
        namespace mp = boost::multiprecision;
        std::vector<mp::int128_t> a(N), b(N), res(N);
        for (std::size_t i = 0; i < N; ++i) {
            a[i] = raw_a[i];
            b[i] = raw_b[i];
        }
        double t_mul = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] * b[i];
            escape(res.data());
        });
        double t_div = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] / b[i];
            escape(res.data());
        });
        double t_add = time_op([&] {
            for (std::size_t i = 0; i < N; ++i) res[i] = a[i] + b[i];
            escape(res.data());
        });
        std::cout << "boost.multiprecision,int128_t,128-bit integer,mul,none,no," << t_mul << "\n";
        std::cout << "boost.multiprecision,int128_t,128-bit integer,div,trunc,no," << t_div << "\n";
        std::cout << "boost.multiprecision,int128_t,128-bit integer,add,none,no," << t_add << "\n";
    }

    return 0;
}
