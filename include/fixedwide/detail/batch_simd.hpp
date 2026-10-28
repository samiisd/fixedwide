#pragma once

#include <fixedwide/detail/batch_multiply.hpp>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>

#if defined(FIXEDWIDE_HAS_X86_64_ASM) && defined(FIXEDWIDE_HAS_MIXED_NATIVE) && !defined(_MSC_VER)
#include <immintrin.h>
#endif

namespace fixedwide::detail::batch {

// Arbitrary user maps and operand topologies retain the ordinary batch loop.
template<class Map, class... Inputs>
struct simd_dispatch {
    static constexpr bool available = false;
};

#if defined(FIXEDWIDE_HAS_X86_64_ASM) && defined(FIXEDWIDE_HAS_MIXED_NATIVE) && !defined(_MSC_VER)

namespace simd {

#define FIXEDWIDE_BATCH_AVX2 [[gnu::target("avx2")]]
#define FIXEDWIDE_BATCH_AVX2_INLINE [[gnu::target("avx2"), gnu::always_inline]] inline

using Vector = __m256i;

struct Words {
    Vector low, high;
};

struct Tile {
    Words values;
    unsigned accepted;
};

enum class Kind { signed32, unsigned32, unsigned64 };

struct Factor {
    std::int64_t raw;
    std::uint64_t magnitude;
    std::uint64_t negative;

    explicit Factor(std::int64_t value) noexcept : raw(value) {
        negative = value < 0 ? ~std::uint64_t{0} : 0;
        magnitude = (static_cast<std::uint64_t>(value) ^ negative) - negative;
    }

