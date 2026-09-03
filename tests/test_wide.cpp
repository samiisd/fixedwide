#include "check.hpp"
#include <fixedwide/wide.hpp>
#include "src/detail.hpp"
#include "src/limbs.hpp"
#include <cstdint>
#include <random>

using namespace fixedwide;
using namespace fixedwide::detail;

void test_wide_basics() {
    wide::uint128 u1(10, 0);
    wide::uint128 u2(20, 0);
    CHECK(u1 + u2 == wide::uint128(30, 0));
    CHECK(u2 - u1 == wide::uint128(10, 0));
    CHECK(u1 * u2 == wide::uint128(200, 0));
    CHECK((u1 < u2));

    wide::int128 i1(-10);
    wide::int128 i2(20);
    CHECK(i1 + i2 == wide::int128(10));
    CHECK(i1 - i2 == wide::int128(-30));
    CHECK(i1 * i2 == wide::int128(-200));
    CHECK((i1 < i2));

    wide::uint256 w1(100);
    wide::uint256 w2(200);
    CHECK(w1 + w2 == wide::uint256(300));
    CHECK(w2 - w1 == wide::uint256(100));
    CHECK(w1 * w2 == wide::uint256(20000));
    CHECK((w1 < w2));

    wide::int256 si1(-100);
    wide::int256 si2(200);
    CHECK(si1 + si2 == wide::int256(100));
    CHECK(si1 - si2 == wide::int256(-300));
    CHECK(si1 * si2 == wide::int256(-20000));
    CHECK((si1 < si2));
}

void test_differential_backend() {
    std::mt19937_64 rng(999);
    for (int i = 0; i < 10000; ++i) {
        std::uint64_t a = rng();
        std::uint64_t b = rng();

        // Native vs portable 64x64 mul
        std::uint64_t h_nat, l_nat;
        mul64x64(a, b, h_nat, l_nat);

        // Portable formula
        std::uint64_t u0 = a & 0xFFFF'FFFFULL;
        std::uint64_t u1 = a >> 32;
        std::uint64_t v0 = b & 0xFFFF'FFFFULL;
        std::uint64_t v1 = b >> 32;

        std::uint64_t w0 = u0 * v0;
        std::uint64_t t = u1 * v0 + (w0 >> 32);
        std::uint64_t w1 = t & 0xFFFF'FFFFULL;
        std::uint64_t w2 = t >> 32;
        w1 += u0 * v1;
        w2 += (w1 >> 32);
        w1 &= 0xFFFF'FFFFULL;

        std::uint64_t h_port = u1 * v1 + w2;
        std::uint64_t l_port = (w1 << 32) | (w0 & 0xFFFF'FFFFULL);

        CHECK(h_nat == h_port);
        CHECK(l_nat == l_port);

        // 128 by 64 division
        if (b != 0) {
            std::uint64_t hi = a % b; // ensure hi < d
            std::uint64_t lo = rng();
            std::uint64_t r_nat, r_port;
            std::uint64_t q_nat = div128by64(hi, lo, b, r_nat);

            // Verify q_nat * b + r_nat == {hi, lo}
            std::uint64_t q_hi, q_lo;
            mul64x64(q_nat, b, q_hi, q_lo);
            q_lo += r_nat;
            if (q_lo < r_nat) q_hi += 1;
            CHECK(q_hi == hi);
            CHECK(q_lo == lo);
            CHECK(r_nat < b);
        }
    }
}

int main() {
    test_wide_basics();
    test_differential_backend();
    std::printf("test_wide passed (%lu checks)\n", checks);
    return 0;
}
