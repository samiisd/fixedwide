// Differential test for the checked add/sub used by the public arithmetic
// header: the portable branch against the compiler builtins. Exhaustive at 8
// bits, strided at 16, and over every interesting boundary at 32 and 64.
#include <fixedwide/detail/overflow.hpp>
#include "check.hpp"
#include <cstdint>
#include <cstdio>

namespace {
namespace portable {
template<typename T> constexpr bool add_overflow(T a, T b, T* out) noexcept {
    using U = std::make_unsigned_t<T>;
    const U sum = static_cast<U>(static_cast<U>(a) + static_cast<U>(b));
    *out = static_cast<T>(sum);
    return ((static_cast<U>(a) ^ sum) & (static_cast<U>(b) ^ sum)) >> (sizeof(T) * 8 - 1);
}
template<typename T> constexpr bool sub_overflow(T a, T b, T* out) noexcept {
    using U = std::make_unsigned_t<T>;
    const U diff = static_cast<U>(static_cast<U>(a) - static_cast<U>(b));
    *out = static_cast<T>(diff);
    return ((static_cast<U>(a) ^ static_cast<U>(b)) & (static_cast<U>(a) ^ diff)) >> (sizeof(T) * 8 - 1);
}
} // namespace portable

template<typename T> void compare(T a, T b) {
    T mine{}, reference{};
    const bool mo = fixedwide::detail::add_overflow(a, b, &mine);
    const bool ro = portable::add_overflow(a, b, &reference);
    CHECK(mo == ro);
    if (!ro) CHECK(mine == reference);
    const bool ms = fixedwide::detail::sub_overflow(a, b, &mine);
    const bool rs = portable::sub_overflow(a, b, &reference);
    CHECK(ms == rs);
    if (!rs) CHECK(mine == reference);
}
} // namespace

int main() {
    for (int a = -128; a < 128; ++a)
        for (int b = -128; b < 128; ++b)
            compare<std::int8_t>(static_cast<std::int8_t>(a), static_cast<std::int8_t>(b));

    for (int a = -32768; a < 32768; a += 7)
        for (int b = -32768; b < 32768; b += 11)
            compare<std::int16_t>(static_cast<std::int16_t>(a), static_cast<std::int16_t>(b));

    const std::int64_t edges[] = {INT64_MIN, INT64_MIN + 1, -3, -1, 0, 1, 3,
                                  INT64_MAX - 1, INT64_MAX, INT32_MIN, INT32_MAX,
                                  std::int64_t{1} << 62, -(std::int64_t{1} << 62)};
    for (auto a : edges)
        for (auto b : edges) {
            compare<std::int64_t>(a, b);
            compare<std::int32_t>(static_cast<std::int32_t>(a), static_cast<std::int32_t>(b));
        }

    // Both branches must be usable in constant expressions.
    static_assert([] { std::int32_t out = 0; return !fixedwide::detail::add_overflow(2, 3, &out) && out == 5; }());
    static_assert([] { std::int8_t out = 0; return fixedwide::detail::add_overflow<std::int8_t>(100, 100, &out); }());

    std::printf("test_overflow passed (%lu checks)\n", checks);
    return 0;
}
