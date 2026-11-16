#include <fixedwide/fixed.hpp>

bool cmp(fixedwide::Fixed64<2> x, int runtime_int) {
    return x == runtime_int;
}
