#include <fixedwide/literal.hpp>

constexpr char text[]{'1', '.', '2', '5'};
auto value = fixedwide::literal<fixedwide::Fixed64<2>>(text);
