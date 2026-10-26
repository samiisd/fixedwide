#include <fixedwide/literal.hpp>

template<class T>
fixedwide::Fixed64<2> value() {
    return fixedwide::Fixed64<2>{"1.251"};
}

auto invalid = value<int>();