    [[nodiscard]] bool signed32() const noexcept {
        return raw >= std::numeric_limits<std::int32_t>::min() && raw <= std::numeric_limits<std::int32_t>::max();
    }
};

template<class T>
inline constexpr bool fixed128 = false;
template<unsigned Digits>
inline constexpr bool fixed128<basic_fixed<128, Digits>> = true;

// Only the public Fixed128 representation is loaded as two adjacent limbs.
// These checks also catch a future representation change before SIMD can
// reinterpret it incorrectly. memcpy permits ordinary alignment and aliasing.
template<class Fixed>
[[nodiscard]] consteval bool packed128() noexcept {
    if constexpr (sizeof(Fixed) != 16 || !std::is_trivially_copyable_v<Fixed>) {
        return false;
    } else {
        constexpr auto positive = std::bit_cast<std::array<std::uint64_t, 2>>(Fixed::from_raw(wide::int128{1, 2}));
        constexpr auto negative = std::bit_cast<std::array<std::uint64_t, 2>>(Fixed::from_raw(wide::int128{-1}));
        return positive[0] == 1 && positive[1] == 2 && negative[0] == ~std::uint64_t{0} &&
               negative[1] == ~std::uint64_t{0};
    }
}

[[nodiscard]] constexpr std::uint32_t inverse_odd(std::uint32_t value) noexcept {
    std::uint32_t inverse = 1;
    for (unsigned i = 0; i < 5; ++i) inverse *= 2u - value * inverse;
    return inverse;
}

[[nodiscard]] FIXEDWIDE_BATCH_AVX2_INLINE Vector broadcast(std::uint64_t value) noexcept {
    return _mm256_set1_epi64x(std::bit_cast<long long>(value));
}

template<class Source>
[[nodiscard]] FIXEDWIDE_BATCH_AVX2_INLINE Words load(const Source* input) noexcept {
    static_assert(packed128<Source>());
    Vector first, second;
    std::memcpy(&first, input, sizeof(first));
    std::memcpy(&second, input + 2, sizeof(second));
    return {_mm256_permute4x64_epi64(_mm256_unpacklo_epi64(first, second), 0xd8),
            _mm256_permute4x64_epi64(_mm256_unpackhi_epi64(first, second), 0xd8)};
}

template<class Dest>
FIXEDWIDE_BATCH_AVX2_INLINE void store(Dest* output, Words value) noexcept {
    static_assert(packed128<Dest>());
    const Vector even = _mm256_unpacklo_epi64(value.low, value.high);
    const Vector odd = _mm256_unpackhi_epi64(value.low, value.high);
    const Vector first = _mm256_permute2x128_si256(even, odd, 0x20);
    const Vector second = _mm256_permute2x128_si256(even, odd, 0x31);
    std::memcpy(static_cast<void*>(output), &first, sizeof(first));
    std::memcpy(static_cast<void*>(output + 2), &second, sizeof(second));
}

[[nodiscard]] FIXEDWIDE_BATCH_AVX2_INLINE Words apply_sign(Words value, Vector negative) noexcept {
    const Vector carry = _mm256_and_si256(negative, _mm256_cmpeq_epi64(value.low, _mm256_setzero_si256()));
    return {_mm256_sub_epi64(_mm256_xor_si256(value.low, negative), negative),
            _mm256_sub_epi64(_mm256_xor_si256(value.high, negative), carry)};
}

[[nodiscard]] FIXEDWIDE_BATCH_AVX2_INLINE Vector carry_mask(Vector sum, Vector addend) noexcept {
    const Vector sign = broadcast(std::uint64_t{1} << 63);
    return _mm256_cmpgt_epi64(_mm256_xor_si256(addend, sign), _mm256_xor_si256(sum, sign));
}

[[nodiscard]] FIXEDWIDE_BATCH_AVX2_INLINE Words multiply32x64(Vector a, std::uint64_t b) noexcept {
    const Vector low = _mm256_mul_epu32(a, broadcast(static_cast<std::uint32_t>(b)));
    const Vector high = _mm256_mul_epu32(a, broadcast(b >> 32));
    const Vector sum = _mm256_add_epi64(low, _mm256_slli_epi64(high, 32));
    return {sum, _mm256_sub_epi64(_mm256_srli_epi64(high, 32), carry_mask(sum, low))};
}

// Candidate recovery is modulo 2^32. The complete 128-bit reconstruction,
// not the residue alone, certifies the small coefficient and exact division.
// Unaccepted lanes are evaluated by the original prepared scalar operation.
template<unsigned Gap, Kind Method, class Source>
[[nodiscard]] FIXEDWIDE_BATCH_AVX2_INLINE Tile evaluate(const Source* input, const Factor& factor) noexcept {
    static_assert(Gap >= 1 && Gap <= 19);
    constexpr auto denominator = static_cast<std::uint64_t>(mixed_native::pow10_v<Gap>);
    constexpr auto inverse = inverse_odd(static_cast<std::uint32_t>(denominator >> Gap));
    const Words raw = load(input);
    const Vector zero = _mm256_setzero_si256();
    if constexpr (Method == Kind::signed32) {
        static_assert(Gap == 10);
        // D = 1410065408 + 2*2^32. Both signed partial products fit64;
        // their sign extensions and the low-limb carry reconstruct signed128.
        const Vector q = _mm256_mul_epu32(_mm256_srli_epi64(raw.low, 10), broadcast(inverse));
        const Vector low_product = _mm256_mul_epi32(q, broadcast(1410065408u));
        const Vector negative = _mm256_cmpgt_epi64(zero, low_product);
        const Vector low = _mm256_add_epi64(low_product, _mm256_slli_epi64(q, 33));
        const Vector high = _mm256_sub_epi64(_mm256_slli_epi64(negative, 1), carry_mask(low, low_product));
        const Vector accepted = _mm256_and_si256(_mm256_cmpeq_epi64(low, raw.low), _mm256_cmpeq_epi64(high, raw.high));
        const Vector product = _mm256_mul_epi32(q, broadcast(static_cast<std::uint32_t>(factor.raw)));
        return {{product, _mm256_cmpgt_epi64(zero, product)},
                static_cast<unsigned>(_mm256_movemask_pd(_mm256_castsi256_pd(accepted)))};
    } else {
        const Vector negative = _mm256_cmpgt_epi64(zero, raw.high);
        const Words magnitude = apply_sign(raw, negative);
        const Vector q = _mm256_and_si256(_mm256_mul_epu32(_mm256_srli_epi64(magnitude.low, Gap), broadcast(inverse)),
                                          broadcast(UINT32_MAX));
        const Words reconstructed = multiply32x64(q, denominator);
        const Vector accepted = _mm256_and_si256(_mm256_cmpeq_epi64(reconstructed.low, magnitude.low),
                                                 _mm256_cmpeq_epi64(reconstructed.high, magnitude.high));
        Words product;
        if constexpr (Method == Kind::unsigned32)
            product = {_mm256_mul_epu32(q, broadcast(factor.magnitude)), zero};
        else
            product = multiply32x64(q, factor.magnitude);
        // q32 * abs(signed64) needs at most95 magnitude bits, so every
        // certified result fits signed128. A certified lane has no remainder.
        return {apply_sign(product, _mm256_xor_si256(negative, broadcast(factor.negative))),
                static_cast<unsigned>(_mm256_movemask_pd(_mm256_castsi256_pd(accepted)))};
    }
}

template<class Commit, class Dest>
inline constexpr bool nothrow_commit = noexcept(std::declval<Commit&>()(std::size_t{}, std::declval<const Dest&>()));

template<class Commit, class Dest>
[[nodiscard, gnu::always_inline]] inline std::expected<void, ArithmeticError>
commit_one(Commit& commit, std::size_t row, const Dest& value) noexcept(nothrow_commit<Commit, Dest>) {
#if defined(__clang__)
    [[clang::always_inline]] return commit(std::size_t{row}, value);
#else
    return commit(std::size_t{row}, value);
#endif
}

template<Kind Method, class Dest>
using Stored = std::conditional_t<Method == Kind::signed32, std::int64_t, Dest>;

template<Kind Method, class Dest>
[[nodiscard, gnu::always_inline]] inline Dest lane_value(const Stored<Method, Dest>& value) noexcept {
    if constexpr (Method == Kind::signed32)
        return Dest::from_raw(wide::int128{value});
    else
        return value;
}

template<unsigned Gap, Kind Method, class Error, class Dest, class Map, class Commit, class Source, std::size_t Extent>
FIXEDWIDE_BATCH_AVX2 std::expected<void, Error> for_each(const Map& map, Commit& commit,
                                                         std::span<Source, Extent> input,
                                                         const Factor& factor) noexcept(nothrow_commit<Commit, Dest>) {
    std::size_t row = 0;
    for (; input.size() - row >= 4; row += 4) {
        const auto tile = evaluate<Gap, Method>(input.data() + row, factor);
        std::array<Stored<Method, Dest>, 4> values;
        if constexpr (Method == Kind::signed32)
            std::memcpy(values.data(), &tile.values.low, sizeof(tile.values.low));
        else
            store(values.data(), tile.values);

        if (tile.accepted == 15) {
            for (unsigned lane = 0; lane < 4; ++lane) {
                const Dest value = lane_value<Method, Dest>(values[lane]);
                const auto status = commit_one(commit, row + lane, value);
                if (!status) return std::unexpected(Error{row + lane, status.error()});
            }
        } else {
            for (unsigned lane = 0; lane < 4; ++lane) {
                Dest value = lane_value<Method, Dest>(values[lane]);
                if ((tile.accepted & (1u << lane)) == 0) {
                    const auto mapped = map(input[row + lane]);
                    if (!mapped) return std::unexpected(Error{row + lane, mapped.error()});
                    value = *mapped;
                }
                const auto status = commit_one(commit, row + lane, std::as_const(value));
                if (!status) return std::unexpected(Error{row + lane, status.error()});
            }
        }
    }
    for (; row < input.size(); ++row) {
        const auto mapped = map(input[row]);
        if (!mapped) return std::unexpected(Error{row, mapped.error()});
        const auto status = commit_one(commit, row, *mapped);
        if (!status) return std::unexpected(Error{row, status.error()});
    }
    return {};
}

template<unsigned Gap, Kind Method, class Error, class Dest, std::size_t OutExtent, class Map, class Source,
         std::size_t InExtent>
FIXEDWIDE_BATCH_AVX2 std::expected<void, Error> transform(const Map& map, std::span<Dest, OutExtent> output,
                                                          std::span<Source, InExtent> input,
                                                          const Factor& factor) noexcept {
    assert(output.size() == input.size());
    std::size_t row = 0;
    for (; input.size() - row >= 4; row += 4) {
        const auto tile = evaluate<Gap, Method>(input.data() + row, factor);
        if (tile.accepted == 15) {
            store(output.data() + row, tile.values);
            continue;
        }
        std::array<Dest, 4> values;
        store(values.data(), tile.values);
        for (unsigned lane = 0; lane < 4; ++lane) {
            if ((tile.accepted & (1u << lane)) != 0) {
                output[row + lane] = values[lane];
            } else {
                const auto mapped = map(input[row + lane]);
                if (!mapped) return std::unexpected(Error{row + lane, mapped.error()});
                output[row + lane] = *mapped;
            }
        }
    }
    for (; row < input.size(); ++row) {
        const auto mapped = map(input[row]);
        if (!mapped) return std::unexpected(Error{row, mapped.error()});
        output[row] = *mapped;
    }
    return {};
}

#undef FIXEDWIDE_BATCH_AVX2
#undef FIXEDWIDE_BATCH_AVX2_INLINE

} // namespace simd

template<class Dest, class B, class A, std::size_t Extent>
    requires(simd::fixed128<Dest> && simd::fixed128<std::remove_const_t<A>>)
struct simd_dispatch<broadcast_multiplier<Dest, B>, std::span<A, Extent>> {
    using Map = broadcast_multiplier<Dest, B>;
    using Span = std::span<A, Extent>;
    static constexpr unsigned product_scale = A::fractional_digits + B::fractional_digits;
    static constexpr unsigned gap =
        product_scale >= Dest::fractional_digits ? product_scale - Dest::fractional_digits : 0;
    static constexpr bool available = B::bits <= 64 && gap >= 1 && gap <= 19;

