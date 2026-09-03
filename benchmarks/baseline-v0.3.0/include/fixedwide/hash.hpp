#pragma once
#include <fixedwide/fixed.hpp>
#include <functional>

namespace fixedwide {
inline namespace FIXEDWIDE_SCALE_NAMESPACE {
[[nodiscard]] constexpr std::size_t hash_value(FP64 value) noexcept {
    return static_cast<std::size_t>(value.raw());
}
[[nodiscard]] constexpr std::size_t hash_value(FP128 value) noexcept {
    const u128 bits = static_cast<u128>(value.raw());
    return static_cast<std::size_t>(bits) ^ static_cast<std::size_t>(bits >> 64);
}
} // inline namespace
} // namespace fixedwide
namespace std {
// Specializing hash for a program-defined type is permitted. Native integer traits
// and overloads of std::to_string/std::abs are deliberately NOT added.
template<> struct hash<fixedwide::FP64> {
    constexpr size_t operator()(fixedwide::FP64 value) const noexcept { return fixedwide::hash_value(value); }
};
template<> struct hash<fixedwide::FP128> {
    constexpr size_t operator()(fixedwide::FP128 value) const noexcept { return fixedwide::hash_value(value); }
};
}
