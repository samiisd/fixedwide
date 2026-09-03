#pragma once
#include <fixedwide/arithmetic.hpp>

namespace fixedwide {
inline namespace FIXEDWIDE_SCALE_NAMESPACE {
// Boundary adapters only. Ingress rounds the EXACT binary64 value * decimal scale.
// This intentionally does not pretend that binary64 remembers the original decimal text.
[[nodiscard]] std::expected<FP64, ArithmeticError> from_double64(double, Rounding) noexcept;
[[nodiscard]] std::expected<FP128, ArithmeticError> from_double128(double, Rounding) noexcept;
[[nodiscard]] double to_double(FP64) noexcept;
[[nodiscard]] double to_double(FP128) noexcept;
} // inline namespace
} // namespace fixedwide
