#pragma once

// Benchmark-only exact-grid candidates. These entry points do not dispatch:
// the caller must establish CPU/OS support before invoking the selected ISA.
#include <fixedwide/fixed.hpp>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <type_traits>

#if defined(FIXEDWIDE_HAS_X86_64_ASM) && !defined(_MSC_VER)
#include <immintrin.h>

namespace fixedwide_zen5 {
namespace ifma_detail {

constexpr std::uint64_t mask52 = (std::uint64_t{1} << 52) - 1;
constexpr std::uint64_t denominator = 10'000'000'000ULL;
constexpr std::uint64_t inverse52 = [] {
    std::uint64_t inverse = 1;
    for (unsigned i = 0; i < 6; ++i) inverse *= 2 - (denominator >> 10) * inverse;
    return inverse & mask52;
}();

template<class Money>
constexpr bool packed_money = [] {
    if constexpr (!std::is_same_v<Money, fixedwide::Fixed128<12>> || sizeof(Money) != 16 ||
                  !std::is_trivially_copyable_v<Money>) {
        return false;
    } else {
        constexpr auto words =
            std::bit_cast<std::array<std::uint64_t, 2>>(Money::from_raw(fixedwide::wide::int128{1, 2}));
        return words[0] == 1 && words[1] == 2;
    }
}();

constexpr std::uint64_t magnitude(std::int64_t value) noexcept {
    const auto sign = value < 0 ? ~std::uint64_t{0} : 0;
    return (static_cast<std::uint64_t>(value) ^ sign) - sign;
}

} // namespace ifma_detail

// q is recovered modulo 2^52 from abs(raw) / 2^10. The complete q*10^10
// reconstruction certifies both the grid and its q52 range; wrapped candidates
// and off-grid inputs are misses, not arithmetic errors. q52*abs(signed64)
// needs at most 115 magnitude bits, so every accepted lane fits signed128.
// All input is loaded before any output is written, permitting exact in-place
// use. A rejected lane's output is unspecified and must not be published.
template<class Money>
[[nodiscard, gnu::target("avx512f,avx512ifma,avx512vl")]]
inline unsigned ifma256_tile(const Money* input, std::int64_t factor, Money* output) noexcept {
    static_assert(ifma_detail::packed_money<Money>);
    __m256i first, second;
    std::memcpy(&first, input, sizeof(first));
    std::memcpy(&second, input + 2, sizeof(second));
    const auto raw_low = _mm256_permutex2var_epi64(first, _mm256_set_epi64x(6, 4, 2, 0), second);
    const auto raw_high = _mm256_permutex2var_epi64(first, _mm256_set_epi64x(7, 5, 3, 1), second);
    const auto zero = _mm256_setzero_si256();
    const auto one = _mm256_set1_epi64x(1);
    const auto source_sign = _mm256_srai_epi64(raw_high, 63);
    const auto source_negative = _mm256_cmp_epi64_mask(raw_high, zero, _MM_CMPINT_LT);
    const auto low = _mm256_sub_epi64(_mm256_xor_si256(raw_low, source_sign), source_sign);
    const auto high_complement = _mm256_xor_si256(raw_high, source_sign);
    const auto high = _mm256_mask_add_epi64(high_complement, source_negative & _mm256_cmpeq_epi64_mask(raw_low, zero),
                                            high_complement, one);

    const auto q = _mm256_madd52lo_epu64(zero, _mm256_srli_epi64(low, 10), _mm256_set1_epi64x(ifma_detail::inverse52));
    const auto divisor = _mm256_set1_epi64x(ifma_detail::denominator);
    const auto reconstruction0 = _mm256_madd52lo_epu64(zero, q, divisor);
    const auto reconstruction1 = _mm256_madd52hi_epu64(zero, q, divisor);
    const auto reconstructed_low = _mm256_or_si256(reconstruction0, _mm256_slli_epi64(reconstruction1, 52));
    const auto reconstructed_high = _mm256_srli_epi64(reconstruction1, 12);
    const unsigned accepted =
        _mm256_cmpeq_epi64_mask(low, reconstructed_low) & _mm256_cmpeq_epi64_mask(high, reconstructed_high);

    const auto factor_magnitude = ifma_detail::magnitude(factor);
    const auto factor0 = _mm256_set1_epi64x(factor_magnitude & ifma_detail::mask52);
    const auto product0 = _mm256_madd52lo_epu64(zero, q, factor0);
    auto product1 = _mm256_madd52hi_epu64(zero, q, factor0);
    auto product2 = zero;
    if ((factor_magnitude >> 52) != 0) {
        const auto factor1 = _mm256_set1_epi64x(factor_magnitude >> 52);
        product1 = _mm256_madd52lo_epu64(product1, q, factor1);
        product2 = _mm256_madd52hi_epu64(zero, q, factor1);
    }
    // product1 < 2*2^52: its carry is retained by the binary high-word add.
    // product2 < 2^11. No accumulator or reconstructed output can overflow.
    const auto magnitude_low = _mm256_or_si256(product0, _mm256_slli_epi64(product1, 52));
    const auto magnitude_high = _mm256_add_epi64(_mm256_srli_epi64(product1, 12), _mm256_slli_epi64(product2, 40));
    const auto sign = _mm256_xor_si256(source_sign, _mm256_set1_epi64x(factor < 0 ? -1 : 0));
    const auto negative = _mm256_cmp_epi64_mask(sign, zero, _MM_CMPINT_LT);
    const auto result_low = _mm256_sub_epi64(_mm256_xor_si256(magnitude_low, sign), sign);
    const auto result_high_complement = _mm256_xor_si256(magnitude_high, sign);
    const auto result_high = _mm256_mask_add_epi64(
        result_high_complement, negative & _mm256_cmpeq_epi64_mask(magnitude_low, zero), result_high_complement, one);
    first = _mm256_permutex2var_epi64(result_low, _mm256_set_epi64x(5, 1, 4, 0), result_high);
    second = _mm256_permutex2var_epi64(result_low, _mm256_set_epi64x(7, 3, 6, 2), result_high);
    std::memcpy(static_cast<void*>(output), &first, sizeof(first));
    std::memcpy(static_cast<void*>(output + 2), &second, sizeof(second));
    return accepted;
}

// The eight-row variant is only legal for a caller's speculation bound >= 8.
// It is separate from the four-row variant and never reads a hidden extra row.
template<class Money>
[[nodiscard, gnu::target("avx512f,avx512ifma")]]
inline unsigned ifma512_tile(const Money* input, std::int64_t factor, Money* output) noexcept {
    static_assert(ifma_detail::packed_money<Money>);
    __m512i first, second;
    std::memcpy(&first, input, sizeof(first));
    std::memcpy(&second, input + 4, sizeof(second));
    const auto raw_low = _mm512_permutex2var_epi64(first, _mm512_set_epi64(14, 12, 10, 8, 6, 4, 2, 0), second);
    const auto raw_high = _mm512_permutex2var_epi64(first, _mm512_set_epi64(15, 13, 11, 9, 7, 5, 3, 1), second);
    const auto zero = _mm512_setzero_si512();
    const auto one = _mm512_set1_epi64(1);
    const auto source_sign = _mm512_srai_epi64(raw_high, 63);
    const auto source_negative = _mm512_cmp_epi64_mask(raw_high, zero, _MM_CMPINT_LT);
    const auto low = _mm512_sub_epi64(_mm512_xor_si512(raw_low, source_sign), source_sign);
    const auto high_complement = _mm512_xor_si512(raw_high, source_sign);
    const auto high = _mm512_mask_add_epi64(high_complement, source_negative & _mm512_cmpeq_epi64_mask(raw_low, zero),
                                            high_complement, one);

    const auto q = _mm512_madd52lo_epu64(zero, _mm512_srli_epi64(low, 10), _mm512_set1_epi64(ifma_detail::inverse52));
    const auto divisor = _mm512_set1_epi64(ifma_detail::denominator);
    const auto reconstruction0 = _mm512_madd52lo_epu64(zero, q, divisor);
    const auto reconstruction1 = _mm512_madd52hi_epu64(zero, q, divisor);
    const auto reconstructed_low = _mm512_or_si512(reconstruction0, _mm512_slli_epi64(reconstruction1, 52));
    const auto reconstructed_high = _mm512_srli_epi64(reconstruction1, 12);
    const unsigned accepted =
        _mm512_cmpeq_epi64_mask(low, reconstructed_low) & _mm512_cmpeq_epi64_mask(high, reconstructed_high);

    const auto factor_magnitude = ifma_detail::magnitude(factor);
    const auto factor0 = _mm512_set1_epi64(factor_magnitude & ifma_detail::mask52);
    const auto product0 = _mm512_madd52lo_epu64(zero, q, factor0);
    auto product1 = _mm512_madd52hi_epu64(zero, q, factor0);
    auto product2 = zero;
    if ((factor_magnitude >> 52) != 0) {
        const auto factor1 = _mm512_set1_epi64(factor_magnitude >> 52);
        product1 = _mm512_madd52lo_epu64(product1, q, factor1);
        product2 = _mm512_madd52hi_epu64(zero, q, factor1);
    }
    const auto magnitude_low = _mm512_or_si512(product0, _mm512_slli_epi64(product1, 52));
    const auto magnitude_high = _mm512_add_epi64(_mm512_srli_epi64(product1, 12), _mm512_slli_epi64(product2, 40));
    const auto sign = _mm512_xor_si512(source_sign, _mm512_set1_epi64(factor < 0 ? -1 : 0));
    const auto negative = _mm512_cmp_epi64_mask(sign, zero, _MM_CMPINT_LT);
    const auto result_low = _mm512_sub_epi64(_mm512_xor_si512(magnitude_low, sign), sign);
    const auto result_high_complement = _mm512_xor_si512(magnitude_high, sign);
    const auto result_high = _mm512_mask_add_epi64(
        result_high_complement, negative & _mm512_cmpeq_epi64_mask(magnitude_low, zero), result_high_complement, one);
    first = _mm512_permutex2var_epi64(result_low, _mm512_set_epi64(11, 3, 10, 2, 9, 1, 8, 0), result_high);
    second = _mm512_permutex2var_epi64(result_low, _mm512_set_epi64(15, 7, 14, 6, 13, 5, 12, 4), result_high);
    std::memcpy(static_cast<void*>(output), &first, sizeof(first));
    std::memcpy(static_cast<void*>(output + 4), &second, sizeof(second));
    return accepted;
}

} // namespace fixedwide_zen5
#endif
