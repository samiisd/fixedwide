#pragma once

// The batch contract is the scalar row order, including intermediate errors.
// A later speculative map must never outrank an earlier commit failure.
#include <fixedwide/batch.hpp>
#include <fixedwide/floating.hpp>
#include <fixedwide/literal.hpp>
#include "check.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <expected>
#include <random>
#include <span>
#include <vector>

namespace batch_test {

namespace fw = fixedwide;
namespace batch = fw::batch;
using Money = fw::Fixed128<12>;
using Price = fw::Fixed64<10>;
using WideMoney = fw::Fixed256<12>;
using Error = fw::ArithmeticError;
using Raw = fw::wide::int128;
template<class T>
using Expected = std::expected<T, Error>;

inline constexpr std::array modes{fw::Rounding::toward_zero,  fw::Rounding::floor,        fw::Rounding::ceil,
                                  fw::Rounding::nearest_even, fw::Rounding::nearest_away, fw::Rounding::exact};

template<class T>
T raw(std::int64_t value) {
    return T::from_raw(static_cast<typename T::raw_type>(value));
}

template<class T>
void same(const Expected<T>& result, const Expected<T>& reference) {
    CHECK(result.has_value() == reference.has_value());
    if (reference)
        CHECK(*result == *reference);
    else
        CHECK(result.error() == reference.error());
}

template<class Dest, class A, class B>
void product(A a, B b, fw::Rounding mode) {
    const auto reference = fw::mul_to<Dest>(a, b, mode);
    const auto prepared = batch::prepare_mul_to<Dest>(b, mode);
    same(prepared(a), reference);
    same(batch::mul_to<Dest>(mode)(a, b), reference);

    const std::array<A, 1> lhs{a};
    const std::array<B, 1> rhs{b};
    const Dest sentinel = raw<Dest>(17);
    std::array<Dest, 1> output{sentinel};
    const auto check_result = [&](const auto& status) {
        CHECK(status.has_value() == reference.has_value());
        if (reference)
            CHECK(output[0] == *reference);
        else {
            CHECK(status.error().index == 0);
            CHECK(status.error().error == reference.error());
            CHECK(output[0] == sentinel);
        }
    };
    check_result(batch::mul_to<Dest>(std::span{output}, std::span{lhs}, std::span{rhs}, mode));
    output[0] = sentinel;
    check_result(batch::mul_to<Dest>(std::span{output}, std::span{lhs}, b, mode));
    output[0] = sentinel;
    check_result(batch::mul_to<Dest>(std::span{output}, a, std::span{rhs}, mode));
}

template<class Dest, class A, class B>
void product(A a, B b) {
    for (auto mode : modes) product<Dest>(a, b, mode);
}

inline void test_multiply_domains() {
    const std::array values{Raw{0},
                            Raw{1},
                            Raw{-1},
                            Raw::min(),
                            Raw::max(),
                            Raw::min() + Raw{1},
                            Raw::max() - Raw{1},
                            Raw{INT64_MIN},
                            Raw{INT64_MAX},
                            Raw{0, 1},
                            Raw{~std::uint64_t{0}, 0x7fff'ffff'ffff'ffffULL}};
    const std::array price_values{std::int64_t{0},
                                  std::int64_t{1},
                                  std::int64_t{-1},
                                  std::int64_t{10'000'000'000},
                                  std::int64_t{-10'000'000'000},
                                  std::int64_t{1'234'567'891},
                                  std::int64_t{INT64_MIN},
                                  std::int64_t{INT64_MAX}};
    for (auto a : values) {
        for (auto b : price_values) {
            product<Money>(Money::from_raw(a), Price::from_raw(b));
            product<Money>(Price::from_raw(b), Money::from_raw(a));
        }
        for (auto b : values) {
            product<Money>(Money::from_raw(a), Money::from_raw(b));
            product<fw::Fixed256<24>>(Money::from_raw(a), Money::from_raw(b));
        }
    }

    // This result is representable despite overflowing the unscaled product.
    const Money huge{"170100000000000000000000000"};
    product<Money>(huge, Price{"1"});
    product<Money>(huge, Price{"2"});
    product<Money>(Money::min(), Price{"1"});
    product<Money>(Money::min(), Price{"-1"});
    product<Money>(Money::max(), Money{"0.1"});

    // All-six-mode rounding, quotient parity, adjacent ties and range-before-
    // inexact precedence for both signs and a deliberately narrow destination.
    for (std::int64_t q = -4; q <= 4; ++q)
        for (std::int64_t offset = -1; offset <= 1; ++offset)
            product<fw::Fixed128<0>>(raw<fw::Fixed128<1>>(q * 10 + 5 + offset), raw<fw::Fixed64<0>>(1));
    for (auto a : {1271, 1275, 1279, 1281, -1281, -1285, -1289, -1291})
        product<fw::Fixed8<0>>(raw<fw::Fixed128<1>>(a), raw<fw::Fixed64<0>>(1));

    // Gaps at and beyond the native power-of-ten limits must use a valid
    // scalar fallback, never instantiate an overflowing native constant.
    for (auto a : {Raw{0}, Raw{1}, Raw{-1}, Raw::min(), Raw::max()}) {
        product<fw::Fixed128<0>>(fw::Fixed128<19>::from_raw(a), fw::Fixed128<20>::max());
        product<fw::Fixed128<0>>(fw::Fixed128<38>::from_raw(a), fw::Fixed128<38>::max());
        product<fw::Fixed128<38>>(fw::Fixed128<0>::from_raw(a), raw<fw::Fixed64<0>>(1));
        product<fw::Fixed256<76>>(fw::Fixed128<0>::from_raw(a), raw<fw::Fixed64<0>>(1));
        product<fw::Fixed128<0>>(fw::Fixed128<19>::from_raw(a), raw<fw::Fixed8<0>>(-1));
        product<fw::Fixed256<36>>(fw::Fixed256<24>::from_raw(fw::wide::int256{a}), Money::max());
    }
    product<fw::Fixed256<46>>(fw::Fixed256<24>::max(), fw::Fixed256<22>::min());
    product<fw::Fixed256<0>>(fw::Fixed256<76>::max(), fw::Fixed256<0>::min());

    std::mt19937_64 random{0xb47c4e7ULL};
    for (unsigned i = 0; i < 400; ++i) {
        const auto a = Money::from_raw(Raw{random(), random()});
        const auto b = Price::from_raw(std::bit_cast<std::int64_t>(random()));
        product<Money>(a, b);
        product<Money>(a, Money::from_raw(Raw{random(), random()}));
        // Retain realistic successful values as well as the mostly-overflow
        // uniform full-width corpus.
        const auto small_a = raw<Money>(static_cast<std::int64_t>(random() % 1'000'000'000'000'000'000ULL));
        const auto small_b = raw<Price>(static_cast<std::int64_t>(random() % 100'000'000'001ULL) - 50'000'000'000LL);
        product<Money>(small_a, small_b);
    }
}

template<std::size_t Block>
void check_transform_contract() {
    const Money sentinel = raw<Money>(-777);
    for (std::size_t count = 0; count < 19; ++count) {
        // Offset the spans by one element: only the type's natural alignment
        // is promised, not an arbitrary SIMD alignment.
        std::vector<Money> input(count + 2, sentinel), output(count + 2, sentinel);
        for (std::size_t i = 0; i < count; ++i) input[i + 1] = raw<Money>(static_cast<std::int64_t>(i));
        for (std::size_t failed = 0; failed <= count; ++failed) {
            std::fill(output.begin(), output.end(), sentinel);
            const auto map = [failed](Money value) noexcept -> Expected<Money> {
                if (value.raw() == Raw{static_cast<std::int64_t>(failed)}) return std::unexpected(Error::inexact);
                return fw::add(value, raw<Money>(100));
            };
            const auto result =
                batch::transform<Block>(std::span{output}.subspan(1, count), map, std::span{input}.subspan(1, count));
            CHECK(result.has_value() == (failed == count));
            if (!result) {
                CHECK(result.error().index == failed);
                CHECK(result.error().error == Error::inexact);
            }
            for (std::size_t i = 0; i < count; ++i)
                CHECK(output[i + 1] == (i < failed ? raw<Money>(static_cast<std::int64_t>(i) + 100) : sentinel));
            CHECK(output.front() == sentinel);
            CHECK(output.back() == sentinel);
        }

        // A pure infallible map and exact in-place alias are both supported.
        const auto result = batch::transform<Block>(
            std::span{input}.subspan(1, count),
            [](Money value, Money increment) noexcept { return Money::from_raw(value.raw() + increment.raw()); },
            std::span{input}.subspan(1, count), raw<Money>(7));
        CHECK(result);
        for (std::size_t i = 0; i < count; ++i) CHECK(input[i + 1] == raw<Money>(static_cast<std::int64_t>(i) + 7));
        CHECK(input.front() == sentinel);
        CHECK(input.back() == sentinel);
    }
}

inline void test_in_place() {
    std::array<Money, 9> values{};
    for (std::size_t i = 0; i < values.size(); ++i) values[i] = raw<Money>(static_cast<std::int64_t>(i) + 1);
    CHECK(batch::add(std::span{values}, std::span{values}, std::span{values}));
    CHECK(batch::mul_to<Money>(std::span{values}, std::span{values}, Price{"2"}, fw::Rounding::exact));
    CHECK(batch::sub(std::span{values}, raw<Money>(100), std::span{values}));
    for (std::size_t i = 0; i < values.size(); ++i)
        CHECK(values[i] == raw<Money>(100 - 4 * (static_cast<std::int64_t>(i) + 1)));

    Price factor{"2"};
    const auto plan = batch::prepare_mul_to<Money>(factor, fw::Rounding::exact);
    const auto copy = plan;
    factor = Price{"3"};
    CHECK(plan(Money{"4"}) == Expected<Money>{Money{"8"}});
    CHECK(copy(Money{"4"}) == Expected<Money>{Money{"8"}});

    // MSVC rejects consteval literal temporaries inside non-constexpr local
    // aggregates. Keep literal tables constexpr and copy them when mutable
    // inputs are needed; the arithmetic under test still executes at runtime.
    constexpr std::array original{Money{"0.01"}, Money{"0.005"}, Money{"1"}, Money{"2"}, Money{"3"}};
    auto exact_values = original;
    const auto failed =
        batch::mul_to<Money>(std::span{exact_values}, std::span{exact_values}, Price::from_raw(1), fw::Rounding::exact);
    CHECK(!failed);
    CHECK(failed.error().index == 1);
    CHECK(failed.error().error == Error::inexact);
    CHECK(exact_values[0] == raw<Money>(1));
    for (std::size_t i = 1; i < exact_values.size(); ++i) CHECK(exact_values[i] == original[i]);

    std::span<Money> empty;
    CHECK(batch::mul_to<Money>(empty, empty, Money::max(), fw::Rounding::exact));
    CHECK(batch::add(empty, empty, Money::max()));
    CHECK(batch::div_to<Money>(empty, empty, Money{}));
}

inline void test_aggregate_map() {
    struct Update {
        Money value, total;
    };
    constexpr std::array slopes{Money{"1"}, Money{"2"}, Money{"100000000000000000000000000"}, Money{"3"}};
    constexpr std::array values{Money{"10"}, Money{"20"}, Money::max(), Money{"30"}};
    constexpr std::array totals{Money{"100"}, Money{"200"}, Money{}, Money{"300"}};
    const auto multiplier = batch::prepare_mul_to<Money>(Price{"1"}, fw::Rounding::exact);
    const auto map = [multiplier](Money slope, Money value, Money total) noexcept -> Expected<Update> {
        const auto delta = multiplier(slope);
        if (!delta) return std::unexpected(delta.error());
        const auto after = fw::add(value, *delta);
        if (!after) return std::unexpected(after.error());
        const auto sum = fw::add(total, *delta);
        if (!sum) return std::unexpected(sum.error());
        return Update{*after, *sum};
    };
    std::array<Update, 4> output{};
    const auto result =
        batch::transform<4>(std::span{output}, map, std::span{slopes}, std::span{values}, std::span{totals});
    CHECK(!result);
    CHECK(result.error().index == 2);
    CHECK(result.error().error == Error::overflow);
    CHECK(output[0].value == Money{"11"} && output[0].total == Money{"101"});
    CHECK(output[1].value == Money{"22"} && output[1].total == Money{"202"});
    CHECK(output[2].value == Money{} && output[2].total == Money{});
    CHECK(output[3].value == Money{} && output[3].total == Money{});
}

inline void test_operation_wrappers() {
    constexpr std::array a{Money{"1.25"}, Money{"-2.5"}, Money{"0"}, Money{"100000000000000000000"}};
    constexpr std::array b{Price{"2"}, Price{"-0.5"}, Price{"3.25"}, Price{"0.1"}};
    constexpr std::array c{Money{"2"}, Money{"0.5"}, Money{"1"}, Money{"100"}};
    std::array<Money, 4> output{};
    for (auto mode : modes) {
        const auto verify = [&](auto batch_operation, auto scalar_operation) {
            const Money sentinel = raw<Money>(-222);
            output.fill(sentinel);
            const auto result = batch_operation();
            std::size_t prefix = 0;
            for (; prefix < a.size(); ++prefix) {
                const auto reference = scalar_operation(prefix);
                if (!reference) {
                    CHECK(!result);
                    CHECK(result.error().index == prefix);
                    CHECK(result.error().error == reference.error());
                    break;
                }
                CHECK(output[prefix] == *reference);
            }
            if (prefix == a.size()) CHECK(result);
            for (std::size_t i = prefix; i < a.size(); ++i) CHECK(output[i] == sentinel);
        };
        verify([&] { return batch::add_to<Money>(std::span{output}, std::span{a}, std::span{b}, mode); },
               [&](std::size_t i) { return fw::add_to<Money>(a[i], b[i], mode); });
        verify([&] { return batch::sub_to<Money>(std::span{output}, std::span{a}, std::span{b}, mode); },
               [&](std::size_t i) { return fw::sub_to<Money>(a[i], b[i], mode); });
        verify([&] { return batch::div_to<Money>(std::span{output}, std::span{a}, std::span{b}, mode); },
               [&](std::size_t i) { return fw::div_to<Money>(a[i], b[i], mode); });
        verify(
            [&] { return batch::mul_div_to<Money>(std::span{output}, std::span{a}, std::span{b}, std::span{c}, mode); },
            [&](std::size_t i) { return fw::mul_div_to<Money>(a[i], b[i], c[i], mode); });
        verify([&] { return batch::fixed_cast<Money>(std::span{output}, std::span{b}, mode); },
               [&](std::size_t i) { return fw::fixed_cast<Money>(b[i], mode); });
    }
    constexpr std::array denominator{Money{"1"}, Money{"0"}, Money{"1"}, Money{"1"}};
    output.fill(raw<Money>(77));
    const auto divided = batch::div_to<Money>(std::span{output}, std::span{a}, std::span{denominator});
    CHECK(!divided);
    CHECK(divided.error().index == 1);
    CHECK(divided.error().error == Error::division_by_zero);
    CHECK(output[0] == a[0]);
    CHECK(output[1] == raw<Money>(77));
    CHECK(output[2] == raw<Money>(77));
}

template<std::size_t Block>
void check_ordered_commit() {
    constexpr std::size_t count = 13;
    std::array<Money, count> input{};
    for (std::size_t i = 0; i < count; ++i) input[i] = raw<Money>(static_cast<std::int64_t>(i));
    for (std::size_t map_error = 0; map_error <= count; ++map_error) {
        for (std::size_t commit_error = 0; commit_error <= count; ++commit_error) {
            std::size_t committed = 0;
            const auto map = [map_error](Money value) noexcept -> Expected<Money> {
                if (value == raw<Money>(static_cast<std::int64_t>(map_error))) return std::unexpected(Error::inexact);
                return value;
            };
            const auto commit = [&](std::size_t index, Money value) noexcept -> Expected<void> {
                CHECK(index == committed);
                CHECK(value == input[index]);
                if (index == commit_error) return std::unexpected(Error::overflow);
                ++committed;
                return {};
            };
            const auto result = batch::for_each<Block>(count, map, commit, std::span{input});
            const std::size_t first_error = map_error < commit_error ? map_error : commit_error;
            CHECK(committed == first_error);
            CHECK(result.has_value() == (first_error == count));
            if (!result) {
                CHECK(result.error().index == first_error);
                // Mapping precedes the commit in the same row.
                CHECK(result.error().error == (map_error <= commit_error ? Error::inexact : Error::overflow));
            }
        }
    }
}

inline void test_scatter_delta() {
    constexpr std::size_t count = 61;
    constexpr std::size_t balance_count = 9;
    for (bool holes : {false, true}) {
        std::array<Money, count> slopes{}, values{};
        std::array<std::uint32_t, count> balance{};
        std::array<Money, balance_count> totals{};
        constexpr std::array seeds{Money{"1"},  Money{"100"},  Money{"100000"},  Money{"0.01"},
                                   Money{"-1"}, Money{"-100"}, Money{"-100000"}, Money{"-0.01"}};
        for (std::size_t i = 0; i < count; ++i) {
            const bool hole = holes && i % 20 == 0;
            slopes[i] = hole ? Money{} : seeds[i % seeds.size()];
            balance[i] = hole ? 0 : static_cast<std::uint32_t>(1 + i % (balance_count - 1));
            values[i] = hole ? Money{} : raw<Money>(static_cast<std::int64_t>(i) * 123);
        }
        auto reference_values = values;
        auto reference_totals = totals;
        const Price dx = Price::from_raw(1);
        for (std::size_t i = 0; i < count; ++i) {
            const auto delta = fw::mul_to<Money>(slopes[i], dx, fw::Rounding::exact);
            CHECK(delta);
            reference_values[i] = *fw::add(reference_values[i], *delta);
            reference_totals[balance[i]] = *fw::add(reference_totals[balance[i]], *delta);
        }
        const auto map = batch::prepare_mul_to<Money>(dx, fw::Rounding::exact);
        const auto commit = [&](std::size_t index, Money delta) noexcept -> Expected<void> {
            const auto next = fw::add(values[index], delta);
            if (!next) return std::unexpected(next.error());
            const auto sum = fw::add(totals[balance[index]], delta);
            if (!sum) return std::unexpected(sum.error());
            values[index] = *next;
            totals[balance[index]] = *sum;
            return {};
        };
        CHECK(batch::for_each<4>(count, map, commit, std::span{slopes}));
        CHECK(values == reference_values);
        CHECK(totals == reference_totals);
        CHECK(totals[0] == Money{});
    }

    // A later inexact multiply must not hide an earlier balance overflow, and
    // the failing row must not update its individual value first.
    constexpr std::array slopes{Money{"0.01"}, Money{"0.01"}, Money{"0.005"}, Money{"0.01"}};
    std::array<Money, 4> values{};
    Money total = Money::max();
    const auto map = batch::prepare_mul_to<Money>(Price::from_raw(1), fw::Rounding::exact);
    const auto commit = [&](std::size_t index, Money delta) noexcept -> Expected<void> {
        const auto after = fw::add(values[index], delta);
        if (!after) return std::unexpected(after.error());
        const auto next_total = fw::add(total, delta);
        if (!next_total) return std::unexpected(next_total.error());
        values[index] = *after;
        total = *next_total;
        return {};
    };
    const auto result = batch::for_each<4>(slopes.size(), map, commit, std::span{slopes});
    CHECK(!result);
    CHECK(result.error().index == 0);
    CHECK(result.error().error == Error::overflow);
    CHECK(total == Money::max());
    for (auto value : values) CHECK(value == Money{});
}

inline Expected<Money> replacement(Money total, Money previous, Money next) noexcept {
    const auto removed = fw::sub_to<WideMoney>(total, previous, fw::Rounding::exact);
    if (!removed) return std::unexpected(removed.error());
    const auto added = fw::add_to<WideMoney>(*removed, next, fw::Rounding::exact);
    if (!added) return std::unexpected(added.error());
    return fw::fixed_cast<Money>(*added, fw::Rounding::exact);
}

inline void test_wide_replacement() {
    constexpr Money h{"100000000000000000000000000"};
    const Money negative_h = Money::from_raw(-h.raw());
    const std::array totals{h, Money::max(), Money::min(), Money::max()};
    const std::array previous{negative_h, Money::min(), Money::max(), Money{}};
    const std::array next{negative_h, Money::min(), Money::max(), raw<Money>(1)};
    std::array<Money, 4> output{};
    const auto result =
        batch::transform(std::span{output}, replacement, std::span{totals}, std::span{previous}, std::span{next});
    CHECK(!result);
    CHECK(result.error().index == 3);
    CHECK(result.error().error == Error::overflow);
    CHECK(output[0] == h);
    CHECK(output[1] == Money::max());
    CHECK(output[2] == Money::min());
    CHECK(output[3] == Money{});
}

inline void test_maintenance_in_place() {
    struct Update {
        Money value, total;
    };
    struct Amounts {
        Money after, old_amount, new_amount;
    };
    struct Axes {
        Price old_price, new_price;
        Money old_fx, new_fx;
    };
    const auto scalar = [](Money slope, Money value, Money total, Axes axes) noexcept -> Expected<Update> {
        const auto old_product = fw::mul_to<Money>(slope, axes.old_price, fw::Rounding::nearest_even);
        if (!old_product) return std::unexpected(old_product.error());
        const auto new_product = fw::mul_to<Money>(slope, axes.new_price, fw::Rounding::nearest_even);
        if (!new_product) return std::unexpected(new_product.error());
        const auto after = replacement(value, *old_product, *new_product);
        if (!after) return std::unexpected(after.error());
        const auto old_amount = fw::mul_to<Money>(value, axes.old_fx, fw::Rounding::nearest_even);
        if (!old_amount) return std::unexpected(old_amount.error());
        const auto new_amount = fw::mul_to<Money>(*after, axes.new_fx, fw::Rounding::nearest_even);
        if (!new_amount) return std::unexpected(new_amount.error());
        const auto sum = replacement(total, *old_amount, *new_amount);
        if (!sum) return std::unexpected(sum.error());
        return Update{*after, *sum};
    };
    constexpr std::array scenarios{Axes{Price{"3"}, Price{"5"}, Money{"1.5"}, Money{"2"}},
                                   Axes{Price::from_raw(1), Price::from_raw(3), Money{"1.2345"}, Money{"0.9"}},
                                   Axes{Price{"-1"}, Price{"-1"}, Money{"1"}, Money{"1"}}};
    constexpr std::array<std::uint32_t, 9> balance{1, 2, 1, 3, 1, 2, 0, 3, 2};
    constexpr std::array initial_slopes{Money{"2"},  Money{"-1"}, Money{"0.005"}, Money{"100000"}, Money{"0.01"},
                                        Money{"-2"}, Money{},     Money{"1"},     Money{"3"}};
    constexpr std::array initial_values{Money{"10"}, Money{"100"}, Money{"-3"}, Money{"100"}, Money{"0.01"},
                                        Money{"1"},  Money{},      Money{"7"},  Money{"-4"}};
    constexpr std::array initial_totals{Money{}, Money{"1000"}, Money{"2000"}, Money{"3000"}};
    for (std::size_t scenario = 0; scenario < scenarios.size(); ++scenario) {
        const auto axes = scenarios[scenario];
        auto slopes = initial_slopes;
        auto values = initial_values;
        auto totals = initial_totals;
        if (scenario == 2) {
            // The removed subtotal is 2e26, but the final per-row value fits.
            slopes[4] = Money{"100000000000000000000000000"};
            values[4] = slopes[4];
        }
        auto expected_values = values;
        auto expected_totals = totals;
        for (std::size_t row = 0; row < values.size(); ++row) {
            const auto reference = scalar(slopes[row], expected_values[row], expected_totals[balance[row]], axes);
            CHECK(reference);
            expected_values[row] = reference->value;
            expected_totals[balance[row]] = reference->total;
        }

        const auto old_price = batch::prepare_mul_to<Money>(axes.old_price, fw::Rounding::nearest_even);
        const auto new_price = batch::prepare_mul_to<Money>(axes.new_price, fw::Rounding::nearest_even);
        const auto old_fx = batch::prepare_mul_to<Money>(axes.old_fx, fw::Rounding::nearest_even);
        const auto new_fx = batch::prepare_mul_to<Money>(axes.new_fx, fw::Rounding::nearest_even);
        const auto map = [=](Money slope, Money value) noexcept -> Expected<Amounts> {
            const auto old_product = old_price(slope);
            if (!old_product) return std::unexpected(old_product.error());
            const auto new_product = new_price(slope);
            if (!new_product) return std::unexpected(new_product.error());
            const auto after = replacement(value, *old_product, *new_product);
            if (!after) return std::unexpected(after.error());
            const auto old_amount = old_fx(value);
            if (!old_amount) return std::unexpected(old_amount.error());
            const auto new_amount = new_fx(*after);
            if (!new_amount) return std::unexpected(new_amount.error());
            return Amounts{*after, *old_amount, *new_amount};
        };
        std::size_t committed = 0;
        const auto commit = [&](std::size_t row, Amounts amounts) noexcept -> Expected<void> {
            CHECK(row == committed);
            // The shared balance is read here, after earlier rows targeting
            // it have committed. Only this row's aliased input is overwritten.
            const auto sum = replacement(totals[balance[row]], amounts.old_amount, amounts.new_amount);
            if (!sum) return std::unexpected(sum.error());
            values[row] = amounts.after;
            totals[balance[row]] = *sum;
            ++committed;
            return {};
        };
        CHECK(batch::for_each<4>(values.size(), map, commit, std::span{slopes}, std::span{values}));
        CHECK(committed == values.size());
        CHECK(values == expected_values);
        CHECK(totals == expected_totals);
        CHECK(totals[0] == Money{});
    }

    // Same-row input writes retain the accepted-prefix contract if commit
    // fails, even when the following mapped row has already failed differently.
    std::array values{raw<Money>(1), raw<Money>(2), raw<Money>(3), raw<Money>(4), raw<Money>(5)};
    const auto original = values;
    const auto map = [](Money value) noexcept -> Expected<Money> {
        if (value == raw<Money>(4)) return std::unexpected(Error::inexact);
        return fw::add(value, raw<Money>(1));
    };
    const auto commit = [&](std::size_t row, Money value) noexcept -> Expected<void> {
        if (row == 2) return std::unexpected(Error::overflow);
        values[row] = value;
        return {};
    };
    const auto result = batch::for_each<4>(values.size(), map, commit, std::span{values});
    CHECK(!result);
    CHECK(result.error().index == 2);
    CHECK(result.error().error == Error::overflow);
    CHECK(values[0] == raw<Money>(2));
    CHECK(values[1] == raw<Money>(3));
    for (std::size_t row = 2; row < values.size(); ++row) CHECK(values[row] == original[row]);
}

inline Expected<Money> anchored(Money slope, Money residual, Price anchor, Price mark, Money fx) noexcept {
    // These tests use the brief's admitted economic price range.
    const auto distance = Price::from_raw(mark.raw() - anchor.raw());
    const auto product_value = fw::mul_to<Money>(slope, distance, fw::Rounding::nearest_even);
    if (!product_value) return std::unexpected(product_value.error());
    const auto pre_fx = fw::add(*product_value, residual);
    if (!pre_fx) return std::unexpected(pre_fx.error());
    return fw::mul_to<Money>(*pre_fx, fx, fw::Rounding::nearest_even);
}

inline void test_anchored_boundaries() {
    constexpr std::array slopes{Money{"1"}, Money{"2"}, Money{"-0.005"}, Money{"100000"}, Money{"0"}};
    constexpr std::array residuals{Money{"1"}, Money{"-2"}, Money::from_raw(1), Money{"100"}, Money{"-0.25"}};
    constexpr std::array anchors{Price{"100"}, Price{"100"}, Price{"99.9999999999"}, Price{"-100"}, Price{"0"}};
    std::array<Money, 5> output{};
    constexpr Price mark{"100"};
    constexpr Money fx{"1.2345"};
    const auto fx_plan = batch::prepare_mul_to<Money>(fx, fw::Rounding::nearest_even);
    const auto map = [=](Money slope, Money residual, Price anchor) noexcept -> Expected<Money> {
        const auto distance = Price::from_raw(mark.raw() - anchor.raw());
        const auto product_value = batch::prepare_mul_to<Money>(distance, fw::Rounding::nearest_even)(slope);
        if (!product_value) return std::unexpected(product_value.error());
        const auto pre_fx = fw::add(*product_value, residual);
        if (!pre_fx) return std::unexpected(pre_fx.error());
        return fx_plan(*pre_fx);
    };
    CHECK(batch::transform(std::span{output}, map, std::span{slopes}, std::span{residuals}, std::span{anchors}));
    for (std::size_t i = 0; i < output.size(); ++i) {
        const auto reference = anchored(slopes[i], residuals[i], anchors[i], mark, fx);
        CHECK(reference);
        CHECK(output[i] == *reference);
    }
    CHECK(output[0] == Money{"1.2345"}); // The first entry's residual is real money.
    CHECK(output[1] == Money{"-2.469"});

    // Neither a cancelling residual nor a small FX rate rescues the explicit
    // out-of-range product boundary in the scalar financial expression.
    constexpr Money huge{"170100000000000000000000000"};
    constexpr std::array huge_slopes{huge, huge, Money{"1"}};
    const std::array huge_residuals{Money{}, Money::from_raw(-huge.raw()), Money{}};
    constexpr std::array huge_anchors{Price{"1"}, Price{"0"}, Price{"0"}};
    constexpr Money sentinel{"-7"};
    std::array<Money, 3> huge_output{sentinel, sentinel, sentinel};
    const auto result = batch::transform(std::span{huge_output}, anchored, std::span{huge_slopes},
                                         std::span{huge_residuals}, std::span{huge_anchors}, Price{"2"}, Money{"0.1"});
    CHECK(!result);
    CHECK(result.error().index == 1);
    CHECK(result.error().error == Error::overflow);
    CHECK(huge_output[0] == Money{"17010000000000000000000000"});
    CHECK(huge_output[1] == Money{"-7"});
    CHECK(huge_output[2] == Money{"-7"});
}

inline void test_ordered_reductions() {
    std::span<Money const> empty;
    const auto empty_sum = batch::reduce_add(empty, Money{"7"});
    CHECK(empty_sum);
    CHECK(*empty_sum == Money{"7"});

    const std::array overflow{Money::max(), raw<Money>(1), raw<Money>(-1)};
    const auto sum = batch::reduce_add(std::span{overflow});
    CHECK(!sum);
    CHECK(sum.error().index == 1);
    CHECK(sum.error().error == Error::overflow);

    constexpr std::array slopes{Money{"0.005"}, Money{"0.005"}};
    const auto per_entry = batch::prepare_mul_to<Money>(Price::from_raw(1), fw::Rounding::nearest_even);
    const auto rounded_sum = batch::transform_reduce(slopes.size(), Money{}, per_entry, std::span{slopes});
    CHECK(rounded_sum);
    CHECK(*rounded_sum == Money{});
    const auto combined =
        fw::mul_to<Money>(*fw::add(slopes[0], slopes[1]), Price::from_raw(1), fw::Rounding::nearest_even);
    CHECK(combined && combined->raw() == Raw{1});

    constexpr std::array entry_slopes{Money{"1"}, Money{"2"}, Money{"0.005"}, Money{"0.005"}, Money{"-1"}};
    constexpr std::array execution{Price{"99"}, Price{"101"}, Price{"99.9999999999"}, Price{"99.9999999999"},
                                   Price{"99"}};
    constexpr std::array<std::size_t, 4> boundaries{0, 2, 4, 5};
    const auto entry = [](Money slope, Price execution_price) noexcept -> Expected<Money> {
        return fw::mul_to<Money>(slope, Price::from_raw(Price{"100"}.raw() - execution_price.raw()),
                                 fw::Rounding::nearest_even);
    };
    std::array<Money, 3> positions{};
    for (std::size_t p = 0; p < positions.size(); ++p) {
        const auto count = boundaries[p + 1] - boundaries[p];
        const auto position =
            batch::transform_reduce(count, Money{}, entry, std::span{entry_slopes}.subspan(boundaries[p], count),
                                    std::span{execution}.subspan(boundaries[p], count));
        CHECK(position);
        positions[p] = *position;
    }
    CHECK(batch::mul_to<Money>(std::span{positions}, std::span{positions}, Money{"1.25"}, fw::Rounding::nearest_even));
    CHECK(positions[0] == Money{"-1.25"});
    CHECK(positions[1] == Money{});
    CHECK(positions[2] == Money{"-1.25"});

    // Clipping remains a caller decision after each position's rounded sum
    // and FX conversion. Individual positive entries must not leak through.
    std::array<Money, 3> positive_positions{};
    CHECK(batch::transform(
        std::span{positive_positions}, [](Money value) noexcept { return value > Money{} ? value : Money{}; },
        std::span{positions}));
    const auto clipped_total = batch::reduce_add(std::span{positive_positions});
    CHECK(clipped_total && *clipped_total == Money{});

    // Ordered reduction failure also precedes a later speculative map error.
    const std::array terms{raw<Money>(1), raw<Money>(2), raw<Money>(3), raw<Money>(4)};
    const auto map = [](Money value) noexcept -> Expected<Money> {
        if (value == raw<Money>(2)) return std::unexpected(Error::inexact);
        return value;
    };
    const auto first_failure = batch::transform_reduce<4>(terms.size(), Money::max(), map, std::span{terms});
    CHECK(!first_failure);
    CHECK(first_failure.error().index == 0);
    CHECK(first_failure.error().error == Error::overflow);
}

inline void test_wide_preparation() {
    using Product24 = fw::Fixed256<24>;
    using Product36 = fw::Fixed256<36>;
    const auto slope = [](Money volume, Money contract, Money numerator, Money conversion,
                          Money denominator) noexcept -> Expected<Money> {
        const auto units = batch::mul_to<Product24>(fw::Rounding::exact)(volume, contract);
        if (!units) return std::unexpected(units.error());
        const auto product_value = batch::mul_to<Product36>(fw::Rounding::exact)(*units, numerator);
        if (!product_value) return std::unexpected(product_value.error());
        return fw::mul_div_to<Money>(*product_value, conversion, denominator, fw::Rounding::nearest_even);
    };
    constexpr std::array volumes{Money{"0.5"}, Money{"0.5"}, Money{"0.5"}, Money{"0.5"}};
    constexpr std::array numerators{Money{"1"}, Money{"-1"}, Money{"1"}, Money{"1"}};
    constexpr std::array denominators{Money{"1"}, Money{"2"}, Money{}, Money{"1"}};
    constexpr Money sentinel{"-7"};
    std::array<Money, 4> output{sentinel, sentinel, sentinel, sentinel};
    const auto result = batch::transform(std::span{output}, slope, std::span{volumes}, Money{"100000"},
                                         std::span{numerators}, Money{"1.234"}, std::span{denominators});
    CHECK(!result);
    CHECK(result.error().index == 2);
    CHECK(result.error().error == Error::division_by_zero);
    CHECK(output[0] == Money{"61700"});
    CHECK(output[1] == Money{"-30850"});
    CHECK(output[2] == Money{"-7"});
    CHECK(output[3] == Money{"-7"});

    // A later division could make the final slope fit, but it cannot rescue
    // an explicitly out-of-range Product36 intermediate. That earlier row
    // also outranks a following row's speculative zero-divisor failure.
    constexpr Money large{"100000000000000000000"};
    constexpr std::array large_volumes{large, large, Money{"1"}};
    constexpr std::array large_contracts{Money{"1"}, large, Money{"1"}};
    constexpr std::array large_numerators{Money{"1"}, Money{"10"}, Money{"1"}};
    constexpr std::array large_denominators{Money{"1"}, large, Money{}};
    std::array<Money, 3> large_output{sentinel, sentinel, sentinel};
    const auto large_result =
        batch::transform<3>(std::span{large_output}, slope, std::span{large_volumes}, std::span{large_contracts},
                            std::span{large_numerators}, Money{"0.000000000001"}, std::span{large_denominators});
    CHECK(!large_result);
    CHECK(large_result.error().index == 1);
    CHECK(large_result.error().error == Error::overflow);
    CHECK(large_output[0] == Money{"100000000"});
    CHECK(large_output[1] == Money{"-7"});
    CHECK(large_output[2] == Money{"-7"});

    // Percentage cross-products require more than signed128 raw bits. The
    // batch transform preserves exact, inclusive comparisons at equality.
    constexpr Money high{"100000000000000000000"};
    const std::array equity{high, Money::from_raw(high.raw() - Raw{1}), Money::from_raw(high.raw() + Raw{1})};
    std::array<Product24, 3> left{}, right{};
    CHECK(batch::mul_to<Product24>(std::span{left}, std::span{equity}, Money{"100"}, fw::Rounding::exact));
    CHECK(batch::mul_to<Product24>(std::span{right}, high, Money{"100"}, fw::Rounding::exact));
    std::array<bool, 3> crossing{};
    CHECK(batch::transform(
        std::span{crossing}, [](Product24 a, Product24 b) noexcept { return a <= b; }, std::span{left},
        std::span{right}));
    CHECK(crossing[0]);
    CHECK(crossing[1]);
    CHECK(!crossing[2]);
}

template<class A, class B, class C>
void check_cold_half_quanta() {
    // Named locals avoid MSVC's consteval-temporary/local-aggregate issue in
    // instantiated templates (also exercised by test_literal_header_only).
    const A positive_half{"0.000000000001"}, negative_half{"-0.000000000001"};
    const A positive_three_halves{"0.000000000003"}, negative_three_halves{"-0.000000000003"};
    const std::array values{positive_half, negative_half, positive_three_halves, negative_three_halves};
    const B factor{"1"};
    const C denominator{"2"};
    const Money sentinel = raw<Money>(777);
    for (auto mode : modes) {
        std::array<Money, 4> output{sentinel, sentinel, sentinel, sentinel};
        const auto result = batch::mul_div_to<Money>(std::span{output}, std::span{values}, factor, denominator, mode);
        if (mode == fw::Rounding::exact) {
            CHECK(!result);
            CHECK(result.error().index == 0);
            CHECK(result.error().error == Error::inexact);
            for (auto value : output) CHECK(value == sentinel);
            continue;
        }
        CHECK(result);
        for (std::size_t row = 0; row < values.size(); ++row) {
            const bool negative = row % 2 != 0;
            std::int64_t magnitude = row / 2;
            const bool increment =
                mode == fw::Rounding::nearest_away || (mode == fw::Rounding::nearest_even && magnitude == 1) ||
                (mode == fw::Rounding::floor && negative) || (mode == fw::Rounding::ceil && !negative);
            magnitude += increment;
            const Money expected = raw<Money>(negative ? -magnitude : magnitude);
            CHECK(output[row] == expected);
            CHECK(fw::mul_div_to<Money>(values[row], factor, denominator, mode) == Expected<Money>{expected});
        }
    }
}

inline void test_cold_division_boundaries() {
    using Product22 = fw::Fixed256<22>;
    using Product24 = fw::Fixed256<24>;
    using Product36 = fw::Fixed256<36>;
    check_cold_half_quanta<Product36, Money, Money>();
    check_cold_half_quanta<Product24, Product22, Money>();
    check_cold_half_quanta<Product36, Product36, Product24>();
    check_cold_half_quanta<Product36, Product24, Product22>();
    check_cold_half_quanta<Product24, fw::Fixed128<20>, Price>();

    using Displacement = fw::Fixed256<10>;
    const auto root = [](WideMoney residual, Money divisor, Price anchor) noexcept -> Expected<Price> {
        const auto displacement = fw::div_to<Displacement>(residual, divisor, fw::Rounding::floor);
        if (!displacement) return std::unexpected(displacement.error());
        const auto added = fw::add_to<Displacement>(*displacement, anchor, fw::Rounding::exact);
        if (!added) return std::unexpected(added.error());
        return fw::fixed_cast<Price>(*added, fw::Rounding::exact);
    };
    // The first two displacements do not fit Price, but adding their anchors
    // while still wide produces valid final roots. The third root overflows;
    // the fourth row's zero divisor must not change that first failure.
    constexpr std::array residuals{WideMoney{"1000000000"}, WideMoney{"-1000000000"}, WideMoney{"1000000000"},
                                   WideMoney{"1"}};
    constexpr std::array divisors{Money{"1"}, Money{"1"}, Money{"1"}, Money{}};
    constexpr std::array anchors{Price{"-100000000"}, Price{"100000000"}, Price{}, Price{}};
    const Price sentinel{"7"};
    std::array<Price, 4> output{sentinel, sentinel, sentinel, sentinel};
    const auto result =
        batch::transform<4>(std::span{output}, root, std::span{residuals}, std::span{divisors}, std::span{anchors});
    CHECK(!result);
    CHECK(result.error().index == 2);
    CHECK(result.error().error == Error::overflow);
    CHECK(output[0] == Price{"900000000"});
    CHECK(output[1] == Price{"-900000000"});
    CHECK(output[2] == sentinel);
    CHECK(output[3] == sentinel);
}

inline void test_generic_conversion_and_comparison() {
    constexpr std::array values{Money{"1.25"}, Money{"-0.1"}, Money::max(), Money::min(), Money{}};
    std::array<double, 5> converted{};
    CHECK(batch::transform(
        std::span{converted}, [](Money value) noexcept { return fw::to_double(value); }, std::span{values}));
    for (std::size_t i = 0; i < values.size(); ++i) CHECK(converted[i] == fw::to_double(values[i]));
    std::array<bool, 5> positive{};
    CHECK(
        batch::transform(std::span{positive}, [](Money value) noexcept { return value > Money{}; }, std::span{values}));
    for (std::size_t i = 0; i < values.size(); ++i) CHECK(positive[i] == (values[i] > Money{}));
    std::array<Money, 5> back{};
    const auto result = batch::transform(
        std::span{back},
        [](double value) noexcept { return fw::from_double<Money>(value, fw::Rounding::nearest_even); },
        std::span{converted});
    for (std::size_t i = 0; i < back.size(); ++i) {
        const auto scalar = fw::from_double<Money>(converted[i], fw::Rounding::nearest_even);
        if (!scalar) {
            CHECK(!result);
            CHECK(result.error().index == i);
            CHECK(result.error().error == scalar.error());
            break;
        }
        CHECK(back[i] == *scalar);
    }
}

template<class Dest, std::size_t Out, class A, std::size_t In, class B>
batch::Result scalar_broadcast(std::span<Dest, Out> output, std::span<const A, In> input, B factor,
                               fw::Rounding mode = fw::Rounding::exact) {
    for (std::size_t i = 0; i < input.size(); ++i) {
        const auto value = fw::mul_to<Dest>(input[i], factor, mode);
        if (!value) return std::unexpected(batch::Error{i, value.error()});
        output[i] = *value;
    }
    return {};
}

// A block-capable prepared map must preserve the same public contract on both
// vector-capable machines and portable/scalar builds. The intentionally offset
// fields require only Money's ordinary alignment, never 16/32-byte alignment.
template<std::size_t BlockSize>
void check_prepared_blocks() {
    constexpr Raw scale{10'000'000'000LL};
    constexpr std::array<std::int64_t, 13> coefficients{0,
                                                        1,
                                                        -1,
                                                        INT32_MIN,
                                                        INT32_MAX,
                                                        std::int64_t{INT32_MIN} - 1,
                                                        std::int64_t{INT32_MAX} + 1,
                                                        std::int64_t{UINT32_MAX} - 1,
                                                        UINT32_MAX,
                                                        -std::int64_t{UINT32_MAX},
                                                        std::int64_t{UINT32_MAX} + 1,
                                                        INT64_MIN,
                                                        INT64_MAX};
    struct alignas(32) Storage {
        std::uint64_t guard = 0x13579bdf;
        std::array<Money, 40> values{};
    } input, output;
    static_assert(offsetof(Storage, values) == 8);
    CHECK(reinterpret_cast<std::uintptr_t>(input.values.data()) % 32 == 8);
    CHECK(reinterpret_cast<std::uintptr_t>(output.values.data()) % 32 == 8);
    for (std::size_t i = 0; i < input.values.size(); ++i)
        input.values[i] = Money::from_raw(Raw{coefficients[i % coefficients.size()]} * scale);
    const auto sentinel = Money::from_raw(Raw{77});
    for (const auto length : std::array<std::size_t, 13>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 17, 31, 39}) {
        for (const auto p : std::array<std::int64_t, 8>{0, 37, -37, INT32_MIN, INT32_MAX, (std::int64_t{1} << 43) + 17,
                                                        INT64_MIN, INT64_MAX}) {
            output.values.fill(sentinel);
            auto expected = output.values;
            const auto factor = Price::from_raw(p);
            const std::span<const Money> source{input.values.data(), length};
            const auto reference = scalar_broadcast(std::span{expected}.first(length), source, factor);
            const auto result =
                batch::transform<BlockSize>(std::span{output.values}.first(length),
                                            batch::prepare_mul_to<Money>(factor, fw::Rounding::exact), source);
            CHECK(result == reference);
            CHECK(output.values == expected);
            CHECK(input.guard == 0x13579bdf && output.guard == 0x13579bdf);
        }
    }

    // Every physical position, including the tail, can be the first failure.
    // Oversized quotients must report overflow before checking exactness.
    for (const bool overflow : {false, true}) {
        for (std::size_t failed = 0; failed < 9; ++failed) {
            std::array<Money, 9> source;
            source.fill(Money::from_raw(scale));
            source[failed] = overflow ? Money::max() : Money::from_raw(1);
            const auto factor = Price::from_raw(overflow ? 20'000'000'001LL : 37);
            std::array<Money, 9> actual;
            actual.fill(sentinel);
            auto expected = actual;
            const auto reference = scalar_broadcast(std::span{expected}, std::span<const Money>{source}, factor);
            const auto result = batch::transform<BlockSize>(
                std::span{actual}, batch::prepare_mul_to<Money>(factor, fw::Rounding::exact), std::span{source});
            CHECK(!reference);
            CHECK(reference.error().index == failed);
            CHECK(reference.error().error == (overflow ? Error::overflow : Error::inexact));
            CHECK(result == reference);
            CHECK(actual == expected);
        }
    }

    // In-place input/output aliases are exact, even when their common address
    // is offset within an allocation. This is not shifted input/output overlap.
    for (std::size_t offset : {0, 1}) {
        for (std::size_t length : {3, 4, 5, 8, 9}) {
            input.values.fill(Money::from_raw(scale));
            input.values[offset + length - 1] = Money::from_raw(1);
            auto expected = input.values;
            const auto original = input.values;
            const auto factor = Price::from_raw(37);
            const auto reference = scalar_broadcast(std::span{expected}.subspan(offset, length),
                                                    std::span<const Money>{original}.subspan(offset, length), factor);
            const auto in_place = std::span{input.values}.subspan(offset, length);
            const auto result = batch::transform<BlockSize>(
                in_place, batch::prepare_mul_to<Money>(factor, fw::Rounding::exact), in_place);
            CHECK(result == reference);
            CHECK(input.values == expected);
        }
    }
}

template<unsigned Gap>
void check_prepared_scale_gap() {
    using A = fw::Fixed128<Gap>;
    using Dest = fw::Fixed128<0>;
    using Factor = fw::Fixed64<0>;
    const Raw scale = fw::compute_pow10<Raw>(Gap);
    const Raw half = scale >> 1;
    std::array<A, 12> source{A::from_raw(scale),
                             A::from_raw(-scale),
                             A::from_raw(Raw{INT32_MIN} * scale),
                             A::from_raw(Raw{UINT32_MAX} * scale),
                             A::from_raw(-Raw{UINT32_MAX} * scale),
                             A::from_raw((Raw{UINT32_MAX} + Raw{1}) * scale),
                             A::from_raw(half),
                             A::from_raw(-half),
                             A::from_raw(scale + half),
                             A::from_raw(1),
                             A::max(),
                             A::min()};
    std::array<Dest, 12> actual;
    for (const auto mode : modes) {
        for (const auto p : std::array<std::int64_t, 6>{1, 2, -3, INT32_MIN, INT64_MIN, INT64_MAX}) {
            actual.fill(Dest::from_raw(77));
            auto expected = actual;
            const auto factor = Factor::from_raw(p);
            const auto reference = scalar_broadcast(std::span{expected}, std::span<const A>{source}, factor, mode);
            const auto result = batch::transform<4>(std::span{actual}, batch::prepare_mul_to<Dest>(factor, mode),
                                                    std::span<const A>{source});
            CHECK(result == reference);
            CHECK(actual == expected);
        }
    }
    // A failed grid certificate must fall back, not report inexact: the
    // broadcast factor can cancel the source's otherwise nonzero remainder.
    source.fill(A::from_raw(half));
    CHECK(batch::transform<4>(std::span{actual}, batch::prepare_mul_to<Dest>(Factor::from_raw(2), fw::Rounding::exact),
                              std::span{source}));
    for (const auto value : actual) CHECK(value == Dest::from_raw(1));
}

template<std::size_t BlockSize>
void check_prepared_ordered_commit() {
    constexpr Raw scale{10'000'000'000LL};
    std::array<Money, 9> slope;
    slope.fill(Money::from_raw(scale));
    slope[2] = Money::from_raw(1); // later map refusal must not outrank commit(1)
    std::array<Money, 9> values{};
    Money total = Money::from_raw(Money::max().raw() - Raw{1});
    std::size_t calls = 0;
    const auto result = batch::for_each<BlockSize>(
        slope.size(), batch::prepare_mul_to<Money>(Price::from_raw(1), fw::Rounding::exact),
        [&](std::size_t row, Money contribution) noexcept -> std::expected<void, Error> {
            CHECK(row == calls++);
            const auto next_value = fw::add(values[row], contribution);
            if (!next_value) return std::unexpected(next_value.error());
            const auto next_total = fw::add(total, contribution);
            if (!next_total) return std::unexpected(next_total.error());
            values[row] = *next_value;
            total = *next_total;
            return {};
        },
        std::span{slope});
    CHECK(!result && result.error() == batch::Error{1, Error::overflow});
    CHECK(calls == 2);
    CHECK(total == Money::max());
    CHECK(values[0] == Money::from_raw(1));
    for (std::size_t i = 1; i < values.size(); ++i) CHECK(values[i] == Money{});

    constexpr std::array<std::size_t, 9> balance{0, 1, 0, 2, 1, 0, 2, 3, 0};
    constexpr std::array<std::int64_t, 9> q{1, -2, 3, 4, -5, -6, 7, 0, 8};
    for (std::size_t i = 0; i < slope.size(); ++i) slope[i] = Money::from_raw(Raw{q[i]} * scale);
    values.fill(Money{});
    std::array<Money, 4> totals{};
    std::array<Money, 4> expected{};
    const auto factor = Price::from_raw(37);
    for (std::size_t i = 0; i < slope.size(); ++i) {
        const auto value = fw::mul_to<Money>(slope[i], factor, fw::Rounding::exact);
        CHECK(value);
        expected[balance[i]] = *fw::add(expected[balance[i]], *value);
    }
    calls = 0;
    CHECK(batch::for_each<BlockSize>(
        slope.size(), batch::prepare_mul_to<Money>(factor, fw::Rounding::exact),
        [&](std::size_t row, Money value) noexcept -> std::expected<void, Error> {
            CHECK(row == calls++);
            const auto sum = fw::add(totals[balance[row]], value);
            if (!sum) return std::unexpected(sum.error());
            values[row] = value;
            totals[balance[row]] = *sum;
            return {};
        },
        std::span{slope}));
    CHECK(calls == slope.size());
    CHECK(totals == expected);
    CHECK(totals[3] == Money{}); // zero hole targets the sink
}

template<unsigned Gap>
void check_prepared_commit_scale_gap() {
    using A = fw::Fixed128<Gap>;
    using Dest = fw::Fixed128<0>;
    using Factor = fw::Fixed64<0>;
    const Raw scale = fw::compute_pow10<Raw>(Gap);
    constexpr std::array<std::size_t, 9> balance{0, 1, 0, 1, 0, 1, 0, 2, 1};
    for (const bool partial : {false, true}) {
        for (const bool commit_fails : {false, true}) {
            for (const auto p : std::array<std::int64_t, 3>{37, (std::int64_t{1} << 43) + 18, INT64_MIN}) {
                std::array<A, 9> source;
                for (std::size_t row = 0; row < source.size(); ++row)
                    source[row] = A::from_raw(Raw{static_cast<std::int64_t>(row + 1)} * scale);
                source[7] = A{};
                if (partial) {
                    source[3] = A::from_raw((Raw{UINT32_MAX} + Raw{1}) * scale);
                    source[4] = A::from_raw(scale >> 1);
                }
                std::array<Dest, 9> values{}, expected_values{};
                std::array<Dest, 3> totals{};
                if (commit_fails) totals[0] = Dest::from_raw((p < 0 ? Dest::min().raw() : Dest::max().raw()) - Raw{p});
                auto expected_totals = totals;
                const auto commit_to = [&](auto& rows, auto& sums, std::size_t row,
                                           Dest contribution) -> std::expected<void, Error> {
                    const auto value = fw::add(rows[row], contribution);
                    if (!value) return std::unexpected(value.error());
                    const auto total = fw::add(sums[balance[row]], contribution);
                    if (!total) return std::unexpected(total.error());
                    rows[row] = *value;
                    sums[balance[row]] = *total;
                    return {};
                };
                const auto factor = Factor::from_raw(p);
                batch::Result reference;
                std::size_t expected_calls = 0;
                for (std::size_t row = 0; row < source.size(); ++row) {
                    const auto value = fw::mul_to<Dest>(source[row], factor, fw::Rounding::exact);
                    if (!value) {
                        reference = std::unexpected(batch::Error{row, value.error()});
                        break;
                    }
                    ++expected_calls;
                    const auto committed = commit_to(expected_values, expected_totals, row, *value);
                    if (!committed) {
                        reference = std::unexpected(batch::Error{row, committed.error()});
                        break;
                    }
                }
                std::size_t calls = 0;
                const auto result = batch::for_each<8>(
                    source.size(), batch::prepare_mul_to<Dest>(factor, fw::Rounding::exact),
                    [&](std::size_t row, const Dest& value) noexcept -> std::expected<void, Error> {
                        CHECK(row == calls++);
                        return commit_to(values, totals, row, value);
                    },
                    std::span<const A>{source});
                CHECK(result == reference);
                CHECK(calls == expected_calls);
                CHECK(values == expected_values);
                CHECK(totals == expected_totals);
                CHECK(totals[2] == Dest{});
                if (commit_fails) CHECK(!result && result.error() == batch::Error{2, Error::overflow});
            }
        }
    }
}

inline void test_prepared_callback_exceptions() {
    constexpr Raw scale{10'000'000'000LL};
    std::array<Money, 5> input;
    input.fill(Money::from_raw(scale));
    const auto map = batch::prepare_mul_to<Money>(Price::from_raw(1), fw::Rounding::exact);
    const auto nonthrowing = [](std::size_t, Money) noexcept -> std::expected<void, Error> { return {}; };
    const auto throwing_signature = [](std::size_t, Money) -> std::expected<void, Error> { return {}; };
    static_assert(noexcept(batch::for_each<4>(input.size(), map, nonthrowing, std::span{input})));
    static_assert(!noexcept(batch::for_each<4>(input.size(), map, throwing_signature, std::span{input})));
    CHECK(batch::for_each<4>(input.size(), map, throwing_signature, std::span{input}));
#if defined(__cpp_exceptions) || defined(_CPPUNWIND)
    struct CallbackFailure {};
    std::size_t calls = 0;
    bool caught = false;
    try {
        const auto result = batch::for_each<4>(
            input.size(), map,
            [&](std::size_t row, Money value) -> std::expected<void, Error> {
                CHECK(row == calls++);
                CHECK(value == Money::from_raw(1));
                if (row == 2) throw CallbackFailure{};
                return {};
            },
            std::span{input});
        (void)result;
    } catch (const CallbackFailure&) {
        caught = true;
    }
    CHECK(caught);
    CHECK(calls == 3);
#endif
}

inline void run() {
    test_multiply_domains();
    check_transform_contract<1>();
    check_transform_contract<4>();
    check_transform_contract<8>();
    test_in_place();
    test_aggregate_map();
    test_operation_wrappers();
    check_ordered_commit<1>();
    check_ordered_commit<4>();
    check_ordered_commit<8>();
    test_scatter_delta();
    test_wide_replacement();
    test_maintenance_in_place();
    test_anchored_boundaries();
    test_ordered_reductions();
    test_wide_preparation();
    test_cold_division_boundaries();
    test_generic_conversion_and_comparison();
    check_prepared_blocks<4>();
    check_prepared_blocks<5>();
    check_prepared_blocks<8>();
    check_prepared_scale_gap<1>();
    check_prepared_scale_gap<9>();
    check_prepared_scale_gap<10>();
    check_prepared_scale_gap<19>();
    check_prepared_ordered_commit<4>();
    check_prepared_ordered_commit<8>();
    check_prepared_commit_scale_gap<1>();
    check_prepared_commit_scale_gap<19>();
    test_prepared_callback_exceptions();
}

} // namespace batch_test
