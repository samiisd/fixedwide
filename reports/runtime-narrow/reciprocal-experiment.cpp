// Exploratory measurement only; not part of the library or a production kernel.
// Positive magnitudes are bounded by 2^127; the realistic products are checked.
// reciprocal = ceil(2^128 / D), giving an estimate at most one above floor(n/D).
// One exact correction recovers both quotient and remainder. The candidate is
// deliberately kept here to reproduce the decision about optional rescaling.
// This standalone experiment is separate from the Google Benchmark API suite.
// See reciprocal-environment.txt for the compiler, commands, and limitations.
#include <fixedwide/detail/mixed_native.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <random>
#include <vector>
namespace mn = fixedwide::detail::mixed_native;
using U = unsigned __int128;
using I = __int128;
using Div = mn::Divided;

U multiply_high(U a, U b) {
    U p00 = U(static_cast<std::uint64_t>(a)) * static_cast<std::uint64_t>(b);
    U p01 = U(static_cast<std::uint64_t>(a)) * (b >> 64);
    U p10 = (a >> 64) * static_cast<std::uint64_t>(b);
    U p11 = (a >> 64) * (b >> 64);
    U middle = (p00 >> 64) + U(static_cast<std::uint64_t>(p01)) + static_cast<std::uint64_t>(p10);
    return p11 + (p01 >> 64) + (p10 >> 64) + (middle >> 64);
}
template<std::uint64_t D>
Div reciprocal(U a) {
    constexpr U r = (~U{0} / D) + 1;
    U q = multiply_high(a, r);
    U product = q * D;
    if (product > a) {
        --q;
        product -= D;
    }
    return {q, a - product};
}
template<std::uint64_t D, bool Reciprocal>
[[gnu::noinline]] I operation(I a, I b) {
    I value;
    if (__builtin_mul_overflow(a, b, &value)) __builtin_trap();
    bool neg = value < 0;
    U mag = neg ? U{0} - U(value) : U(value);
    Div divided;
    if constexpr (Reciprocal)
        divided = reciprocal<D>(mag);
    else
        divided = mn::divide_magnitude(mag, D);
    U q = divided.quotient, r = divided.remainder;
    q += r * 2 > D || (r * 2 == D && (q & 1) != 0);
    return I(neg ? U{0} - q : q);
}
template<std::uint64_t D, bool Recip>
double time(const std::vector<I>& a, const std::vector<I>& b, std::uint64_t& sum) {
    constexpr std::size_t count = 1 << 22;
    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < count; ++i) {
        I r = operation<D, Recip>(a[i & 65535], b[i & 65535]);
        sum += static_cast<std::uint64_t>(r);
        asm volatile("" : "+r"(sum) : : "memory");
    }
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::nano>(end - start).count() / count;
}
template<std::uint64_t D>
void run(std::mt19937_64& gen) {
    std::vector<I> a(65536), b(65536);
    for (std::size_t i = 0; i < a.size(); ++i) {
        if constexpr (D == 10'000'000'000ULL) {
            a[i] = I(gen() % 100'000 + 1) * 1'000'000'000'000'000ULL;
            b[i] = I(gen() % 10'000'000'000'001ULL) - 5'000'000'000'000LL;
        } else {
            a[i] = (I(gen() % 20'000'000'000'000'001ULL) - 10'000'000'000'000'000LL) * 1000;
            b[i] = I(gen() % 4'800'000'000'001ULL + 200'000'000'000ULL);
        }
        if (operation<D, false>(a[i], b[i]) != operation<D, true>(a[i], b[i])) __builtin_trap();
    }
    for (std::size_t i = 0; i < 6'000'000; ++i) {
        U n = ((U(gen()) << 64) | gen()) >> 1;
        if (i % 3 == 0) n >>= gen() % 128;
        auto r = reciprocal<D>(n);
        auto expected = mn::divide_magnitude(n, D);
        if (r.quotient != expected.quotient || r.remainder != expected.remainder) __builtin_trap();
    }
    std::uint64_t sum = 0;
    std::vector<double> divs, recips;
    for (int rep = 0; rep < 9; ++rep) {
        if (rep % 2 == 0) {
            divs.push_back(time<D, false>(a, b, sum));
            recips.push_back(time<D, true>(a, b, sum));
        } else {
            recips.push_back(time<D, true>(a, b, sum));
            divs.push_back(time<D, false>(a, b, sum));
        }
    }
    for (std::size_t i = 0; i < divs.size(); ++i)
        std::printf("sample,D=%llu,pair=%zu,divq_ns=%.6f,reciprocal_ns=%.6f\n", (unsigned long long)D, i, divs[i],
                    recips[i]);
    std::sort(divs.begin(), divs.end());
    std::sort(recips.begin(), recips.end());
    std::printf("D=%llu divq=%.3f reciprocal=%.3f speedup=%.3f sink=%llu\n", (unsigned long long)D, divs[4], recips[4],
                divs[4] / recips[4], (unsigned long long)sum);
}
int main() {
    std::mt19937_64 gen(42);
    run<10'000'000'000ULL>(gen);
    run<1'000'000'000'000ULL>(gen);
}
