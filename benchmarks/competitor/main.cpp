// Cross-library throughput benchmark with explicit numerical contracts.
// Decimal fixtures are exact at their scale. Binary fixtures have bounded raw
// intermediates and separate tolerance-based validation; they are not decimal.
#include "competitor_common.hpp"
#include "competitor_versions.hpp"

#include <cstdio>
#include <cstdlib>
#include <string_view>

int main(int argc, char** argv) {
    using namespace fixedwide_competitor;
    fixedwide_bench::iterations = 262144;
    fixedwide_bench::repetitions = 11;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--filter" && i + 1 < argc) {
            fixedwide_bench::filter = argv[++i];
        } else if (arg == "--iterations" && i + 1 < argc) {
            fixedwide_bench::iterations = std::strtoull(argv[++i], nullptr, 10);
        } else if (arg == "--repetitions" && i + 1 < argc) {
            fixedwide_bench::repetitions = static_cast<unsigned>(std::strtoul(argv[++i], nullptr, 10));
        } else {
            fail("unknown or incomplete command-line argument");
        }
    }
    if (fixedwide_bench::iterations == 0 || fixedwide_bench::repetitions == 0 ||
        fixedwide_bench::repetitions % 2 == 0) {
        fail("iterations must be nonzero and repetitions a nonzero odd number");
    }

    const Fixtures scale4 = make_scale4_fixtures();
    const Fixtures scale12 = make_scale12_fixtures();
    const char* mode = std::getenv("FIXEDWIDE_BENCH_MODE");

    std::printf("# schema=3\n");
    std::printf("# mode=%s\n", mode != nullptr ? mode : "timing");
    std::printf("# compiler=%s\n", __VERSION__);
    std::printf("# iterations=%zu\n", fixedwide_bench::iterations);
    std::printf("# repetitions=%u\n", fixedwide_bench::repetitions);
    std::printf("# dependencies=%s\n", fixedwide_bench::competitor_dependencies);
    std::printf("# decimal_contract=signed exact scaled-integer fixtures; multiplication and division exact at "
                "declared scale\n");
    std::printf("# binary_contract=shared scale-4 multiplication inputs divided by 32; raw product bounds checked "
                "before execution; tolerance-based validation\n");
    std::printf("# text_contract=fixed notation with all declared fractional digits\n");
    std::printf(
        "library,type,semantic_class,operation,iterations,repetitions,min_ns,median_ns,p95_ns,max_ns,samples\n");

    benchmark_fixed_scale4(scale4);
    benchmark_decimal_float(scale4);
    benchmark_fixed_scale12(scale12);
    benchmark_adjacent_types(scale4);
    benchmark_hardware_floors(scale4);
    benchmark_serialization(scale4);

    std::printf("# validations=%llu\n", static_cast<unsigned long long>(validations));
    std::fprintf(stderr, "PASSED validations=%llu\n", static_cast<unsigned long long>(validations));
    return 0;
}
