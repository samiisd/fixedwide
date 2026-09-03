#pragma once
#include <fixedwide/types.hpp>
#include <cstddef>
namespace fixedwide::detail {
// Returns the beginning of digits written backwards into caller-provided storage.
[[nodiscard]] char* integer_digits(char* end, u128 value) noexcept;
}
