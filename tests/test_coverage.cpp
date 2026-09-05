// One binary for these suites gives LLVM one coherent mapping for all of the
// exercised header specializations. The regular CTest suite still runs too.
#include "coverage_support.hpp"
#include <string_view>

void run_coverage_wide();
void run_coverage_text();
void run_coverage_arithmetic();
void run_coverage_adapters();
void run_coverage_mixed();

int main(int argc, char** argv) {
    CHECK(argc == 2);
    const std::string_view suite = argv[1];
    bool matched = false;
    auto run = [&](std::string_view name, auto function) {
        if (suite == name || suite == "all") {
            matched = true;
            function();
        }
    };
    run("wide", run_coverage_wide);
    run("text", run_coverage_text);
    run("arithmetic", run_coverage_arithmetic);
    run("adapters", run_coverage_adapters);
    run("mixed", run_coverage_mixed);
    CHECK(matched);
    std::printf("coverage suite %s passed; assertions=%llu\n", argv[1], static_cast<unsigned long long>(checks));
    return 0;
}