    [[nodiscard]] static bool can_run(const Map& map, std::size_t count) noexcept {
        if constexpr (available)
            return count >= 4 && map.rounding() == Rounding::exact && __builtin_cpu_supports("avx2");
        else
            return false;
    }

    template<class Error, class Commit>
    [[nodiscard]] static std::expected<void, Error> for_each(const Map& map, Commit& commit,
                                                             Span input) noexcept(simd::nothrow_commit<Commit, Dest>) {
        const simd::Factor factor{static_cast<std::int64_t>(map.factor().raw())};
        if constexpr (gap == 10) {
            if (factor.signed32())
                return simd::for_each<gap, simd::Kind::signed32, Error, Dest>(map, commit, input, factor);
        }
        if (factor.magnitude <= UINT32_MAX)
            return simd::for_each<gap, simd::Kind::unsigned32, Error, Dest>(map, commit, input, factor);
        return simd::for_each<gap, simd::Kind::unsigned64, Error, Dest>(map, commit, input, factor);
    }

    template<class Error, std::size_t OutExtent>
    [[nodiscard]] static std::expected<void, Error> transform(const Map& map, std::span<Dest, OutExtent> output,
                                                              Span input) noexcept {
        const simd::Factor factor{static_cast<std::int64_t>(map.factor().raw())};
        if constexpr (gap == 10) {
            if (factor.signed32()) return simd::transform<gap, simd::Kind::signed32, Error>(map, output, input, factor);
        }
        if (factor.magnitude <= UINT32_MAX)
            return simd::transform<gap, simd::Kind::unsigned32, Error>(map, output, input, factor);
        return simd::transform<gap, simd::Kind::unsigned64, Error>(map, output, input, factor);
    }
};

#endif

} // namespace fixedwide::detail::batch
