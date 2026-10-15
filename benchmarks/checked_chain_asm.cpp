// Compile -O3 -DNDEBUG -S or objdump the object. These ordinary, non-C-linkage
// entry points expose identical formula bodies and both expected-return ABIs.
#include "checked_chain_cases.hpp"

using namespace checked_chain_bench;

std::expected<F128, Error> checked_chain_add8_expected(const AddInput& in) noexcept {
    return add8_expected(in);
}
std::expected<F128, Error> checked_chain_add8_accumulated(const AddInput& in) noexcept {
    return add8_accumulated(in);
}
F128 checked_chain_add8_unchecked(const AddInput& in) noexcept {
    return add8_unchecked(in);
}
std::expected<F256, Error> checked_chain_wide_expected(const WideInput& in) noexcept {
    return wide_expected(in);
}
std::expected<F256, Error> checked_chain_wide_accumulated(const WideInput& in) noexcept {
    return wide_accumulated(in);
}
F256 checked_chain_wide_unchecked(const WideInput& in) noexcept {
    return wide_unchecked(in);
}
std::expected<Ratio, Error> checked_chain_float_expected(const FloatInput& in) noexcept {
    return float_expected(in);
}
std::expected<Ratio, Error> checked_chain_float_accumulated(const FloatInput& in) noexcept {
    return float_accumulated(in);
}
Ratio checked_chain_float_unchecked(const FloatInput& in) noexcept {
    return float_unchecked(in);
}
