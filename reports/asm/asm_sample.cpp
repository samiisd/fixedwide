#include <fixedwide/all.hpp>

using namespace fixedwide;

auto asm_mul64_nearest_even(Fixed64<12> a, Fixed64<12> b) noexcept {
    return mul(a, b, Rounding::nearest_even);
}

auto asm_div64_nearest_even(Fixed64<12> a, Fixed64<12> b) noexcept {
    return div(a, b, Rounding::nearest_even);
}

auto asm_mul_div64_nearest_even(Fixed64<12> a, Fixed64<12> b, Fixed64<12> c) noexcept {
    return mul_div(a, b, c, Rounding::nearest_even);
}

auto asm_quantize64_nearest_even(Fixed64<12> a) noexcept {
    return quantize(a, 4, Rounding::nearest_even);
}

auto asm_mul128_nearest_even(Fixed128<12> a, Fixed128<12> b) noexcept {
    return mul(a, b, Rounding::nearest_even);
}

auto asm_div128_nearest_even(Fixed128<12> a, Fixed128<12> b) noexcept {
    return div(a, b, Rounding::nearest_even);
}
