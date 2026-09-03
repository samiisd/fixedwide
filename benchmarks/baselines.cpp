#include "baselines.hpp"
#include <fixedwide/fixed.hpp>

namespace baseline {
namespace {
using old_i256 = _BitInt(256);
constexpr old_i128 maximum = static_cast<old_i128>((static_cast<unsigned _BitInt(128)>(1) << 127) - 1);
constexpr old_i128 minimum = -maximum - 1;
inline bool old_fast_mul_div(old_i128 a, old_i128 b, old_i128 scale, old_i128& out) noexcept {
    if (scale == 0) return false;
    old_i128 product;
    if (!__builtin_mul_overflow(a, b, &product)) {
        if (product == minimum && scale == -1) return false;
        out = product / scale;
        return true;
    }
    const old_i256 wide_product = static_cast<old_i256>(a) * static_cast<old_i256>(b);
    const old_i256 result = wide_product / static_cast<old_i256>(scale);
    if (result > static_cast<old_i256>(maximum) || result < static_cast<old_i256>(minimum)) return false;
    out = static_cast<old_i128>(result);
    return true;
}
}
bool legacy_mul(old_i128 a, old_i128 b, old_i128& out) noexcept { return old_fast_mul_div(a, b, fixedwide::scale, out); }
bool legacy_div(old_i128 a, old_i128 b, old_i128& out) noexcept { return old_fast_mul_div(a, fixedwide::scale, b, out); }
bool legacy_mul_div(old_i128 a, old_i128 b, old_i128 d, old_i128& out) noexcept { return old_fast_mul_div(a, b, d, out); }
std::expected<fixedwide::UnsignedDivision, fixedwide::ArithmeticError>
native_divmod(fixedwide::u256 n, fixedwide::u128 d) noexcept {
    if (d == 0) return std::unexpected(fixedwide::ArithmeticError::division_by_zero);
    return fixedwide::UnsignedDivision{n / static_cast<fixedwide::u256>(d),
                                     static_cast<fixedwide::u128>(n % static_cast<fixedwide::u256>(d))};
}
} // namespace baseline
