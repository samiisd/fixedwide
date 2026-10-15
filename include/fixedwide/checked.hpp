#pragma once

/// \file
/// Accumulated checked arithmetic: evaluate a formula, then test once.
/// Every member calls the existing checked operation, including after an error.
/// A failed step returns a value-initialized placeholder; the first error sticks.
/// Do not publish or use intermediate values unless the context succeeds.

#include <fixedwide/arithmetic.hpp>
#include <fixedwide/mixed.hpp>
#include <fixedwide/floating.hpp>
#include <fixedwide/bigint.hpp>
#include <expected>
#include <optional>
#include <type_traits>
#include <utility>

// Keep the expected producer/consumer boundary visible to the optimizer.
// Without this, Clang can if-convert each failed add/sub into conditional-zero
// moves and keep every overflow flag live throughout an otherwise cheap chain.
#if defined(__GNUC__) || defined(__clang__)
#define FIXEDWIDE_CHECKED_INLINE [[gnu::always_inline]]
#else
#define FIXEDWIDE_CHECKED_INLINE
#endif

namespace fixedwide {

/// A local, copyable first-error accumulator, not a value/storage wrapper.
/// No reset, exceptions, allocation or global state. Copies are independent.
/// Keep fallible steps in separate statements: argument evaluation order does
/// not define the first error in nested expressions using the same context.
class Checked {
public:
    constexpr Checked() noexcept = default;

    /// True until the first failure. Querying never resets the context.
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return !error_; }

    /// Empty on success; otherwise the first ArithmeticError encountered.
    [[nodiscard]] constexpr std::optional<ArithmeticError> error() const noexcept { return error_; }

    /// Absorb a checked result. Success returns its value even after an earlier
    /// failure; failure returns T{} and records the error only if it is first.
    /// T must support nonthrowing default construction and the selected transfer.
    template<class T>
        requires(std::is_nothrow_default_constructible_v<T> && std::is_nothrow_move_constructible_v<T>)
    [[nodiscard]] FIXEDWIDE_CHECKED_INLINE constexpr T take(std::expected<T, ArithmeticError>&& value) noexcept {
        if (!value) [[unlikely]] {
            record(value.error());
            return T{};
        }
        return std::move(*value);
    }

    /// Copy an lvalue result without consuming it.
    template<class T>
        requires(std::is_nothrow_default_constructible_v<T> && std::is_nothrow_copy_constructible_v<T>)
    [[nodiscard]] FIXEDWIDE_CHECKED_INLINE constexpr T take(const std::expected<T, ArithmeticError>& value) noexcept {
        if (!value) [[unlikely]] {
            record(value.error());
            return T{};
        }
        return *value;
    }

    /// Absorb an operation that reports status but has no value.
    constexpr void take(std::expected<void, ArithmeticError> value) noexcept {
        if (!value) [[unlikely]]
            record(value.error());
    }

    /// Publish a value only if every accumulated operation succeeded.
    /// The selected copy/move construction must be nonthrowing. Does not reset.
    template<class T>
        requires(std::is_nothrow_constructible_v<std::remove_cvref_t<T>, T&&>)
    [[nodiscard]] constexpr std::expected<std::remove_cvref_t<T>, ArithmeticError> result(T&& value) const noexcept {
        if (error_.has_value()) [[unlikely]]
            return std::unexpected(*error_);
        return std::expected<std::remove_cvref_t<T>, ArithmeticError>(std::in_place, std::forward<T>(value));
    }

    /// Publish the accumulated status without a value.
    [[nodiscard]] constexpr std::expected<void, ArithmeticError> result() const noexcept {
        if (error_.has_value()) [[unlikely]]
            return std::unexpected(*error_);
        return {};
    }

    /// Accumulated `fixedwide::add` with the same operands and rounding.
    template<std::size_t Bits, unsigned D>
    [[nodiscard]] FIXEDWIDE_CHECKED_INLINE constexpr basic_fixed<Bits, D> add(basic_fixed<Bits, D> a,
                                                                              basic_fixed<Bits, D> b) noexcept {
        return take(fixedwide::add(a, b));
    }

