#pragma once

/// \file
/// `std::ostream` / `std::istream` support. Include only where you stream:
/// `<iostream>` is one of the heaviest headers in the standard library.

#include <fixedwide/chars.hpp>
#include <iostream>
#include <string>

/// Stream a value out as decimal text, all decimals, nearest-even. A value that
/// cannot be formatted writes nothing rather than throwing.
template<std::size_t Bits, unsigned D>
inline std::ostream& operator<<(std::ostream& os, fixedwide::basic_fixed<Bits, D> val) {
    char buf[fixedwide::text_capacity];
    auto res = fixedwide::to_chars(buf, sizeof(buf), val);
    if (res) {
        os.write(buf, static_cast<std::streamsize>(*res));
    }
    return os;
}

/// Read one whitespace-delimited token and parse it. Text that is not exactly
/// representable sets `failbit` and leaves `val` alone, because the underlying
/// `parse` defaults to `Rounding::exact`.
template<std::size_t Bits, unsigned D>
inline std::istream& operator>>(std::istream& is, fixedwide::basic_fixed<Bits, D>& val) {
    std::string s;
    if (is >> s) {
        auto res = fixedwide::parse<fixedwide::basic_fixed<Bits, D>>(s);
        if (res) {
            val = *res;
        } else {
            is.setstate(std::ios_base::failbit);
        }
    }
    return is;
}
