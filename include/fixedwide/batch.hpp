#pragma once

/// \file
/// Checked arithmetic over existing SoA fields. Scalars broadcast; spans name
/// contiguous fields. Independent calculations run in small stack blocks and
/// their results are committed in physical row order. No allocation, worker
/// threads, special alignment, persistent packing, or change to scalar storage.

#include <fixedwide/arithmetic.hpp>
#include <fixedwide/mixed.hpp>
#include <fixedwide/detail/batch_multiply.hpp>
#include <array>
#include <cassert>
#include <cstddef>
#include <expected>
#include <span>
#include <type_traits>
#include <utility>

// Keep the callback/expected boundary visible to Clang. Outlining a block
// materializes all expected objects and loses the caller's constant rounding
// mode before the native multiply is optimized. GCC already inlines these.
#if defined(__clang__)
#define FIXEDWIDE_BATCH_INLINE [[gnu::always_inline]]
#define FIXEDWIDE_BATCH_CALL [[clang::always_inline]]
#else
#define FIXEDWIDE_BATCH_INLINE
#define FIXEDWIDE_BATCH_CALL
#endif

namespace fixedwide::batch {

/// The first failed physical row and its unchanged scalar error.
struct Error {
    std::size_t index;
    ArithmeticError error;

    friend constexpr bool operator==(Error, Error) noexcept = default;
};

using Result = std::expected<void, Error>;

namespace detail {

template<class T>
inline constexpr bool is_span = false;
template<class T, std::size_t Extent>
inline constexpr bool is_span<std::span<T, Extent>> = true;

template<class Input>
[[nodiscard]] constexpr decltype(auto) at(const Input& input, std::size_t index) noexcept {
    if constexpr (is_span<Input>)
        return std::as_const(input[index]);
    else
        return (input);
}

template<class Input>
constexpr void check_size([[maybe_unused]] const Input& input, [[maybe_unused]] std::size_t count) noexcept {
    if constexpr (is_span<Input>) assert(input.size() == count);
}

template<class T>
struct mapped_result {
    using value_type = T;
    static constexpr bool checked = false;
};
template<class T, class E>
struct mapped_result<std::expected<T, E>> {
    static_assert(std::is_same_v<E, ArithmeticError>, "batch map errors must be ArithmeticError");
    using value_type = T;
    static constexpr bool checked = true;
};

template<class Map, class... Inputs>
using mapped_type = std::remove_cvref_t<decltype(std::declval<Map&>()(at(std::declval<const Inputs&>(), 0)...))>;
template<class Map, class... Inputs>
using value_type = typename mapped_result<mapped_type<Map, Inputs...>>::value_type;

template<class Map, class... Inputs>
[[nodiscard]] FIXEDWIDE_BATCH_INLINE constexpr auto
evaluate(Map& map, std::size_t index, const Inputs&... inputs) noexcept(noexcept(map(at(inputs, index)...)))
    -> std::expected<value_type<Map, Inputs...>, ArithmeticError> {
    // Inline the callback itself as well as this wrapper. Otherwise a larger
    // composed map can still materialize expected and call out once per row.
    FIXEDWIDE_BATCH_CALL return map(at(inputs, index)...);
}

template<class Commit, class Value>
[[nodiscard]] FIXEDWIDE_BATCH_INLINE constexpr std::expected<void, ArithmeticError>
commit_one(Commit& commit, std::size_t index,
           const Value& value) noexcept(noexcept(commit(std::size_t{index}, value))) {
    FIXEDWIDE_BATCH_CALL return commit(std::size_t{index}, value);
}

template<class Map, class... Inputs, std::size_t... Lane>
[[nodiscard]] FIXEDWIDE_BATCH_INLINE constexpr auto
evaluate_block(Map& map, std::size_t first, std::index_sequence<Lane...>,
               const Inputs&... inputs) noexcept(noexcept(evaluate(map, first, inputs...))) {
    using LaneResult = std::expected<value_type<Map, Inputs...>, ArithmeticError>;
    // List initialization sequences the calls. Keeping their expected results
    // local lets the compiler overlap independent arithmetic before committing.
    return std::array<LaneResult, sizeof...(Lane)>{evaluate(map, first + Lane, inputs...)...};
}

} // namespace detail

/// Map independent rows, then commit each successful row in ascending order.
///
/// `map(inputs[i]...)` returns T or expected<T, ArithmeticError>. A scalar input
/// is copied once and broadcast. `commit(i, value)` returns expected<void,
/// ArithmeticError>. Every input span must have exactly `count` elements.
///
/// A map MUST be free of externally observable side effects. Its result must
/// not depend on earlier commits: up to BlockSize rows can be evaluated ahead. Commit
/// runs only for the accepted prefix. Its failure at row i takes precedence
/// over a speculative map failure at i+1. Commit must check its whole row before
/// writing if the caller needs row-atomic updates. Earlier committed rows stay
/// committed on failure; this is not an array transaction.
///
/// Commit(i) may update input element i after mapping it, provided that storage
/// cannot be read by any other row. Shared running state belongs in commit.
/// `transform` permits exact same-type input/output aliasing on this basis.
/// Shifted/partial overlap is unsupported. Ordinary type alignment suffices.
/// BlockSize=1 is the default and disables speculative mapping. Larger blocks
/// expose independent calculations; choose them using full-kernel measurements.
template<std::size_t BlockSize = 1, class Map, class Commit, class... Inputs>
[[nodiscard]] FIXEDWIDE_BATCH_INLINE constexpr Result
for_each(std::size_t count, Map&& map, Commit&& commit,
         Inputs... inputs) noexcept(noexcept(detail::evaluate(map, 0, inputs...)) &&
                                    noexcept(commit(std::size_t{},
                                                    std::declval<const detail::value_type<Map, Inputs...>&>()))) {
    static_assert(BlockSize > 0 && BlockSize <= 64, "batch block size must be in [1, 64]");
    using Value = detail::value_type<Map, Inputs...>;
    static_assert(!std::is_void_v<Value>, "batch map must produce a value");
    static_assert(std::is_nothrow_copy_constructible_v<Value>, "batch values must be nonthrowing to copy");
    static_assert(std::is_nothrow_move_constructible_v<Value>, "batch values must be nonthrowing to move");
    static_assert(std::is_nothrow_destructible_v<Value>, "batch values must be nonthrowing to destroy");
    static_assert((std::is_nothrow_copy_constructible_v<Inputs> && ...), "batch inputs must be nonthrowing to copy");
    static_assert(std::is_same_v<std::remove_cvref_t<decltype(commit(std::size_t{}, std::declval<const Value&>()))>,
                                 std::expected<void, ArithmeticError>>,
                  "batch commit must return expected<void, ArithmeticError>");
    (detail::check_size(inputs, count), ...);

    std::size_t first = 0;
    for (; count - first >= BlockSize; first += BlockSize) {
        const auto block = detail::evaluate_block(map, first, std::make_index_sequence<BlockSize>{}, inputs...);
        for (std::size_t lane = 0; lane < BlockSize; ++lane) {
            if (!block[lane]) [[unlikely]]
                return std::unexpected(Error{first + lane, block[lane].error()});
            const auto result = detail::commit_one(commit, first + lane, *block[lane]);
            if (!result) [[unlikely]]
                return std::unexpected(Error{first + lane, result.error()});
        }
    }
    for (; first < count; ++first) {
        const auto value = detail::evaluate(map, first, inputs...);
        if (!value) [[unlikely]]
            return std::unexpected(Error{first, value.error()});
        const auto result = detail::commit_one(commit, first, *value);
        if (!result) [[unlikely]]
            return std::unexpected(Error{first, result.error()});
    }
    return {};
}

/// Transform independent rows into caller-owned output. On failure i, output
/// [0,i) is written and [i,size) is unchanged. Exact in-place aliasing with any
/// same-type input is supported; all other output/input overlap is forbidden.
/// Inputs are spans of the output's size or copied scalar broadcasts.
template<std::size_t BlockSize = 1, class Dest, std::size_t Extent, class Map, class... Inputs>
[[nodiscard]] FIXEDWIDE_BATCH_INLINE constexpr Result
transform(std::span<Dest, Extent> output, Map&& map,
          Inputs... inputs) noexcept(noexcept(detail::evaluate(map, 0, inputs...))) {
    static_assert(!std::is_const_v<Dest>, "batch output must be writable");
    static_assert(std::is_same_v<Dest, detail::value_type<Map, Inputs...>>, "batch map must produce the output type");
    static_assert(std::is_nothrow_copy_constructible_v<Dest>, "batch values must be nonthrowing to copy");
    static_assert(std::is_nothrow_destructible_v<Dest>, "batch values must be nonthrowing to destroy");
    static_assert(std::is_nothrow_copy_assignable_v<Dest>, "batch output assignment must not throw");
    static_assert(BlockSize > 0 && BlockSize <= 64, "batch block size must be in [1, 64]");
    if constexpr (!detail::mapped_result<detail::mapped_type<Map, Inputs...>>::checked) {
        // An infallible map has no error flags or accepted-prefix decision to
        // retain. A canonical loop exposes comparison/select and conversions
        // directly to the vectorizer, without expected's representation.
        (detail::check_size(inputs, output.size()), ...);
        for (std::size_t row = 0; row < output.size(); ++row) {
            const auto value = map(detail::at(inputs, row)...);
            output[row] = value;
        }
        return {};
    } else {
        return batch::for_each<BlockSize>(
            output.size(), std::forward<Map>(map),
            [output](std::size_t row, const Dest& value) noexcept -> std::expected<void, ArithmeticError> {
                output[row] = value;
                return {};
            },
            inputs...);
    }
}

/// An optimized, checked binary multiply operation for composing row kernels.
/// Its scalar result, rounding and error precedence equal fixedwide::mul_to.
template<class Dest>
[[nodiscard]] constexpr auto mul_to(Rounding rounding = Rounding::nearest_even) noexcept {
    return fixedwide::detail::batch::binary_multiplier<Dest>{rounding};
}

/// Prepare a shared multiplier once, then call the returned operation on each
/// row. The plan owns its operand; changing the original factor has no effect.
/// Plans are immutable/copyable and retain the full scalar domain and fallback.
template<class Dest, class B>
[[nodiscard]] FIXEDWIDE_BATCH_INLINE inline auto prepare_mul_to(B factor,
                                                                Rounding rounding = Rounding::nearest_even) noexcept {
    return fixedwide::detail::batch::broadcast_multiplier<Dest, B>{factor, rounding};
}

/// Vector-vector or vector-broadcast multiply, rounded separately in every row.
/// A scalar operand prepares its rescaling once for the entire call.
template<class Dest, std::size_t Extent, class A, class B>
[[nodiscard]] FIXEDWIDE_BATCH_INLINE inline Result mul_to(std::span<Dest, Extent> output, A a, B b,
                                                          Rounding rounding = Rounding::nearest_even) noexcept {
    if constexpr (!detail::is_span<B>) {
        return batch::transform(output, batch::prepare_mul_to<Dest>(b, rounding), a);
    } else if constexpr (!detail::is_span<A>) {
        return batch::transform(output, batch::prepare_mul_to<Dest>(a, rounding), b);
    } else {
        return batch::transform(output, batch::mul_to<Dest>(rounding), a, b);
    }
}

/// Checked same-type addition, with ordered accepted-prefix output.
template<class Dest, std::size_t Extent, class A, class B>
[[nodiscard]] constexpr Result add(std::span<Dest, Extent> output, A a, B b) noexcept {
    return batch::transform(output, [](Dest x, Dest y) noexcept { return fixedwide::add(x, y); }, a, b);
}

template<class Dest, std::size_t Extent, class A, class B>
[[nodiscard]] constexpr Result sub(std::span<Dest, Extent> output, A a, B b) noexcept {
    return batch::transform(output, [](Dest x, Dest y) noexcept { return fixedwide::sub(x, y); }, a, b);
}

template<class Dest, std::size_t Extent, class A, class B>
[[nodiscard]] inline Result add_to(std::span<Dest, Extent> output, A a, B b,
                                   Rounding rounding = Rounding::nearest_even) noexcept {
    return batch::transform(
        output, [rounding](auto x, auto y) noexcept { return fixedwide::add_to<Dest>(x, y, rounding); }, a, b);
}

template<class Dest, std::size_t Extent, class A, class B>
[[nodiscard]] inline Result sub_to(std::span<Dest, Extent> output, A a, B b,
                                   Rounding rounding = Rounding::nearest_even) noexcept {
    return batch::transform(
        output, [rounding](auto x, auto y) noexcept { return fixedwide::sub_to<Dest>(x, y, rounding); }, a, b);
}

template<class Dest, std::size_t Extent, class A, class B>
[[nodiscard]] inline Result div_to(std::span<Dest, Extent> output, A a, B b,
                                   Rounding rounding = Rounding::nearest_even) noexcept {
    return batch::transform(
        output, [rounding](auto x, auto y) noexcept { return fixedwide::div_to<Dest>(x, y, rounding); }, a, b);
}

template<class Dest, std::size_t Extent, class A, class B, class C>
[[nodiscard]] inline Result mul_div_to(std::span<Dest, Extent> output, A a, B b, C c,
                                       Rounding rounding = Rounding::nearest_even) noexcept {
    return batch::transform(
        output, [rounding](auto x, auto y, auto z) noexcept { return fixedwide::mul_div_to<Dest>(x, y, z, rounding); },
        a, b, c);
}

template<class Dest, std::size_t Extent, class A>
[[nodiscard]] inline Result fixed_cast(std::span<Dest, Extent> output, A a,
                                       Rounding rounding = Rounding::exact) noexcept {
    return batch::transform(
        output, [rounding](auto x) noexcept { return fixedwide::fixed_cast<Dest>(x, rounding); }, a);
}

/// Map rows independently, then add into a private accumulator in input order.
/// Every checked prefix is preserved: this is not an associative/tree reduce.
/// A failed addition at i outranks a speculative map failure at a later row.
/// The caller's initial value and all inputs are unchanged, including on error.
template<std::size_t BlockSize = 1, class T, class Map, class... Inputs>
[[nodiscard]] constexpr std::expected<T, Error>
transform_reduce(std::size_t count, T initial, Map&& map,
                 Inputs... inputs) noexcept(noexcept(detail::evaluate(map, 0, inputs...))) {
    static_assert(std::is_same_v<T, detail::value_type<Map, Inputs...>>, "batch map must produce the accumulator type");
    const auto result = batch::for_each<BlockSize>(
        count, std::forward<Map>(map),
        [&initial](std::size_t, T value) noexcept -> std::expected<void, ArithmeticError> {
            const auto next = fixedwide::add(initial, value);
            if (!next) return std::unexpected(next.error());
            initial = *next;
            return {};
        },
        inputs...);
    if (!result) return std::unexpected(result.error());
    return initial;
}

template<class T, std::size_t Extent>
[[nodiscard]] constexpr std::expected<std::remove_const_t<T>, Error>
reduce_add(std::span<T, Extent> values, std::remove_const_t<T> initial = {}) noexcept {
    return batch::transform_reduce(
        values.size(), initial, [](std::remove_const_t<T> value) noexcept { return value; }, values);
}

} // namespace fixedwide::batch

#undef FIXEDWIDE_BATCH_INLINE
#undef FIXEDWIDE_BATCH_CALL
