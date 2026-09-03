#pragma once
#include <cstdint>

namespace fixedwide {

enum class Rounding : std::uint8_t {
    toward_zero,
    floor,
    ceil,
    nearest_even,
    nearest_away,
    exact,
};

} // namespace fixedwide
