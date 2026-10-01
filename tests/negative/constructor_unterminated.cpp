#include <fixedwide/literal.hpp>

constexpr char text[]{'1', '.', '2', '5'};
fixedwide::Fixed64<2> value = text;
