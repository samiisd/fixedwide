#pragma once
#include <fixedwide/chars.hpp>
#include <iostream>
#include <string>

template<std::size_t Bits, unsigned D>
inline std::ostream& operator<<(std::ostream& os, fixedwide::basic_fixed<Bits, D> val) {
    char buf[fixedwide::text_capacity];
    auto res = fixedwide::to_chars(buf, sizeof(buf), val);
    if (res) {
        os.write(buf, static_cast<std::streamsize>(*res));
    }
    return os;
}

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
