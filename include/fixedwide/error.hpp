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

enum class ParseError : std::uint8_t {
    empty,
    invalid,
    too_precise,
    overflow,
    inexact,
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
