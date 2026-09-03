#pragma once
#include <fixedwide/bigint.hpp>

namespace baseline {
using old_i128 = fixedwide::i128;
bool legacy_mul(old_i128, old_i128, old_i128&) noexcept;
bool legacy_div(old_i128, old_i128, old_i128&) noexcept;
bool legacy_mul_div(old_i128, old_i128, old_i128, old_i128&) noexcept;
std::expected<fixedwide::UnsignedDivision, fixedwide::ArithmeticError>
native_divmod(fixedwide::u256, fixedwide::u128) noexcept;
} // namespace baseline
