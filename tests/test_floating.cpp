#include "check.hpp"
#include <fixedwide/all.hpp>
#include <limits>
#include <cmath>

using namespace fixedwide;

void test_floating_conversions() {
    using F = Fixed64<4>;
    double d = 123.4567;
    auto f_res = from_float<F>(d);
    CHECK(f_res.has_value());
    CHECK(f_res->raw() == 1234567LL);

    double back = to_float<double>(*f_res);
    CHECK(std::abs(back - d) < 1e-6);

    // Float overload
    float flt = 12.5f;
    auto f_flt = from_float<Fixed32<2>>(flt);
    CHECK(f_flt.has_value() && f_flt->raw() == 1250);

    // NaN / Inf error check
    CHECK(!from_float<F>(std::numeric_limits<double>::quiet_NaN()));
    CHECK(!from_float<F>(std::numeric_limits<double>::infinity()));
    CHECK(!from_float<F>(-std::numeric_limits<double>::infinity()));
    auto nan_err = from_float<F>(std::numeric_limits<double>::quiet_NaN());
    CHECK(nan_err.error() == ArithmeticError::invalid_value);

    // Overflow check
    auto of_err = from_float<Fixed8<1>>(1000.0);
    CHECK(!of_err.has_value() && of_err.error() == ArithmeticError::overflow);
}

int main() {
    test_floating_conversions();
    std::printf("test_floating passed (%lu checks)\n", checks);
    return 0;
}
