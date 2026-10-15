#include "checked_cases.hpp"

int main() {
    checked_tests::run();
    std::printf("checked chains passed; assertions=%llu\n", static_cast<unsigned long long>(checks));
}