    /// Accumulated `fixedwide::sub` with the same operands and rounding.
    template<std::size_t Bits, unsigned D>
    [[nodiscard]] FIXEDWIDE_CHECKED_INLINE constexpr basic_fixed<Bits, D> sub(basic_fixed<Bits, D> a,
                                                                              basic_fixed<Bits, D> b) noexcept {
        return take(fixedwide::sub(a, b));
    }

    /// Accumulated `fixedwide::negate` with the same operands and rounding.
    template<std::size_t Bits, unsigned D>
    [[nodiscard]] constexpr basic_fixed<Bits, D> negate(basic_fixed<Bits, D> a) noexcept {
        return take(fixedwide::negate(a));
    }

    /// Accumulated `fixedwide::abs` with the same operands and rounding.
    template<std::size_t Bits, unsigned D>
    [[nodiscard]] constexpr basic_fixed<Bits, D> abs(basic_fixed<Bits, D> a) noexcept {
        return take(fixedwide::abs(a));
    }

    /// Accumulated `fixedwide::mul` with the same operands and rounding.
    template<std::size_t Bits, unsigned D>
    [[nodiscard]] constexpr basic_fixed<Bits, D> mul(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b,
                                                     Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::mul(a, b, rounding));
    }

    /// Accumulated `fixedwide::div` with the same operands and rounding.
    template<std::size_t Bits, unsigned D>
    [[nodiscard]] constexpr basic_fixed<Bits, D> div(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b,
                                                     Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::div(a, b, rounding));
    }

    /// Accumulated `fixedwide::mul_div` with the same operands and rounding.
    template<std::size_t Bits, unsigned D>
    [[nodiscard]] constexpr basic_fixed<Bits, D> mul_div(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b,
                                                         basic_fixed<Bits, D> c,
                                                         Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::mul_div(a, b, c, rounding));
    }

    /// Accumulated `fixedwide::remainder` with the same operands and rounding.
    template<std::size_t Bits, unsigned D>
    [[nodiscard]] constexpr basic_fixed<Bits, D> remainder(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b) noexcept {
        return take(fixedwide::remainder(a, b));
    }

    /// Accumulated `fixedwide::midpoint` with the same operands and rounding.
    template<std::size_t Bits, unsigned D>
    [[nodiscard]] constexpr basic_fixed<Bits, D> midpoint(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b,
                                                          Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::midpoint(a, b, rounding));
    }

    /// Round to a coarser decimal grid, retaining the original type.
    template<std::size_t Bits, unsigned D>
    [[nodiscard]] constexpr basic_fixed<Bits, D> quantize(basic_fixed<Bits, D> a, unsigned decimals,
                                                          Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::quantize(a, decimals, rounding));
    }

    /// Midpoint rounded directly onto a coarser decimal grid.
    template<std::size_t Bits, unsigned D>
    [[nodiscard]] constexpr basic_fixed<Bits, D> midpoint(basic_fixed<Bits, D> a, basic_fixed<Bits, D> b,
                                                          unsigned decimals,
                                                          Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::midpoint(a, b, decimals, rounding));
    }

    /// Checked construction from any integer accepted by the free function.
    template<class Target, class Integer>
        requires requires(Integer value) { fixedwide::from_integer<Target>(value); }
    [[nodiscard]] constexpr Target from_integer(Integer value) noexcept {
        return take(fixedwide::from_integer<Target>(value));
    }

    /// Accumulated `fixedwide::fixed_cast<Dest>`; identical destination and defaults.
    template<class Dest, std::size_t BitsA, unsigned Da>
    [[nodiscard]] Dest fixed_cast(basic_fixed<BitsA, Da> a, Rounding rounding = Rounding::exact) noexcept {
        return take(fixedwide::fixed_cast<Dest>(a, rounding));
    }

    /// Accumulated `fixedwide::add_to<Dest>`; identical destination and defaults.
    template<class Dest, std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db>
    [[nodiscard]] Dest add_to(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b,
                              Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::add_to<Dest>(a, b, rounding));
    }

    /// Accumulated `fixedwide::sub_to<Dest>`; identical destination and defaults.
    template<class Dest, std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db>
    [[nodiscard]] Dest sub_to(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b,
                              Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::sub_to<Dest>(a, b, rounding));
    }

    /// Accumulated `fixedwide::mul_to<Dest>`; identical destination and defaults.
    template<class Dest, std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db>
    [[nodiscard]] Dest mul_to(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b,
                              Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::mul_to<Dest>(a, b, rounding));
    }

    /// Accumulated `fixedwide::div_to<Dest>`; identical destination and defaults.
    template<class Dest, std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db>
    [[nodiscard]] Dest div_to(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b,
                              Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::div_to<Dest>(a, b, rounding));
    }

    /// Accumulated `fixedwide::mul_div_to<Dest>`; identical destination and defaults.
    template<class Dest, std::size_t BitsA, unsigned Da, std::size_t BitsB, unsigned Db, std::size_t BitsC, unsigned Dc>
    [[nodiscard]] Dest mul_div_to(basic_fixed<BitsA, Da> a, basic_fixed<BitsB, Db> b, basic_fixed<BitsC, Dc> c,
                                  Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::mul_div_to<Dest>(a, b, c, rounding));
    }

    /// Binary float ingress; preserves the free function's format/error rules.
    template<class Target, std::floating_point Float>
    [[nodiscard]] Target from_float(Float value, Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::from_float<Target>(value, rounding));
    }

    /// Double-specific convenience, identical to the free function.
    template<class Target>
    [[nodiscard]] Target from_double(double value, Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::from_double<Target>(value, rounding));
    }

    /// Legacy twelve-decimal double ingress aliases.
    [[nodiscard]] FP64 from_double64(double value, Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::from_double64(value, rounding));
    }
    [[nodiscard]] FP128 from_double128(double value, Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::from_double128(value, rounding));
    }

    /// Legacy twelve-decimal widening product.
    [[nodiscard]] FP128 mul_wide(FP64 a, FP64 b, Rounding rounding = Rounding::nearest_even) noexcept {
        return take(fixedwide::mul_wide(a, b, rounding));
    }

    /// Legacy twelve-decimal narrowing conversion.
    [[nodiscard]] constexpr FP64 narrow(FP128 value) noexcept { return take(fixedwide::narrow(value)); }

    /// Unscaled wide-integer division; failure returns zero quotient/remainder.
    [[nodiscard]] UnsignedDivision divmod(u256 numerator, u128 divisor) noexcept {
        return take(fixedwide::divmod(numerator, divisor));
    }
    [[nodiscard]] SignedDivision divmod(i256 numerator, i128 divisor) noexcept {
        return take(fixedwide::divmod(numerator, divisor));
    }

    /// Unscaled division rounded into i128. Rounding is required, as in bigint.hpp.
    [[nodiscard]] i128 divide_to_i128(i256 numerator, i128 divisor, Rounding rounding) noexcept {
        return take(fixedwide::divide_to_i128(numerator, divisor, rounding));
    }

    /// Unscaled i128 product/divisor with one rounding. Rounding is required.
    [[nodiscard]] i128 mul_div(i128 a, i128 b, i128 divisor, Rounding rounding) noexcept {
        return take(fixedwide::mul_div(a, b, divisor, rounding));
    }

private:
    constexpr void record(ArithmeticError error) noexcept {
        // This is reached only on a failed operation: no context-state test is
        // added to a successful arithmetic step.
        // Construct the first error directly, rather than invoking optional's
        // value-assignment path (which also has to handle an engaged object).
        if (!error_) error_.emplace(error);
    }

    // Standard optional storage preserves trivial copyability. std::expected
    // remains the operation/result boundary; expected<void, E> itself is not
    // required to be trivially copyable. No error code is reserved for success.
    std::optional<ArithmeticError> error_{};
};

} // namespace fixedwide

#undef FIXEDWIDE_CHECKED_INLINE
