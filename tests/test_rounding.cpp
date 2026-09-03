#include "check.hpp"
#include <fixedwide/arithmetic.hpp>
#include <fixedwide/rounding.hpp>
#include "src/detail.hpp"

using namespace fixedwide;

void test_rounding_primitives() {
    using detail::round_magnitude;

    // Test nearest_even tie logic on exact halves: remainder == divisor / 2
    // Even quotient -> stays even (no increment)
    auto r1 = round_magnitude(2ULL, 5ULL, 10ULL, false, Rounding::nearest_even, 100ULL);
    CHECK(r1.has_value() && *r1 == 2ULL);

    // Odd quotient -> increments to even
    auto r2 = round_magnitude(3ULL, 5ULL, 10ULL, false, Rounding::nearest_even, 100ULL);
    CHECK(r2.has_value() && *r2 == 4ULL);

    // Negative even quotient -> stays even
    auto r3 = round_magnitude(2ULL, 5ULL, 10ULL, true, Rounding::nearest_even, 100ULL);
    CHECK(r3.has_value() && *r3 == 2ULL);

    // Negative odd quotient -> increments magnitude to even
    auto r4 = round_magnitude(3ULL, 5ULL, 10ULL, true, Rounding::nearest_even, 100ULL);
    CHECK(r4.has_value() && *r4 == 4ULL);

    // Test nearest_away tie logic: always round away from zero
    auto ra1 = round_magnitude(2ULL, 5ULL, 10ULL, false, Rounding::nearest_away, 100ULL);
    CHECK(ra1.has_value() && *ra1 == 3ULL);
    auto ra2 = round_magnitude(3ULL, 5ULL, 10ULL, false, Rounding::nearest_away, 100ULL);
    CHECK(ra2.has_value() && *ra2 == 4ULL);

    // Test floor and ceil
    // Positive
    auto rf_pos = round_magnitude(2ULL, 1ULL, 10ULL, false, Rounding::floor, 100ULL);
    CHECK(rf_pos.has_value() && *rf_pos == 2ULL);
    auto rc_pos = round_magnitude(2ULL, 1ULL, 10ULL, false, Rounding::ceil, 100ULL);
    CHECK(rc_pos.has_value() && *rc_pos == 3ULL);

    // Negative: floor moves away from zero (-2.1 -> -3.0), ceil moves toward zero (-2.1 -> -2.0)
    auto rf_neg = round_magnitude(2ULL, 1ULL, 10ULL, true, Rounding::floor, 100ULL);
    CHECK(rf_neg.has_value() && *rf_neg == 3ULL);
    auto rc_neg = round_magnitude(2ULL, 1ULL, 10ULL, true, Rounding::ceil, 100ULL);
    CHECK(rc_neg.has_value() && *rc_neg == 2ULL);

    // Test toward_zero
    auto rtz_pos = round_magnitude(2ULL, 9ULL, 10ULL, false, Rounding::toward_zero, 100ULL);
    CHECK(rtz_pos.has_value() && *rtz_pos == 2ULL);
    auto rtz_neg = round_magnitude(2ULL, 9ULL, 10ULL, true, Rounding::toward_zero, 100ULL);
    CHECK(rtz_neg.has_value() && *rtz_neg == 2ULL);

    // Test exact: fails if remainder != 0
    auto re_ok = round_magnitude(2ULL, 0ULL, 10ULL, false, Rounding::exact, 100ULL);
    CHECK(re_ok.has_value() && *re_ok == 2ULL);
    auto re_fail = round_magnitude(2ULL, 1ULL, 10ULL, false, Rounding::exact, 100ULL);
    CHECK(!re_fail.has_value() && re_fail.error() == ArithmeticError::inexact);
}

void test_fixed64_rounding() {
    using F = Fixed64<1>; // scale 10
    auto one = F::from_raw(10); // 1.0
    auto four = F::from_raw(40); // 4.0

    // 1.0 / 4.0 = 0.25 -> 0.2 with scale 10 (even quotient: 2)
    auto d_even = div(one, four, Rounding::nearest_even);
    CHECK(d_even.has_value() && d_even->raw() == 2); // 0.2 (even)

    auto three = F::from_raw(30); // 3.0
    // 3.0 / 4.0 = 0.75 -> 0.8 with scale 10 (odd quotient: 7 -> rounds to 8)
    auto d_odd = div(three, four, Rounding::nearest_even);
    CHECK(d_odd.has_value() && d_odd->raw() == 8); // 0.8 (even)

    // With nearest_away: 0.25 -> 0.3
    auto d_away = div(one, four, Rounding::nearest_away);
    CHECK(d_away.has_value() && d_away->raw() == 3);

    // Negative: -0.25 -> -0.2 with nearest_even
    auto neg_one = F::from_raw(-10);
    auto d_neg_even = div(neg_one, four, Rounding::nearest_even);
    CHECK(d_neg_even.has_value() && d_neg_even->raw() == -2);

    // Negative: -0.25 -> -0.3 with nearest_away
    auto d_neg_away = div(neg_one, four, Rounding::nearest_away);
    CHECK(d_neg_away.has_value() && d_neg_away->raw() == -3);
}

int main() {
    test_rounding_primitives();
    test_fixed64_rounding();
    std::printf("test_rounding passed (%lu checks)\n", checks);
    return 0;
}
