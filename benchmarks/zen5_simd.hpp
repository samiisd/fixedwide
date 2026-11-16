#pragma once

// Bounded, benchmark-only exact-grid candidates. These are never selected by
// the installed library. Public values stay binary signed128; limb vectors
// exist only for one tile and unaccepted lanes retain the scalar operation.
#include <fixedwide/batch.hpp>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#if defined(FIXEDWIDE_HAS_X86_64_ASM) && defined(FIXEDWIDE_HAS_MIXED_NATIVE) && !defined(_MSC_VER)
#include <immintrin.h>
#endif

namespace fixedwide_zen5 {
using Money = fixedwide::Fixed128<12>;
using Price = fixedwide::Fixed64<10>;
using Status = fixedwide::batch::Result;
#if defined(FIXEDWIDE_HAS_X86_64_ASM) && defined(FIXEDWIDE_HAS_MIXED_NATIVE) && !defined(_MSC_VER)
using U128 = unsigned __int128;
inline constexpr std::uint64_t denominator = 10'000'000'000ULL;
inline constexpr U128 inverse = [] {
    U128 value = 1;
    for (unsigned i = 0; i < 7; ++i) value *= 2 - U128{9'765'625} * value;
    return value;
}();

namespace evex256 {
#define FW_Z5_INLINE [[gnu::target("avx512f,avx512dq,avx512vl"), gnu::always_inline]] inline
using V = __m256i;
struct Words {
    V low, high;
};
FW_Z5_INLINE V set(std::uint64_t value) noexcept {
    return _mm256_set1_epi64x(std::bit_cast<long long>(value));
}
FW_Z5_INLINE V zero() noexcept {
    return _mm256_setzero_si256();
}
FW_Z5_INLINE V add(V a, V b) noexcept {
    return _mm256_add_epi64(a, b);
}
FW_Z5_INLINE V sub(V a, V b) noexcept {
    return _mm256_sub_epi64(a, b);
}
FW_Z5_INLINE V bitand_(V a, V b) noexcept {
    return _mm256_and_si256(a, b);
}
FW_Z5_INLINE V bitor_(V a, V b) noexcept {
    return _mm256_or_si256(a, b);
}
FW_Z5_INLINE V bitxor_(V a, V b) noexcept {
    return _mm256_xor_si256(a, b);
}
FW_Z5_INLINE V low_product(V a, V b) noexcept {
    return _mm256_mullo_epi64(a, b);
}
template<unsigned Shift>
FW_Z5_INLINE V left(V a) noexcept {
    return _mm256_slli_epi64(a, Shift);
}
template<unsigned Shift>
FW_Z5_INLINE V right(V a) noexcept {
    return _mm256_srli_epi64(a, Shift);
}
FW_Z5_INLINE unsigned equal(V a, V b) noexcept {
    return _mm256_cmpeq_epi64_mask(a, b);
}
FW_Z5_INLINE unsigned less(V a, V b) noexcept {
    return _mm256_cmp_epu64_mask(a, b, _MM_CMPINT_LT);
}
FW_Z5_INLINE V ones(unsigned mask) noexcept {
    return _mm256_maskz_set1_epi64(static_cast<__mmask8>(mask), 1);
}
FW_Z5_INLINE V choose(unsigned mask, V a, V b) noexcept {
    return _mm256_mask_blend_epi64(static_cast<__mmask8>(mask), a, b);
}

// Four 32-bit partial products form both 64-bit limbs; VPMULLQ alone
// cannot supply a full product. The middle accumulator needs at most 34 bits.
FW_Z5_INLINE Words multiply(V a, V b) noexcept {
    const V mask = set(UINT32_MAX);
    const V p00 = _mm256_mul_epu32(a, b);
    const V p01 = _mm256_mul_epu32(a, right<32>(b));
    const V p10 = _mm256_mul_epu32(right<32>(a), b);
    const V p11 = _mm256_mul_epu32(right<32>(a), right<32>(b));
    const V middle = add(right<32>(p00), add(bitand_(p01, mask), bitand_(p10, mask)));
    return {bitor_(bitand_(p00, mask), left<32>(middle)),
            add(p11, add(right<32>(p01), add(right<32>(p10), right<32>(middle))))};
}
FW_Z5_INLINE Words negate(Words a, unsigned negative) noexcept {
    const V low = sub(zero(), a.low);
    const V high = sub(sub(zero(), a.high), ones((~equal(a.low, zero())) & 15u));
    return {choose(negative, a.low, low), choose(negative, a.high, high)};
}
FW_Z5_INLINE Words load(const Money* input) noexcept {
    static_assert(fixedwide::detail::batch::simd::packed128<Money>());
    // memcpy keeps natural alignment, input aliases and object representation
    // rules. The shuffle is an ephemeral SoA tile, never a duplicate book.
    V first, second;
    std::memcpy(&first, input, sizeof(first));
    std::memcpy(&second, input + 4 / 2, sizeof(second));
    const V lo_index = _mm256_setr_epi64x(0, 2, 4, 6);
    const V hi_index = _mm256_setr_epi64x(1, 3, 5, 7);
    return {_mm256_permutex2var_epi64(first, lo_index, second), _mm256_permutex2var_epi64(first, hi_index, second)};
}
FW_Z5_INLINE void store(Money* output, Words value) noexcept {
    static_assert(fixedwide::detail::batch::simd::packed128<Money>());
    const V first_index = _mm256_setr_epi64x(0, 4, 1, 5);
    const V second_index = _mm256_setr_epi64x(2, 6, 3, 7);
    const V first = _mm256_permutex2var_epi64(value.low, first_index, value.high);
    const V second = _mm256_permutex2var_epi64(value.low, second_index, value.high);
    std::memcpy(static_cast<void*>(output), &first, sizeof(first));
    std::memcpy(static_cast<void*>(output + 4 / 2), &second, sizeof(second));
}

// Wide=false certifies unsigned64 coefficients by full source reconstruction.
// Wide=true recovers the entire gap10 coefficient (<2^94). For a divisible
// magnitude a, n=a>>10; q=n*inverse(5^10) modulo2^128. Accepting q<=2^127/D
// proves q*5^10=n because both bounded values lie below the modulus. The
// 128x64 product then needs three binary limbs and a signed128 range check.
template<bool Wide>
FW_Z5_INLINE unsigned evaluate(const Money* input, std::int64_t factor, Money* output) noexcept {
    const Words raw = load(input);
    const unsigned negative = _mm256_cmp_epi64_mask(raw.high, zero(), _MM_CMPINT_LT);
    const Words magnitude = negate(raw, negative);
    const V nlow = bitor_(right<10>(magnitude.low), left<54>(magnitude.high));
    const V nhigh = right<10>(magnitude.high);
    Words q;
    unsigned accepted;
    if constexpr (Wide) {
        q = multiply(nlow, set(static_cast<std::uint64_t>(inverse)));
        q.high = add(q.high, add(low_product(nlow, set(static_cast<std::uint64_t>(inverse >> 64))),
                                 low_product(nhigh, set(static_cast<std::uint64_t>(inverse)))));
        constexpr U128 bound = (U128{1} << 127) / denominator;
        const V boundlow = set(static_cast<std::uint64_t>(bound));
        const V boundhigh = set(static_cast<std::uint64_t>(bound >> 64));
        accepted = equal(bitand_(magnitude.low, set(1023)), zero()) &
                   (less(q.high, boundhigh) | (equal(q.high, boundhigh) & (~less(boundlow, q.low)))) & 15u;
    } else {
        q = {low_product(nlow, set(static_cast<std::uint64_t>(inverse))), zero()};
        const Words reconstructed = multiply(q.low, set(denominator));
        accepted = equal(reconstructed.low, magnitude.low) & equal(reconstructed.high, magnitude.high);
    }
    const auto sign = factor < 0 ? UINT64_MAX : std::uint64_t{0};
    const auto mag = (static_cast<std::uint64_t>(factor) ^ sign) - sign;
    Words product = multiply(q.low, set(mag));
    if constexpr (Wide) {
        const Words upper = multiply(q.high, set(mag));
        const V high = add(product.high, upper.low);
        // A nonzero third limb cannot be repaired by sign restoration.
        accepted &= equal(add(upper.high, ones(less(high, product.high))), zero());
        product.high = high;
        const unsigned output_negative = negative ^ (factor < 0 ? 15u : 0u);
        const V limit_high = choose(output_negative, set(INT64_MAX), set(std::uint64_t{1} << 63));
        const V limit_low = choose(output_negative, set(UINT64_MAX), zero());
        accepted &=
            (less(product.high, limit_high) | (equal(product.high, limit_high) & ~less(limit_low, product.low))) & 15u;
    }
    store(output, negate(product, negative ^ (factor < 0 ? 15u : 0u)));
    return accepted;
}
// Safe pointer-only boundary for baseline callers. Matched-target complete
// kernels call evaluate() so GCC cannot outline the arithmetic per tile.
template<bool Wide>
[[nodiscard, gnu::target("avx512f,avx512dq,avx512vl")]]
inline unsigned tile(const Money* input, std::int64_t factor, Money* output) noexcept {
    return evaluate<Wide>(input, factor, output);
}
#undef FW_Z5_INLINE
} // namespace evex256

namespace evex512 {
#define FW_Z5_INLINE [[gnu::target("avx512f,avx512dq"), gnu::always_inline]] inline
using V = __m512i;
struct Words {
    V low, high;
};
FW_Z5_INLINE V set(std::uint64_t value) noexcept {
    return _mm512_set1_epi64(std::bit_cast<long long>(value));
}
FW_Z5_INLINE V zero() noexcept {
    return _mm512_setzero_si512();
}
FW_Z5_INLINE V add(V a, V b) noexcept {
    return _mm512_add_epi64(a, b);
}
FW_Z5_INLINE V sub(V a, V b) noexcept {
    return _mm512_sub_epi64(a, b);
}
FW_Z5_INLINE V bitand_(V a, V b) noexcept {
    return _mm512_and_si512(a, b);
}
FW_Z5_INLINE V bitor_(V a, V b) noexcept {
    return _mm512_or_si512(a, b);
}
FW_Z5_INLINE V bitxor_(V a, V b) noexcept {
    return _mm512_xor_si512(a, b);
}
FW_Z5_INLINE V low_product(V a, V b) noexcept {
    return _mm512_mullo_epi64(a, b);
}
template<unsigned Shift>
FW_Z5_INLINE V left(V a) noexcept {
    return _mm512_slli_epi64(a, Shift);
}
template<unsigned Shift>
FW_Z5_INLINE V right(V a) noexcept {
    return _mm512_srli_epi64(a, Shift);
}
FW_Z5_INLINE unsigned equal(V a, V b) noexcept {
    return _mm512_cmpeq_epi64_mask(a, b);
}
FW_Z5_INLINE unsigned less(V a, V b) noexcept {
    return _mm512_cmp_epu64_mask(a, b, _MM_CMPINT_LT);
}
FW_Z5_INLINE V ones(unsigned mask) noexcept {
    return _mm512_maskz_set1_epi64(static_cast<__mmask8>(mask), 1);
}
FW_Z5_INLINE V choose(unsigned mask, V a, V b) noexcept {
    return _mm512_mask_blend_epi64(static_cast<__mmask8>(mask), a, b);
}

// Four 32-bit partial products form both 64-bit limbs; VPMULLQ alone
// cannot supply a full product. The middle accumulator needs at most 34 bits.
FW_Z5_INLINE Words multiply(V a, V b) noexcept {
    const V mask = set(UINT32_MAX);
    const V p00 = _mm512_mul_epu32(a, b);
    const V p01 = _mm512_mul_epu32(a, right<32>(b));
    const V p10 = _mm512_mul_epu32(right<32>(a), b);
    const V p11 = _mm512_mul_epu32(right<32>(a), right<32>(b));
    const V middle = add(right<32>(p00), add(bitand_(p01, mask), bitand_(p10, mask)));
    return {bitor_(bitand_(p00, mask), left<32>(middle)),
            add(p11, add(right<32>(p01), add(right<32>(p10), right<32>(middle))))};
}
FW_Z5_INLINE Words negate(Words a, unsigned negative) noexcept {
    const V low = sub(zero(), a.low);
    const V high = sub(sub(zero(), a.high), ones((~equal(a.low, zero())) & 255u));
    return {choose(negative, a.low, low), choose(negative, a.high, high)};
}
FW_Z5_INLINE Words load(const Money* input) noexcept {
    static_assert(fixedwide::detail::batch::simd::packed128<Money>());
    // memcpy keeps natural alignment, input aliases and object representation
    // rules. The shuffle is an ephemeral SoA tile, never a duplicate book.
    V first, second;
    std::memcpy(&first, input, sizeof(first));
    std::memcpy(&second, input + 8 / 2, sizeof(second));
    const V lo_index = _mm512_setr_epi64(0, 2, 4, 6, 8, 10, 12, 14);
    const V hi_index = _mm512_setr_epi64(1, 3, 5, 7, 9, 11, 13, 15);
    return {_mm512_permutex2var_epi64(first, lo_index, second), _mm512_permutex2var_epi64(first, hi_index, second)};
}
FW_Z5_INLINE void store(Money* output, Words value) noexcept {
    static_assert(fixedwide::detail::batch::simd::packed128<Money>());
    const V first_index = _mm512_setr_epi64(0, 8, 1, 9, 2, 10, 3, 11);
    const V second_index = _mm512_setr_epi64(4, 12, 5, 13, 6, 14, 7, 15);
    const V first = _mm512_permutex2var_epi64(value.low, first_index, value.high);
    const V second = _mm512_permutex2var_epi64(value.low, second_index, value.high);
    std::memcpy(static_cast<void*>(output), &first, sizeof(first));
    std::memcpy(static_cast<void*>(output + 8 / 2), &second, sizeof(second));
}

// Wide=false certifies unsigned64 coefficients by full source reconstruction.
// Wide=true recovers the entire gap10 coefficient (<2^94). For a divisible
// magnitude a, n=a>>10; q=n*inverse(5^10) modulo2^128. Accepting q<=2^127/D
// proves q*5^10=n because both bounded values lie below the modulus. The
// 128x64 product then needs three binary limbs and a signed128 range check.
template<bool Wide>
FW_Z5_INLINE unsigned evaluate(const Money* input, std::int64_t factor, Money* output) noexcept {
    const Words raw = load(input);
    const unsigned negative = _mm512_cmp_epi64_mask(raw.high, zero(), _MM_CMPINT_LT);
    const Words magnitude = negate(raw, negative);
    const V nlow = bitor_(right<10>(magnitude.low), left<54>(magnitude.high));
    const V nhigh = right<10>(magnitude.high);
    Words q;
    unsigned accepted;
    if constexpr (Wide) {
        q = multiply(nlow, set(static_cast<std::uint64_t>(inverse)));
        q.high = add(q.high, add(low_product(nlow, set(static_cast<std::uint64_t>(inverse >> 64))),
                                 low_product(nhigh, set(static_cast<std::uint64_t>(inverse)))));
        constexpr U128 bound = (U128{1} << 127) / denominator;
        const V boundlow = set(static_cast<std::uint64_t>(bound));
        const V boundhigh = set(static_cast<std::uint64_t>(bound >> 64));
        accepted = equal(bitand_(magnitude.low, set(1023)), zero()) &
                   (less(q.high, boundhigh) | (equal(q.high, boundhigh) & (~less(boundlow, q.low)))) & 255u;
    } else {
        q = {low_product(nlow, set(static_cast<std::uint64_t>(inverse))), zero()};
        const Words reconstructed = multiply(q.low, set(denominator));
        accepted = equal(reconstructed.low, magnitude.low) & equal(reconstructed.high, magnitude.high);
    }
    const auto sign = factor < 0 ? UINT64_MAX : std::uint64_t{0};
    const auto mag = (static_cast<std::uint64_t>(factor) ^ sign) - sign;
    Words product = multiply(q.low, set(mag));
    if constexpr (Wide) {
        const Words upper = multiply(q.high, set(mag));
        const V high = add(product.high, upper.low);
        // A nonzero third limb cannot be repaired by sign restoration.
        accepted &= equal(add(upper.high, ones(less(high, product.high))), zero());
        product.high = high;
        const unsigned output_negative = negative ^ (factor < 0 ? 255u : 0u);
        const V limit_high = choose(output_negative, set(INT64_MAX), set(std::uint64_t{1} << 63));
        const V limit_low = choose(output_negative, set(UINT64_MAX), zero());
        accepted &=
            (less(product.high, limit_high) | (equal(product.high, limit_high) & ~less(limit_low, product.low))) & 255u;
    }
    store(output, negate(product, negative ^ (factor < 0 ? 255u : 0u)));
    return accepted;
}
template<bool Wide>
[[nodiscard, gnu::target("avx512f,avx512dq")]]
inline unsigned tile(const Money* input, std::int64_t factor, Money* output) noexcept {
    return evaluate<Wide>(input, factor, output);
}
#undef FW_Z5_INLINE
} // namespace evex512

#endif
} // namespace fixedwide_zen5
