#pragma once
#include <cstdint>
#include <type_traits>

// Checked add/sub for the narrow widths, used by the public arithmetic header.
//
// GCC and Clang have builtins that lower to one instruction plus a flag test.
// MSVC does not, and the raw builtins in the public header were the reason the
// library could not be compiled there at all. The portable branch is a plain
// signed-overflow test written without signed overflow, so it is correct
// everywhere; it is only reached on compilers without the builtins, and
// FIXEDWIDE_FORCE_PORTABLE selects it so the existing portable build and the
// overflow differential test exercise it on every CI run.
namespace fixedwide::detail {

template<typename T>
[[nodiscard]] constexpr bool add_overflow(T a, T b, T* out) noexcept {
    static_assert(std::is_signed_v<T> && std::is_integral_v<T>);
#if (defined(__GNUC__) || defined(__clang__)) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    return __builtin_add_overflow(a, b, out);
#else
    using U = std::make_unsigned_t<T>;
    const U sum = static_cast<U>(static_cast<U>(a) + static_cast<U>(b));
    *out = static_cast<T>(sum);
    // Overflow exactly when both addends share a sign that the result does not.
    return ((static_cast<U>(a) ^ sum) & (static_cast<U>(b) ^ sum)) >> (sizeof(T) * 8 - 1);
#endif
}

template<typename T>
[[nodiscard]] constexpr bool sub_overflow(T a, T b, T* out) noexcept {
    static_assert(std::is_signed_v<T> && std::is_integral_v<T>);
#if (defined(__GNUC__) || defined(__clang__)) && !defined(FIXEDWIDE_FORCE_PORTABLE)
    return __builtin_sub_overflow(a, b, out);
#else
    using U = std::make_unsigned_t<T>;
    const U diff = static_cast<U>(static_cast<U>(a) - static_cast<U>(b));
    *out = static_cast<T>(diff);
    // Overflow exactly when the operands differ in sign and the result takes
    // the subtrahend's.
    return ((static_cast<U>(a) ^ static_cast<U>(b)) & (static_cast<U>(a) ^ diff)) >> (sizeof(T) * 8 - 1);
#endif
}

} // namespace fixedwide::detail
