#include <fixedwide/literal.hpp>
#include <fixedwide/unchecked.hpp>

auto add() {
    char runtime_text[] = "1.25";
    return fixedwide::Fixed64<2>{} + runtime_text;
}
