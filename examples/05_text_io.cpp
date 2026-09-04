// 05 - Text in and out: parse, to_chars, FormatOptions, std::format, streams.
//
// to_chars writes into a buffer you own and never allocates. to_string wraps it
// for convenience and does allocate. text_capacity is always big enough for any
// basic_fixed, so to_chars into char[text_capacity] cannot run out of room.
//
// Docs: ../docs/api_reference.md#text-conversion

#include <fixedwide/all.hpp>
#include <fixedwide/format.hpp>
#include <fixedwide/iostream.hpp>
#include <format>
#include <iostream>
#include <string>

int main() {
    using namespace fixedwide;
    using Money = Fixed64<6>;

    // Allocation-free formatting into your own buffer.
    const auto v = parse<Money>("1234.567890").value();
    char buf[text_capacity];
    const auto written = to_chars(buf, sizeof buf, v);
    if (!written) return 1;
    const std::string all_digits(buf, *written);
    if (all_digits != "1234.567890") return 1;

    // Fewer decimals, with the rounding named. digits == 0 means "all of them"
    // unless explicit_digits says otherwise.
    const auto two = to_string(v, FormatOptions{.digits = 2, .rounding = Rounding::nearest_even});
    const auto trimmed = to_string(v, FormatOptions{.trim_trailing_zeros = true});
    const auto whole = to_string(v, FormatOptions{.digits = 0, .explicit_digits = true});
    if (!two || !trimmed || !whole) return 1;
    if (*two != "1234.57" || *trimmed != "1234.56789" || *whole != "1235") return 1;
    std::cout << "digits=2            " << *two << "\n";
    std::cout << "trim_trailing_zeros " << *trimmed << "\n";
    std::cout << "digits=0 explicit   " << *whole << "\n";

    // Asking exact for a rendering that would drop a nonzero digit is an error,
    // not a silent round. Dropping only trailing zeros is not a loss, and
    // succeeds.
    const auto refused = to_string(v, FormatOptions{.digits = 1, .rounding = Rounding::exact});
    if (refused || refused.error() != FormatError::inexact) return 1;

    // std::format and std::ostream, for when you are not in a hot loop.
    if (std::format("{}", v) != "1234.567890") return 1;
    std::cout << "operator<<          " << v << "\n";

    // Parsing rejects what it cannot represent exactly, by default.
    if (parse<Fixed64<2>>("1.005")) return 1;                       // too_precise
    if (!parse<Fixed64<2>>("1.005", Rounding::nearest_even)) return 1;
    if (parse<Money>("not a number")) return 1;                     // invalid
    if (parse<Money>("")) return 1;                                 // empty

    std::cout << "OK" << std::endl;
    return 0;
}
