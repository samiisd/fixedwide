#include "baselines.hpp"
#include <fixedwide/bigint.hpp>
#include <fixedwide/fixed.hpp>

namespace baseline {
namespace {
#if defined(__clang__)
using old_i128_raw = _BitInt(128);
using old_i256 = _BitInt(256);
constexpr old_i128_raw maximum = static_cast<old_i128_raw>((static_cast<unsigned _BitInt(128)>(1) << 127) - 1);
constexpr old_i128_raw minimum = -maximum - 1;
#else
using old_i128_raw = __int128;
constexpr old_i128_raw maximum = static_cast<old_i128_raw>((static_cast<unsigned __int128>(1) << 127) - 1);
constexpr old_i128_raw minimum = -maximum - 1;
#endif

inline old_i128_raw to_raw(fixedwide::i128 v) noexcept {
    return (static_cast<old_i128_raw>(v.high) << 64) | v.low;
}

inline fixedwide::i128 from_raw(old_i128_raw v) noexcept {
    return fixedwide::i128(static_cast<std::uint64_t>(v),
                           static_cast<std::uint64_t>(static_cast<unsigned __int128>(v) >> 64));
}

inline bool old_fast_mul_div(old_i128_raw a, old_i128_raw b, old_i128_raw scale, old_i128_raw& out) noexcept {
    if (scale == 0) return false;
    old_i128_raw product;
    if (!__builtin_mul_overflow(a, b, &product)) {
        if (product == minimum && scale == -1) return false;
        out = product / scale;
        return true;
    }
#if defined(__clang__)
    const old_i256 wide_product = static_cast<old_i256>(a) * static_cast<old_i256>(b);
    const old_i256 result = wide_product / static_cast<old_i256>(scale);
    if (result > static_cast<old_i256>(maximum) || result < static_cast<old_i256>(minimum)) return false;
    out = static_cast<old_i128_raw>(result);
    return true;
#else
    auto res = fixedwide::mul_div(from_raw(a), from_raw(b), from_raw(scale), fixedwide::Rounding::toward_zero);
    if (!res) return false;
    out = to_raw(*res);
    return true;
#endif
}
} // namespace

bool legacy_mul(old_i128 a, old_i128 b, old_i128& out) noexcept {
    old_i128_raw res;
    bool ok = old_fast_mul_div(to_raw(a), to_raw(b), fixedwide::scale, res);
    if (ok) out = from_raw(res);
    return ok;
}

bool legacy_div(old_i128 a, old_i128 b, old_i128& out) noexcept {
    old_i128_raw res;
    bool ok = old_fast_mul_div(to_raw(a), fixedwide::scale, to_raw(b), res);
    if (ok) out = from_raw(res);
    return ok;
}

bool legacy_mul_div(old_i128 a, old_i128 b, old_i128 d, old_i128& out) noexcept {
    old_i128_raw res;
    bool ok = old_fast_mul_div(to_raw(a), to_raw(b), to_raw(d), res);
    if (ok) out = from_raw(res);
    return ok;
}

std::expected<fixedwide::UnsignedDivision, fixedwide::ArithmeticError> native_divmod(fixedwide::u256 n,
                                                                                     fixedwide::u128 d) noexcept {
    if (d == 0) return std::unexpected(fixedwide::ArithmeticError::division_by_zero);
    return fixedwide::UnsignedDivision{n / static_cast<fixedwide::u256>(d),
                                       static_cast<fixedwide::u128>(n % static_cast<fixedwide::u256>(d))};
}
} // namespace baseline
