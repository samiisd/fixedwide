#pragma once

namespace fixedwide {
enum class ArithmeticError : unsigned char { overflow, division_by_zero, inexact, invalid_precision, invalid_value };
enum class ParseError : unsigned char { empty, invalid, too_precise, overflow };
enum class FormatError : unsigned char { buffer_too_small, invalid_precision, inexact };

// Multiplication/division require a deliberate choice. Text parsing defaults to exact.
enum class Rounding : unsigned char { toward_zero, floor, ceil, nearest_even, nearest_away, exact };
} // namespace fixedwide
