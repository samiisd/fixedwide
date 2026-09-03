#pragma once
#include <fixedwide/wide.hpp>
#include <cstddef>

namespace fixedwide::detail {
[[nodiscard]] char* integer_digits(char* end, wide::uint128 value) noexcept;
[[nodiscard]] char* integer_digits(char* end, wide::uint256 value) noexcept;
}
