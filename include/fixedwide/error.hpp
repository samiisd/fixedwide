#pragma once
#include <cstdint>

namespace fixedwide {

enum class ArithmeticError : std::uint8_t {
    overflow,
    division_by_zero,
    inexact,
    invalid_precision,
    invalid_value,
};

// `inexact` and `too_precise` are not two names for one thing, and neither is
// `too_precise` a spelling of `invalid_precision` below:
//
//   invalid_precision  the CALLER asked for more decimals than the type has
//   too_precise        the DATA carries more decimals than the type can hold
//   inexact            an exact operation had a remainder
//
// ParseError has no `inexact`: text that does not land on the type's decimal
// grid is `too_precise`, and there is no other way for a parse to be inexact.
enum class ParseError : std::uint8_t {
    empty,
    invalid,
    too_precise,
    overflow,
};

enum class FormatError : std::uint8_t {
    buffer_too_small,
    invalid_precision,
    inexact,
};

enum class BinaryError : std::uint8_t {
    wrong_size,
    invalid_encoding,
};

} // namespace fixedwide
