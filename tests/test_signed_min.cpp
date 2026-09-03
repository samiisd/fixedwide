#include "check.hpp"
#include <fixedwide/bigint.hpp>
#include <fixedwide/all.hpp>
#include <limits>
#include <iostream>

using namespace fixedwide;

void test_signed_min_64() {
    using F = Fixed64<0>;
    constexpr auto min_val = F::min();
    constexpr auto one = *from_integer<F>(1);
    constexpr auto neg_one = *from_integer<F>(-1);
    constexpr auto zero = F{};

    constexpr Rounding modes[] = {
        Rounding::toward_zero,
        Rounding::floor,
        Rounding::ceil,
        Rounding::nearest_away,
        Rounding::nearest_even,
        Rounding::exact
    };

    for (auto mode : modes) {
        // min / -1 must overflow
        auto res = div(min_val, neg_one, mode);
        CHECK(!res.has_value() && res.error() == ArithmeticError::overflow);

        // min / 1 == min
        auto res1 = div(min_val, one, mode);
        CHECK(res1.has_value() && res1->raw() == min_val.raw());

        // min / min == 1
        auto res_self = div(min_val, min_val, mode);
        CHECK(res_self.has_value() && res_self->raw() == 1);

        // 0 / min == 0
        auto res_zero = div(zero, min_val, mode);
        CHECK(res_zero.has_value() && res_zero->raw() == 0);

        // 1 / min
        auto res_small = div(one, min_val, mode);
        if (mode == Rounding::exact) {
            CHECK(!res_small.has_value() && res_small.error() == ArithmeticError::inexact);
        } else {
            CHECK(res_small.has_value());
        }

        // -1 / min
        auto res_neg_small = div(neg_one, min_val, mode);
        if (mode == Rounding::exact) {
            CHECK(!res_neg_small.has_value() && res_neg_small.error() == ArithmeticError::inexact);
        } else {
            CHECK(res_neg_small.has_value());
        }

        // mul_div with signed minimum
        auto md_self = mul_div(min_val, one, min_val, mode);
        CHECK(md_self.has_value() && md_self->raw() == 1);

        auto md_zero = mul_div(zero, one, min_val, mode);
        CHECK(md_zero.has_value() && md_zero->raw() == 0);

        auto md_over = mul_div(min_val, one, neg_one, mode);
        CHECK(!md_over.has_value() && md_over.error() == ArithmeticError::overflow);
    }
}

void test_signed_min_128() {
    using F = Fixed128<0>;
    constexpr auto min_val = F::min();
    constexpr auto one = *from_integer<F>(1);
    constexpr auto neg_one = *from_integer<F>(-1);
    constexpr auto zero = F{};

    constexpr Rounding modes[] = {
        Rounding::toward_zero,
        Rounding::floor,
        Rounding::ceil,
        Rounding::nearest_away,
        Rounding::nearest_even,
        Rounding::exact
    };

    for (auto mode : modes) {
        // min / -1 must overflow
        auto res = div(min_val, neg_one, mode);
        CHECK(!res.has_value() && res.error() == ArithmeticError::overflow);

        // min / 1 == min
        auto res1 = div(min_val, one, mode);
        CHECK(res1.has_value() && res1->raw() == min_val.raw());

        // min / min == 1
        auto res_self = div(min_val, min_val, mode);
        CHECK(res_self.has_value() && res_self->raw() == 1);

        // 0 / min == 0
        auto res_zero = div(zero, min_val, mode);
        CHECK(res_zero.has_value() && res_zero->raw() == 0);

        // 1 / min
        auto res_small = div(one, min_val, mode);
        if (mode == Rounding::exact) {
            CHECK(!res_small.has_value() && res_small.error() == ArithmeticError::inexact);
        } else {
            CHECK(res_small.has_value());
        }

        // -1 / min
        auto res_neg_small = div(neg_one, min_val, mode);
        if (mode == Rounding::exact) {
            CHECK(!res_neg_small.has_value() && res_neg_small.error() == ArithmeticError::inexact);
        } else {
            CHECK(res_neg_small.has_value());
        }

        // mul_div with signed minimum
        auto md_self = mul_div(min_val, one, min_val, mode);
        CHECK(md_self.has_value() && md_self->raw() == 1);

        auto md_zero = mul_div(zero, one, min_val, mode);
        CHECK(md_zero.has_value() && md_zero->raw() == 0);

        auto md_over = mul_div(min_val, one, neg_one, mode);
        CHECK(!md_over.has_value() && md_over.error() == ArithmeticError::overflow);
    }

    // Scaled Fixed128
    using FS = Fixed128<12>;
    constexpr auto min_fs = FS::min();
    constexpr auto one_fs = *from_integer<FS>(1);
    constexpr auto zero_fs = FS{};

    for (auto mode : modes) {
        // min / min == scale
        auto res_self = div(min_fs, min_fs, mode);
        CHECK(res_self.has_value() && res_self->raw() == FS::scale());

        // 0 / min == 0
        auto res_zero = div(zero_fs, min_fs, mode);
        CHECK(res_zero.has_value() && res_zero->raw() == 0);

        // mul_div with min
        auto md_self = mul_div(min_fs, one_fs, min_fs, mode);
        CHECK(md_self.has_value() && md_self->raw() == one_fs.raw());
    }
}

