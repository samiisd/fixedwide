#include "checked_cases.hpp"

namespace {
template<class T>
void runtime_checked_chain() {
    // These chains are also static_asserted. Execute them at runtime as well:
    // constexpr-only instantiations have no counters in this coverage binary.
    CHECK(checked_tests::constant_chain<T>());
    for (int value : {-32768, -2, -1, 0, 1, 2, 32767}) {
        checked_tests::parity(fixedwide::from_integer<T>(value),
                              [&](auto& c) { return c.template from_integer<T>(value); });
    }
}
} // namespace

void run_coverage_checked() {
    checked_tests::run();
    runtime_checked_chain<fixedwide::Fixed8<1>>();
    runtime_checked_chain<fixedwide::Fixed16<2>>();
    runtime_checked_chain<fixedwide::Fixed32<4>>();
    runtime_checked_chain<fixedwide::Fixed64<8>>();
    runtime_checked_chain<fixedwide::Fixed128<10>>();
    runtime_checked_chain<fixedwide::Fixed256<12>>();
    runtime_checked_chain<fixedwide::Fixed64<0>>();
    runtime_checked_chain<fixedwide::Fixed128<0>>();
    runtime_checked_chain<fixedwide::Fixed256<0>>();
    runtime_checked_chain<fixedwide::Fixed64<18>>();
    runtime_checked_chain<fixedwide::Fixed128<38>>();
    runtime_checked_chain<fixedwide::Fixed256<76>>();
}
