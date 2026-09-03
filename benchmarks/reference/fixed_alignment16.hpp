#pragma once
#include <fixedwide/config.hpp>
#include <fixedwide/types.hpp>
#include <compare>

namespace fixedwide {
inline namespace FIXEDWIDE_SCALE_NAMESPACE {
inline constexpr unsigned fractional_digits = FIXEDWIDE_DECIMALS;
inline constexpr std::int64_t scale = [] {
    std::int64_t value = 1;
    for (unsigned i = 0; i < fractional_digits; ++i) value *= 10;
    return value;
}();

class FP64 final {
public:
    constexpr FP64() noexcept = default;
    [[nodiscard]] static constexpr FP64 from_raw(std::int64_t raw) noexcept {
        FP64 result;
        result.raw_ = raw;
        return result;
    }
    [[nodiscard]] constexpr std::int64_t raw() const noexcept { return raw_; }
    auto operator<=>(const FP64&) const = default;
private:
    std::int64_t raw_{};
};

class FP128 final {
public:
    constexpr FP128() noexcept = default;
    explicit constexpr FP128(FP64 value) noexcept : raw_(value.raw()) {}
    [[nodiscard]] static constexpr FP128 from_raw(i128 raw) noexcept {
        FP128 result;
        result.raw_ = raw;
        return result;
    }
    [[nodiscard]] constexpr i128 raw() const noexcept { return raw_; }
    auto operator<=>(const FP128&) const = default;
private:
    i128 raw_{};
};

inline constexpr FP64 fp64_min = FP64::from_raw(INT64_MIN);
inline constexpr FP64 fp64_max = FP64::from_raw(INT64_MAX);
inline constexpr FP128 fp128_min = FP128::from_raw(i128_min);
inline constexpr FP128 fp128_max = FP128::from_raw(i128_max);
} // inline namespace
} // namespace fixedwide
