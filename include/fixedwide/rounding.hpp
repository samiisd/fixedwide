#pragma once
#include <cstdint>

namespace fixedwide {

/// How an inexact result is resolved to a representable value.
///
/// Every operation that can lose information takes one of these. The default is
/// `nearest_even` for arithmetic and `exact` for parsing and `fixed_cast`, so a
/// conversion never silently rounds unless you ask it to.
enum class Rounding : std::uint8_t {
    /// Truncate towards zero: 2.5 -> 2, -2.5 -> -2. The machine's own division.
    toward_zero,
    /// Round towards negative infinity: 2.5 -> 2, -2.5 -> -3.
    floor,
    /// Round towards positive infinity: 2.5 -> 3, -2.5 -> -2.
    ceil,
    /// Round to nearest; a tie goes to the even neighbour. Banker's rounding,
    /// and the default for arithmetic because it does not drift over a sum.
    nearest_even,
    /// Round to nearest; a tie goes away from zero. Commercial rounding.
    nearest_away,
    /// Refuse to round. An inexact result is `ArithmeticError::inexact`, or
    /// `ParseError::too_precise` when parsing.
    exact,
};

} // namespace fixedwide