void test_signed_min_256() {
    using F = Fixed256<0>;
    constexpr auto min_val = F::min();
    constexpr auto one = *from_integer<F>(1);
    constexpr auto neg_one = *from_integer<F>(-1);
    constexpr auto zero = F{};

    constexpr Rounding modes[] = {
        Rounding::toward_zero,
        Rounding::floor,
        Rounding::ceil,
        Rounding::nearest_away,
        Rounding::nearest_even,
        Rounding::exact
    };

    for (auto mode : modes) {
        // min / -1 must overflow
        auto res = div(min_val, neg_one, mode);
        CHECK(!res.has_value() && res.error() == ArithmeticError::overflow);

        // min / 1 == min
        auto res1 = div(min_val, one, mode);
        CHECK(res1.has_value() && res1->raw() == min_val.raw());

        // min / min == 1
        auto res_self = div(min_val, min_val, mode);
        CHECK(res_self.has_value() && res_self->raw() == 1);

        // 0 / min == 0
        auto res_zero = div(zero, min_val, mode);
        CHECK(res_zero.has_value() && res_zero->raw() == 0);

        // 1 / min
        auto res_small = div(one, min_val, mode);
        if (mode == Rounding::exact) {
            CHECK(!res_small.has_value() && res_small.error() == ArithmeticError::inexact);
        } else {
            CHECK(res_small.has_value());
        }

        // -1 / min
        auto res_neg_small = div(neg_one, min_val, mode);
        if (mode == Rounding::exact) {
            CHECK(!res_neg_small.has_value() && res_neg_small.error() == ArithmeticError::inexact);
        } else {
            CHECK(res_neg_small.has_value());
        }

        // mul_div with signed minimum
        auto md_self = mul_div(min_val, one, min_val, mode);
        CHECK(md_self.has_value() && md_self->raw() == 1);

        auto md_zero = mul_div(zero, one, min_val, mode);
        CHECK(md_zero.has_value() && md_zero->raw() == 0);
    }
}

void test_bigint_primitives() {
    constexpr Rounding modes[] = {
        Rounding::toward_zero,
        Rounding::floor,
        Rounding::ceil,
        Rounding::nearest_away,
        Rounding::nearest_even,
        Rounding::exact
    };

    wide::int128 min128 = wide::int128::min();
    wide::int128 one128(1ULL);
    wide::int128 neg_one128(-1);

    wide::int256 min256 = wide::int256::min();

    for (auto mode : modes) {
        // bigint::divide_to_i128
        wide::int256 n256(min128.low, min128.high, (min128.high >> 63) ? ~0ULL : 0ULL, (min128.high >> 63) ? ~0ULL : 0ULL);
        auto d_res = divide_to_i128(n256, min128, mode);
        CHECK(d_res.has_value() && *d_res == one128);

        // bigint::mul_div
        auto md_res = mul_div(min128, one128, min128, mode);
        CHECK(md_res.has_value() && *md_res == one128);

        auto md_over = mul_div(min128, one128, neg_one128, mode);
        CHECK(!md_over.has_value() && md_over.error() == ArithmeticError::overflow);
    }

    // divmod
    auto dm = divmod(min256, min128);
    CHECK(dm.has_value());
}

int main() {
    test_signed_min_64();
    test_signed_min_128();
    test_signed_min_256();
    test_bigint_primitives();
    std::cout << "All signed minimum tests passed! Total checks: " << checks << std::endl;
    return 0;
}
