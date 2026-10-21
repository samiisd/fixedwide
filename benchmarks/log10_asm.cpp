#include <fixedwide/log10.hpp>
#include <cmath>

int log10_floor64(fixedwide::Fixed64<8> value) noexcept {
    return fixedwide::log10_floor(value).value_or(-1000);
}
int log10_floor128(fixedwide::Fixed128<12> value) noexcept {
    return fixedwide::log10_floor(value).value_or(-1000);
}
int log10_floor256(fixedwide::Fixed256<18> value) noexcept {
    return fixedwide::log10_floor(value).value_or(-1000);
}
double log10_double(double value) noexcept {
    return std::log10(value);
}
long double log10_long_double(long double value) noexcept {
    return std::log10(value);
}
