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

// Two ordinary 64-bit storage limbs keep alignment at eight bytes. Arithmetic
// still uses native __int128 values returned by raw(); these casts/shifts become
// register moves, not a software integer representation or a packed object.
class FP128 final {
public:
    constexpr FP128() noexcept = default;
    explicit constexpr FP128(FP64 value) noexcept
        : low_(static_cast<std::uint64_t>(value.raw())), high_(value.raw() < 0 ? UINT64_MAX : 0) {}
    [[nodiscard]] static constexpr FP128 from_raw(i128 raw) noexcept {
        FP128 result;
        result.low_ = static_cast<std::uint64_t>(raw);
        result.high_ = static_cast<std::uint64_t>(static_cast<u128>(raw) >> 64);
        return result;
    }
    [[nodiscard]] constexpr i128 raw() const noexcept {
        return static_cast<i128>((u128{high_} << 64) | low_);
    }
    constexpr bool operator==(const FP128&) const noexcept = default;
    constexpr std::strong_ordering operator<=>(const FP128& other) const noexcept {
        return raw() <=> other.raw();
    }
private:
    std::uint64_t low_{};
    std::uint64_t high_{};
};
static_assert(sizeof(FP64) == 8 && alignof(FP64) == 8);
static_assert(sizeof(FP128) == 16 && alignof(FP128) == 8);

inline constexpr FP64 fp64_min = FP64::from_raw(INT64_MIN);
inline constexpr FP64 fp64_max = FP64::from_raw(INT64_MAX);
inline constexpr FP128 fp128_min = FP128::from_raw(i128_min);
inline constexpr FP128 fp128_max = FP128::from_raw(i128_max);
} // inline namespace
} // namespace fixedwide
