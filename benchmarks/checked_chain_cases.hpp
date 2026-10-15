#pragma once

#include <fixedwide/checked.hpp>
#include <fixedwide/unchecked.hpp>
#include <array>

namespace checked_chain_bench {
namespace fw = fixedwide;
using Error = fw::ArithmeticError;
using F128 = fw::Fixed128<10>;
using F256 = fw::Fixed256<12>;
using Rate = fw::Fixed64<8>;
using Ratio = fw::Fixed64<10>;
using Integer = fw::Fixed64<0>;

struct AddInput {
    std::array<F128, 9> v;
};
struct WideInput {
    F256 a, b, divisor;
};
struct FloatInput {
    double rate;
    F128 amount, backing, blocked, margin;
};

inline std::expected<F128, Error> add8_expected(const AddInput& in) noexcept {
    const auto& v = in.v;
    auto a = fw::add(v[0], v[1]);
    if (!a) return std::unexpected(a.error());
    auto b = fw::sub(*a, v[2]);
    if (!b) return std::unexpected(b.error());
    auto c = fw::add(*b, v[3]);
    if (!c) return std::unexpected(c.error());
    auto d = fw::sub(*c, v[4]);
    if (!d) return std::unexpected(d.error());
    auto e = fw::add(*d, v[5]);
    if (!e) return std::unexpected(e.error());
    auto f = fw::sub(*e, v[6]);
    if (!f) return std::unexpected(f.error());
    auto g = fw::add(*f, v[7]);
    if (!g) return std::unexpected(g.error());
    return fw::sub(*g, v[8]);
}
inline std::expected<F128, Error> add8_accumulated(const AddInput& in) noexcept {
    const auto& v = in.v;
    fw::Checked checked;
    auto a = checked.add(v[0], v[1]);
    auto b = checked.sub(a, v[2]);
    auto c = checked.add(b, v[3]);
    auto d = checked.sub(c, v[4]);
    auto e = checked.add(d, v[5]);
    auto f = checked.sub(e, v[6]);
    auto g = checked.add(f, v[7]);
    auto h = checked.sub(g, v[8]);
    return checked.result(h);
}
inline F128 add8_unchecked(const AddInput& in) noexcept {
    const auto& v = in.v;
    return v[0] + v[1] - v[2] + v[3] - v[4] + v[5] - v[6] + v[7] - v[8];
}

inline std::expected<F256, Error> wide_expected(const WideInput& in) noexcept {
    auto p = fw::mul_to<F256>(in.a, in.b);
    if (!p) return std::unexpected(p.error());
    auto q = fw::mul_div_to<F256>(*p, in.b, in.divisor);
    if (!q) return std::unexpected(q.error());
    auto total = fw::add(*q, in.a);
    if (!total) return std::unexpected(total.error());
    return fw::sub(*total, in.a);
}
inline std::expected<F256, Error> wide_accumulated(const WideInput& in) noexcept {
    fw::Checked checked;
    const auto p = checked.mul_to<F256>(in.a, in.b);
    const auto q = checked.mul_div_to<F256>(p, in.b, in.divisor);
    const auto total = checked.add(q, in.a);
    const auto out = checked.sub(total, in.a);
    return checked.result(out);
}
inline F256 wide_unchecked(const WideInput& in) noexcept {
    const auto p = fw::unchecked::mul_to<F256>(in.a, in.b);
    const auto q = fw::unchecked::mul_div_to<F256>(p, in.b, in.divisor);
    return (q + in.a) - in.a;
}

inline std::expected<Ratio, Error> float_expected(const FloatInput& in) noexcept {
    auto rate = fw::from_float<Rate>(in.rate);
    if (!rate) return std::unexpected(rate.error());
    auto converted = fw::mul_to<F128>(in.amount, *rate);
    if (!converted) return std::unexpected(converted.error());
    auto equity = fw::add(*converted, in.backing);
    if (!equity) return std::unexpected(equity.error());
    auto net = fw::sub(*equity, in.blocked);
    if (!net) return std::unexpected(net.error());
    return fw::mul_div_to<Ratio>(*net, Integer::from_raw(100), in.margin);
}
inline std::expected<Ratio, Error> float_accumulated(const FloatInput& in) noexcept {
    fw::Checked checked;
    const auto rate = checked.from_float<Rate>(in.rate);
    const auto converted = checked.mul_to<F128>(in.amount, rate);
    const auto equity = checked.add(converted, in.backing);
    const auto net = checked.sub(equity, in.blocked);
    const auto ratio = checked.mul_div_to<Ratio>(net, Integer::from_raw(100), in.margin);
    return checked.result(ratio);
}
inline Ratio float_unchecked(const FloatInput& in) noexcept {
    // No unchecked floating-ingress API exists. The input fixture has already
    // proved finite, representable conversion; retain the checked kernel and
    // assume its result is engaged, rather than benchmarking a different cast.
    const auto rate = *fw::from_float<Rate>(in.rate);
    const auto converted = fw::unchecked::mul_to<F128>(in.amount, rate);
    const auto net = (converted + in.backing) - in.blocked;
    return fw::unchecked::mul_div_to<Ratio>(net, Integer::from_raw(100), in.margin);
}
} // namespace checked_chain_bench
