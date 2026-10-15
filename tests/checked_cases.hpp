#pragma once

#include <fixedwide/checked.hpp> // This header must stand alone.
#include "check.hpp"
#include <array>
#include <limits>
#include <type_traits>
#include <utility>

namespace checked_tests {
namespace fw = fixedwide;
using E = fw::ArithmeticError;
using R = fw::Rounding;
inline constexpr std::array modes{R::nearest_even, R::nearest_away, R::toward_zero, R::floor, R::ceil, R::exact};
inline constexpr std::array errors{E::overflow, E::division_by_zero, E::inexact, E::invalid_precision,
                                   E::invalid_value};

static_assert(std::is_trivially_copyable_v<fw::Checked>);
static_assert(sizeof(fw::Checked) == sizeof(std::uint16_t));
static_assert(std::is_nothrow_default_constructible_v<fw::Checked>);
static_assert(!std::is_convertible_v<fw::Checked, bool>);
static_assert(noexcept(std::declval<fw::Checked&>().add(fw::Fixed64<8>{}, fw::Fixed64<8>{})));

template<class A, class B>
concept CanAdd = requires(fw::Checked c, A a, B b) { c.add(a, b); };
static_assert(!CanAdd<fw::Fixed64<8>, fw::Fixed64<10>>);
static_assert(!CanAdd<fw::Fixed64<8>, fw::Fixed128<8>>);
static_assert(!CanAdd<fw::Fixed64<8>, double>);

template<class T>
bool equal(const T& a, const T& b) {
    if constexpr (requires {
                      a.quotient;
                      a.remainder;
                  })
        return a.quotient == b.quotient && a.remainder == b.remainder;
    else
        return a == b;
}

template<class T, class Call>
void parity(const std::expected<T, E>& expected, Call call) {
    fw::Checked c;
    const T actual = call(c);
    CHECK(static_cast<bool>(c) == expected.has_value());
    CHECK(c.error() == (expected ? std::optional<E>{} : expected.error()));
    CHECK(equal(actual, expected ? *expected : T{}));
    const auto published = c.result(actual);
    CHECK(published.has_value() == expected.has_value());
    if (expected)
        CHECK(equal(*published, *expected));
    else
        CHECK(published.error() == expected.error());
}

template<class F>
constexpr bool constant_chain() {
    fw::Checked c;
    const auto a = F::from_raw(typename F::raw_type{12});
    const auto b = F::from_raw(typename F::raw_type{2});
    const auto sum = c.add(a, b);
    const auto total = c.sub(sum, b);
    const auto negative = c.negate(total);
    const auto magnitude = c.abs(negative);
    const auto middle = c.midpoint(magnitude, a, R::exact);
    const auto reduced = c.quantize(middle, F::fractional_digits, R::exact);
    const auto rem = c.remainder(reduced, b);
    const auto zero = c.from_integer<F>(0);
    const auto product = c.mul(zero, reduced, R::exact);
    const auto quotient = c.div(product, b, R::exact);
    const auto fused = c.mul_div(quotient, reduced, b, R::exact);
    const auto coarse = c.midpoint(fused, zero, 0u, R::exact);
    c.take(std::expected<void, E>{});
    if (!c || c.error() || !c.result() || *c.result(coarse) != F{} || rem != F{}) return false;
    const auto failed = c.add(F::max(), F::from_raw(typename F::raw_type{1}));
    if (failed != F{} || c || c.error() != E::overflow) return false;
    const auto later = c.div(a, failed); // Zero placeholder is checked, not UB.
    return later == F{} && !c.result(a) && c.result(a).error() == E::overflow;
}
static_assert(constant_chain<fw::Fixed8<1>>());
static_assert(constant_chain<fw::Fixed16<2>>());
static_assert(constant_chain<fw::Fixed32<4>>());
static_assert(constant_chain<fw::Fixed64<8>>());
static_assert(constant_chain<fw::Fixed128<10>>());
static_assert(constant_chain<fw::Fixed256<12>>());
static_assert([] {
    fw::Checked c;
    auto value = c.narrow(fw::FP128{});
    return c && value == fw::FP64{} && !c.error();
}());

template<class F>
void same_domain() {
    using Raw = typename F::raw_type;
    const std::array values{F{},
                            F::from_raw(Raw{1}),
                            F::from_raw(Raw{-1}),
                            F::from_raw(Raw{2}),
                            F::from_raw(Raw{3}),
                            F::from_raw(F::scale()),
                            F::min(),
                            F::max()};
    for (const auto a : values) {
        parity(fw::negate(a), [&](auto& c) { return c.negate(a); });
        parity(fw::abs(a), [&](auto& c) { return c.abs(a); });
        for (const auto r : modes) {
            for (const auto digits : {0u, F::fractional_digits, F::fractional_digits + 1u}) {
                parity(fw::quantize(a, digits, r), [&](auto& c) { return c.quantize(a, digits, r); });
            }
        }
        for (const auto b : values) {
            parity(fw::add(a, b), [&](auto& c) { return c.add(a, b); });
            parity(fw::sub(a, b), [&](auto& c) { return c.sub(a, b); });
            parity(fw::remainder(a, b), [&](auto& c) { return c.remainder(a, b); });
            // Default rounding is part of the member/free-function contract.
            parity(fw::mul(a, b), [&](auto& c) { return c.mul(a, b); });
            parity(fw::div(a, b), [&](auto& c) { return c.div(a, b); });
            parity(fw::mul_div(a, b, b), [&](auto& c) { return c.mul_div(a, b, b); });
            parity(fw::midpoint(a, b), [&](auto& c) { return c.midpoint(a, b); });
            parity(fw::midpoint(a, b, 0u), [&](auto& c) { return c.midpoint(a, b, 0u); });
            for (const auto r : modes) {
                parity(fw::mul(a, b, r), [&](auto& c) { return c.mul(a, b, r); });
                parity(fw::div(a, b, r), [&](auto& c) { return c.div(a, b, r); });
                parity(fw::mul_div(a, b, b, r), [&](auto& c) { return c.mul_div(a, b, b, r); });
                parity(fw::mul_div(a, a, b, r), [&](auto& c) { return c.mul_div(a, a, b, r); });
                parity(fw::midpoint(a, b, r), [&](auto& c) { return c.midpoint(a, b, r); });
                for (const auto digits : {0u, F::fractional_digits, F::fractional_digits + 1u}) {
                    parity(fw::midpoint(a, b, digits, r), [&](auto& c) { return c.midpoint(a, b, digits, r); });
                }
            }
        }
    }
    parity(fw::quantize(F::max(), 0u), [&](auto& c) { return c.quantize(F::max(), 0u); });
    for (const auto n : {std::int64_t{0}, std::int64_t{1}, std::int64_t{-1}, std::numeric_limits<std::int64_t>::max(),
                         std::numeric_limits<std::int64_t>::min()}) {
        parity(fw::from_integer<F>(n), [&](auto& c) { return c.template from_integer<F>(n); });
    }
    constexpr auto umax = std::numeric_limits<std::uint64_t>::max();
    parity(fw::from_integer<F>(umax), [&](auto& c) { return c.template from_integer<F>(umax); });
    parity(fw::from_integer<F>(true), [&](auto& c) { return c.template from_integer<F>(true); });
#if defined(__SIZEOF_INT128__)
    const unsigned __int128 big = static_cast<unsigned __int128>(1) << 127;
    parity(fw::from_integer<F>(big), [&](auto& c) { return c.template from_integer<F>(big); });
    const auto negative = static_cast<__int128>(big);
    parity(fw::from_integer<F>(negative), [&](auto& c) { return c.template from_integer<F>(negative); });
#endif
}

template<class Dest, class A, class B>
void mixed() {
    const std::array as{A{}, A::from_raw(typename A::raw_type{1}), A::from_raw(A::scale()), A::max(), A::min()};
    const std::array bs{B{},
                        B::from_raw(typename B::raw_type{-1}),
                        B::from_raw(typename B::raw_type{3}),
                        B::from_raw(B::scale()),
                        B::max(),
                        B::min()};
    for (const auto a : as) {
        parity(fw::fixed_cast<Dest>(a), [&](auto& c) { return c.template fixed_cast<Dest>(a); });
        for (const auto r : modes) {
            parity(fw::fixed_cast<Dest>(a, r), [&](auto& c) { return c.template fixed_cast<Dest>(a, r); });
        }
        for (const auto b : bs) {
            parity(fw::add_to<Dest>(a, b), [&](auto& c) { return c.template add_to<Dest>(a, b); });
            parity(fw::sub_to<Dest>(a, b), [&](auto& c) { return c.template sub_to<Dest>(a, b); });
            parity(fw::mul_to<Dest>(a, b), [&](auto& c) { return c.template mul_to<Dest>(a, b); });
            parity(fw::div_to<Dest>(a, b), [&](auto& c) { return c.template div_to<Dest>(a, b); });
            parity(fw::mul_div_to<Dest>(a, b, b), [&](auto& c) { return c.template mul_div_to<Dest>(a, b, b); });
            for (const auto r : modes) {
                parity(fw::add_to<Dest>(a, b, r), [&](auto& c) { return c.template add_to<Dest>(a, b, r); });
                parity(fw::sub_to<Dest>(a, b, r), [&](auto& c) { return c.template sub_to<Dest>(a, b, r); });
                parity(fw::mul_to<Dest>(a, b, r), [&](auto& c) { return c.template mul_to<Dest>(a, b, r); });
                parity(fw::div_to<Dest>(a, b, r), [&](auto& c) { return c.template div_to<Dest>(a, b, r); });
                parity(fw::mul_div_to<Dest>(a, b, b, r),
                       [&](auto& c) { return c.template mul_div_to<Dest>(a, b, b, r); });
            }
        }
    }
}

template<class F, class Float>
void floating() {
    const std::array values{Float{0},
                            -Float{0},
                            Float{1.25},
                            Float{-1.25},
                            static_cast<Float>(0.1),
                            std::numeric_limits<Float>::max(),
                            std::numeric_limits<Float>::lowest(),
                            std::numeric_limits<Float>::denorm_min(),
                            std::numeric_limits<Float>::infinity(),
                            -std::numeric_limits<Float>::infinity(),
                            std::numeric_limits<Float>::quiet_NaN()};
    for (const auto value : values) {
        parity(fw::from_float<F>(value), [&](auto& c) { return c.template from_float<F>(value); });
        for (const auto r : modes) {
            parity(fw::from_float<F>(value, r), [&](auto& c) { return c.template from_float<F>(value, r); });
            if constexpr (std::is_same_v<Float, double>) {
                parity(fw::from_double<F>(value, r), [&](auto& c) { return c.template from_double<F>(value, r); });
                parity(fw::from_double64(value, r), [&](auto& c) { return c.from_double64(value, r); });
                parity(fw::from_double128(value, r), [&](auto& c) { return c.from_double128(value, r); });
            }
        }
    }
}

inline void legacy_and_raw() {
    for (const auto a : {fw::FP64{}, fw::FP64::from_raw(1), fw::FP64::from_raw(-1), fw::FP64::max(), fw::FP64::min()}) {
        for (const auto r : modes) {
            parity(fw::mul_wide(a, a, r), [&](auto& c) { return c.mul_wide(a, a, r); });
        }
    }
    for (const auto a :
         {fw::FP128{}, fw::FP128::from_raw(fw::i128{1}), fw::FP128::from_raw(fw::i128{-1}),
          fw::FP128::from_raw(fw::i128{0x8000000000000000ULL, ~0ULL}), fw::FP128::max(), fw::FP128::min()}) {
        parity(fw::narrow(a), [&](auto& c) { return c.narrow(a); });
    }
    for (const auto a : {fw::u256{}, fw::u256{7}, ~fw::u256{}}) {
        for (const auto b : {fw::u128{}, fw::u128{1}, fw::u128{3}, ~fw::u128{}}) {
            parity(fw::divmod(a, b), [&](auto& c) { return c.divmod(a, b); });
        }
    }
    for (const auto a :
         {fw::i256{}, fw::i256{7}, fw::i256{-7}, fw::Fixed256<0>::max().raw(), fw::Fixed256<0>::min().raw()}) {
        for (const auto b : {fw::i128{}, fw::i128{1}, fw::i128{-1}, fw::i128{3}, fw::Fixed128<0>::min().raw()}) {
            parity(fw::divmod(a, b), [&](auto& c) { return c.divmod(a, b); });
            for (const auto r : modes) {
                parity(fw::divide_to_i128(a, b, r), [&](auto& c) { return c.divide_to_i128(a, b, r); });
            }
        }
    }
    for (const auto a :
         {fw::i128{}, fw::i128{7}, fw::i128{-7}, fw::Fixed128<0>::max().raw(), fw::Fixed128<0>::min().raw()}) {
        for (const auto b : {fw::i128{}, fw::i128{1}, fw::i128{-1}, fw::i128{3}}) {
            for (const auto r : modes) {
                parity(fw::mul_div(a, a, b, r), [&](auto& c) { return c.mul_div(a, a, b, r); });
            }
        }
    }
}

struct MoveOnly {
    int value = 0;
    MoveOnly() noexcept = default;
    explicit MoveOnly(int v) noexcept : value(v) {}
    MoveOnly(MoveOnly&& other) noexcept : value(std::exchange(other.value, -1)) {}
    MoveOnly(const MoveOnly&) = delete;
};
struct ThrowingDefault {
    ThrowingDefault() noexcept(false) {}
};
template<class T>
concept CanTake = requires(fw::Checked c, std::expected<T, E> value) { c.take(std::move(value)); };
static_assert(CanTake<MoveOnly>);
static_assert(!CanTake<ThrowingDefault>);

inline void state_and_hooks() {
    using F = fw::Fixed64<2>;
    fw::Checked c;
    CHECK(c && !c.error());
    const std::expected<F, E> good{F::from_raw(123)};
    CHECK(c.take(good) == *good); // Const lvalue.
    auto mutable_good = good;
    CHECK(c.take(mutable_good) == *good); // Nonconst lvalue copies, not consumes.
    CHECK(c.take(std::expected<F, E>{*good}) == *good);
    CHECK(c.result(*good) == good);
    CHECK(c.result(*good) == good); // Publishing is a snapshot, not reset/move state.
    c.take(std::expected<void, E>{});
    CHECK(c.result());
    for (const auto first : errors) {
        for (const auto second : errors) {
            fw::Checked state;
            const std::expected<F, E> failure{std::unexpected(first)};
            CHECK(state.take(failure) == F{});
            CHECK(state.take(good) == *good); // Successful steps still return their value.
            CHECK(state.take(std::expected<F, E>{std::unexpected(second)}) == F{});
            state.take(std::expected<void, E>{std::unexpected(second)});
            CHECK(!state && state.error() == first);
            CHECK(!state.result(*good) && state.result(*good).error() == first);
            CHECK(!state.result() && state.result().error() == first);
        }
    }
    fw::Checked all_error_bits;
    const auto unusual = static_cast<E>(255);
    all_error_bits.take(std::expected<void, E>{std::unexpected(unusual)});
    CHECK(!all_error_bits && all_error_bits.error() == unusual);
    CHECK(all_error_bits.result().error() == unusual);
    fw::Checked void_first;
    void_first.take(std::expected<void, E>{std::unexpected(E::invalid_value)});
    CHECK(void_first.error() == E::invalid_value);
    CHECK(void_first.take(good) == *good && !void_first);

    auto independent = c;
    const auto denominator = independent.add(F::max(), F::from_raw(1));
    CHECK(denominator == F{} && independent.error() == E::overflow);
    CHECK(independent.div(*good, denominator) == F{});
    CHECK(independent.from_float<F>(std::numeric_limits<double>::quiet_NaN()) == F{});
    CHECK(independent.error() == E::overflow && c);
    CHECK(independent.add(F::from_raw(2), F::from_raw(3)) == F::from_raw(5));
    CHECK(!independent.result(*good));
    auto failed_copy = independent;
    CHECK(failed_copy.error() == E::overflow);
    CHECK(failed_copy.quantize(*good, 99) == F{});
    CHECK(failed_copy.error() == E::overflow);

    auto moved = c.take(std::expected<MoveOnly, E>{std::in_place, 9});
    CHECK(moved.value == 9);
    const auto result = c.result(std::move(moved));
    CHECK(result && result->value == 9 && moved.value == -1);
    MoveOnly untouched{11};
    const auto not_published = independent.result(std::move(untouched));
    CHECK(!not_published && untouched.value == 11);
    fw::Checked failed_move;
    const auto placeholder = failed_move.take(std::expected<MoveOnly, E>{std::unexpected(E::inexact)});
    CHECK(placeholder.value == 0 && failed_move.error() == E::inexact);

    int calls = 0;
    const auto custom = [&]() noexcept -> std::expected<F, E> {
        ++calls;
        return F::from_raw(7);
    };
    CHECK(independent.take(custom()) == F::from_raw(7));
    CHECK(calls == 1); // take is eager; caller-defined work is never skipped.
}

inline void run() {
    state_and_hooks();
    same_domain<fw::Fixed8<1>>();
    same_domain<fw::Fixed16<2>>();
    same_domain<fw::Fixed32<4>>();
    same_domain<fw::Fixed64<8>>();
    same_domain<fw::Fixed128<10>>();
    same_domain<fw::Fixed256<12>>();
    same_domain<fw::Fixed64<0>>();
    same_domain<fw::Fixed128<0>>();
    same_domain<fw::Fixed256<0>>();
    same_domain<fw::Fixed64<18>>();
    same_domain<fw::Fixed128<38>>();
    same_domain<fw::Fixed256<76>>();
    mixed<fw::Fixed8<1>, fw::Fixed16<2>, fw::Fixed32<4>>();
    mixed<fw::Fixed16<2>, fw::Fixed32<4>, fw::Fixed8<1>>();
    mixed<fw::Fixed32<4>, fw::Fixed64<8>, fw::Fixed16<2>>();
    mixed<fw::Fixed64<10>, fw::Fixed64<8>, fw::Fixed64<12>>();
    mixed<fw::Fixed128<12>, fw::Fixed64<8>, fw::Fixed128<10>>();
    mixed<fw::Fixed256<12>, fw::Fixed128<10>, fw::Fixed256<38>>();
    mixed<fw::Fixed256<0>, fw::Fixed256<76>, fw::Fixed256<0>>();
    mixed<fw::Fixed256<76>, fw::Fixed256<0>, fw::Fixed256<76>>();
    floating<fw::Fixed8<1>, float>();
    floating<fw::Fixed16<2>, double>();
    floating<fw::Fixed32<4>, long double>();
    floating<fw::Fixed64<8>, double>();
    floating<fw::Fixed128<10>, long double>();
    floating<fw::Fixed256<76>, double>();
    legacy_and_raw();
}
} // namespace checked_tests
