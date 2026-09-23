#include <fixedwide/literal.hpp>

auto convert() {
    char runtime_text[] = "1.25";
    return fixedwide::literal<fixedwide::Fixed64<2>>(runtime_text);
}
