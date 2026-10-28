#include "batch_cases.hpp"

int main() {
    batch_test::run();
    std::printf("batch: %llu checks passed\n", static_cast<unsigned long long>(checks));
}
