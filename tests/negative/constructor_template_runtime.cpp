#include <fixedwide/literal.hpp>

template<class T>
fixedwide::Fixed64<2> value(char (&text)[5]) {
    return fixedwide::Fixed64<2>{text};
}

char text[] = "1.25";
auto invalid = value<int>(text);
