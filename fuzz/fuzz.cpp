#include <fixedwide/all.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string_view>

using namespace fixedwide;

template<typename T>
T load_val(const std::uint8_t* ptr) noexcept {
    T val;
    std::memcpy(&val, ptr, sizeof(T));
    return val;
}

namespace {

// Keep the independent general kernels as the reference, even after the
// public mixed operations acquire runtime fast paths. Compare errors as well
// as values: an overflow/inexact precedence change is a numerical regression.
template<typename Dest>
void same_as_kernel(const std::expected<Dest, ArithmeticError>& got,
                    const std::expected<wide::int256, ArithmeticError>& reference) {
    if (got.has_value() != reference.has_value()) std::abort();
    if (reference) {
        if (detail::to_int256_raw(got->raw()) != *reference) std::abort();
    } else if (got.error() != reference.error()) {
        std::abort();
    }
}

template<typename Dest, typename A, typename B>
void mixed_differential(A a, B b, Rounding mode) {
    const auto ar = detail::to_int256_raw(a.raw());
    const auto br = detail::to_int256_raw(b.raw());
    same_as_kernel(add_to<Dest>(a, b, mode),
                   detail::mixed_add_sub_kernel(ar, A::fractional_digits, br, B::fractional_digits, false,
                                                Dest::fractional_digits, mode, Dest::bits));
    same_as_kernel(sub_to<Dest>(a, b, mode),
                   detail::mixed_add_sub_kernel(ar, A::fractional_digits, br, B::fractional_digits, true,
                                                Dest::fractional_digits, mode, Dest::bits));
    same_as_kernel(mul_to<Dest>(a, b, mode),
                   detail::mixed_mul_kernel(ar, A::fractional_digits, br, B::fractional_digits, Dest::fractional_digits,
                                            mode, Dest::bits));
    same_as_kernel(fixed_cast<Dest>(a, mode),
                   detail::mixed_cast_kernel(ar, A::fractional_digits, Dest::fractional_digits, mode, Dest::bits));
}

void replacement_differential(Fixed128<12> total, Fixed128<12> previous, Fixed128<12> next) {
    using W = Fixed256<12>;
    const auto removed = sub_to<W>(total, previous, Rounding::exact);
    const auto removed_reference = detail::mixed_add_sub_kernel(
        wide::int256{total.raw()}, 12, wide::int256{previous.raw()}, 12, true, 12, Rounding::exact, 256);
    same_as_kernel(removed, removed_reference);
    if (!removed) std::abort(); // A difference of int128 values always fits int256.
    const auto replaced = add_to<W>(*removed, next, Rounding::exact);
    const auto replaced_reference = detail::mixed_add_sub_kernel(*removed_reference, 12, wide::int256{next.raw()}, 12,
                                                                 false, 12, Rounding::exact, 256);
    same_as_kernel(replaced, replaced_reference);
    if (!replaced) std::abort();
    same_as_kernel(fixed_cast<Fixed128<12>>(*replaced, Rounding::exact),
                   detail::mixed_cast_kernel(*replaced_reference, 12, 12, Rounding::exact, 128));
}

template<unsigned K>
void scale_boundary_differential(std::uint64_t choice, Rounding mode) {
    using Scaled = Fixed128<K>;
    using I128 = wide::int128;
    using U128 = wide::uint128;
    constexpr U128 factor = static_cast<U128>(Scaled::scale());
    constexpr I128 upper = static_cast<I128>(static_cast<U128>(I128::max()) / factor);
    constexpr I128 lower = -static_cast<I128>(U128{0, 0x8000'0000'0000'0000ULL} / factor);
    const bool negative = (choice & 1u) != 0;
    const I128 delta{static_cast<std::int64_t>((choice >> 1) % 3u) - 1};
    const auto a = Fixed128<0>::from_raw((negative ? lower : upper) + delta);
    mixed_differential<Scaled>(a, negative ? Scaled::max() : Scaled::min(), mode);
    mixed_differential<Scaled>(a, Fixed128<0>::from_raw(I128{1}), mode);
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size == 0) return 0;
    const std::string_view text(reinterpret_cast<const char*>(data), size);
    const auto mode = static_cast<Rounding>(data[0] % 6);

    // 1. Fuzz text parsing and formatting roundtrip
    char buf[128];
    auto f64 = parse<Fixed64<4>>(text, mode);
    if (f64) {
        auto formatted =
            to_chars(buf, sizeof(buf), *f64, FormatOptions{.digits = 4, .rounding = Rounding::nearest_even});
        if (formatted) {
            auto reparsed = parse<Fixed64<4>>(std::string_view(buf, *formatted), Rounding::nearest_even);
            if (!reparsed || *reparsed != *f64) std::abort();
        }
    }

    auto f128 = parse<Fixed128<12>>(text, mode);
    if (f128) {
        auto formatted =
            to_chars(buf, sizeof(buf), *f128, FormatOptions{.digits = 12, .rounding = Rounding::nearest_even});
        if (formatted) {
            auto reparsed = parse<Fixed128<12>>(std::string_view(buf, *formatted), Rounding::nearest_even);
            if (!reparsed || *reparsed != *f128) std::abort();
        }
    }

    auto f256 = parse<Fixed256<18>>(text, mode);
    if (f256) {
        auto formatted =
            to_chars(buf, sizeof(buf), *f256, FormatOptions{.digits = 18, .rounding = Rounding::nearest_even});
        if (formatted) {
            auto reparsed = parse<Fixed256<18>>(std::string_view(buf, *formatted), Rounding::nearest_even);
            if (!reparsed || *reparsed != *f256) std::abort();
        }
    }

    // 2. Binary roundtripping & unaligned load/store with memcpy
    if (size >= 16) {
        std::int64_t r1 = load_val<std::int64_t>(data);
        std::int64_t r2 = load_val<std::int64_t>(data + 8);
        auto a = Fixed64<4>::from_raw(r1);
        auto b = Fixed64<4>::from_raw(r2);

        // Binary LE & BE round trips
        auto le_bytes = to_bytes<endian::little>(a);
        auto a_le = from_bytes<Fixed64<4>, endian::little>(le_bytes);
        if (!a_le || *a_le != a) std::abort();

        auto be_bytes = to_bytes<endian::big>(a);
        auto a_be = from_bytes<Fixed64<4>, endian::big>(be_bytes);
        if (!a_be || *a_be != a) std::abort();

        auto s = add(a, b);
        auto d = sub(a, b);
        auto p = mul(a, b, mode);
        auto q = div(a, b, mode);
        (void)s;
        (void)d;
        (void)p;
        (void)q;
    }

    if (size >= 32) {
        std::uint64_t u0 = load_val<std::uint64_t>(data);
        std::uint64_t u1 = load_val<std::uint64_t>(data + 8);
        std::uint64_t u2 = load_val<std::uint64_t>(data + 16);
        std::uint64_t u3 = load_val<std::uint64_t>(data + 24);

        wide::int128 i1(u0, u1);
        wide::int128 i2(u2, u3);
        auto f128_1 = Fixed128<12>::from_raw(i1);
        auto f128_2 = Fixed128<12>::from_raw(i2);

        auto le128 = to_bytes<endian::little>(f128_1);
        auto f128_restored_le = from_bytes<Fixed128<12>, endian::little>(le128);
        if (!f128_restored_le || *f128_restored_le != f128_1) std::abort();

        auto be128 = to_bytes<endian::big>(f128_1);
        auto f128_restored_be = from_bytes<Fixed128<12>, endian::big>(be128);
        if (!f128_restored_be || *f128_restored_be != f128_1) std::abort();

        auto p128 = mul(f128_1, f128_2, mode);
        auto q128 = div(f128_1, f128_2, mode);
        (void)p128;
        (void)q128;

        auto mixed_res = mul_to<Fixed128<12>>(Fixed64<4>::from_raw(static_cast<std::int64_t>(i1.low)),
                                              Fixed64<8>::from_raw(static_cast<std::int64_t>(i2.low)), mode);
        (void)mixed_res;

        // Full-range inputs reach overflow fallbacks; a second biased draw
        // reaches successful int128 products, including nonzero upper limbs.
        wide::int128 narrow1{u0, u1 & 7u};
        wide::int128 narrow2{u2 & 0xFFFF'FFFF'FFFFULL, 0};
        if ((u1 & 8u) != 0) narrow1 = -narrow1;
        if ((u3 & 1u) != 0) narrow2 = -narrow2;
        for (bool narrow : {false, true}) {
            const auto left = narrow ? narrow1 : i1;
            const auto right = narrow ? narrow2 : i2;
            mixed_differential<Fixed128<12>>(Fixed128<12>::from_raw(left), Fixed128<12>::from_raw(right), mode);
            mixed_differential<Fixed128<12>>(Fixed128<12>::from_raw(left),
                                             Fixed64<10>::from_raw(static_cast<std::int64_t>(right)), mode);
            mixed_differential<Fixed256<12>>(Fixed128<12>::from_raw(left), Fixed128<12>::from_raw(right), mode);
            replacement_differential(Fixed128<12>::from_raw(left), Fixed128<12>::from_raw(right), f128_1);
            const auto sign_extended = Fixed256<12>::from_raw(wide::int256{left});
            same_as_kernel(fixed_cast<Fixed128<12>>(sign_extended, mode),
                           detail::mixed_cast_kernel(sign_extended.raw(), 12, 12, mode, 128));
        }

        // Change source and destination scales in both directions. A divisor
        // beyond int128 capacity must bypass native pow10 instantiation.
        switch (u3 % 4u) {
        case 0:
            mixed_differential<Fixed128<20>>(Fixed128<12>::from_raw(i1), Fixed128<6>::from_raw(i2), mode);
            scale_boundary_differential<1>(u1, mode);
            break;
        case 1:
            mixed_differential<Fixed64<10>>(Fixed128<12>::from_raw(narrow1), Fixed128<10>::from_raw(narrow2), mode);
            scale_boundary_differential<12>(u1, mode);
            break;
        case 2:
            mixed_differential<Fixed128<0>>(Fixed128<19>::from_raw(i1), Fixed128<20>::from_raw(i2), mode);
            scale_boundary_differential<19>(u1, mode);
            break;
        default:
            mixed_differential<Fixed128<38>>(Fixed128<0>::from_raw(i1), Fixed128<0>::from_raw(i2), mode);
            scale_boundary_differential<38>(u1, mode);
            break;
        }
    }

    if (size >= 64) {
        std::uint64_t limbs[4];
        std::memcpy(limbs, data, 32);
        wide::int256 i256_1(limbs[0], limbs[1], limbs[2], limbs[3]);
        auto f256_1 = Fixed256<18>::from_raw(i256_1);

        auto le256 = to_bytes<endian::little>(f256_1);
        auto f256_le = from_bytes<Fixed256<18>, endian::little>(le256);
        if (!f256_le || *f256_le != f256_1) std::abort();

        auto be256 = to_bytes<endian::big>(f256_1);
        auto f256_be = from_bytes<Fixed256<18>, endian::big>(be256);
        if (!f256_be || *f256_be != f256_1) std::abort();

        std::uint64_t other_limbs[4];
        std::memcpy(other_limbs, data + 32, 32);
        const wide::int256 i256_2{other_limbs[0], other_limbs[1], other_limbs[2], other_limbs[3]};
        mixed_differential<Fixed256<18>>(f256_1, Fixed256<18>::from_raw(i256_2), mode);
        mixed_differential<Fixed128<18>>(f256_1, Fixed256<18>::from_raw(i256_2), mode);
        const auto cancelling = -i256_1 + wide::int256{static_cast<std::int64_t>(other_limbs[0])};
        mixed_differential<Fixed128<18>>(f256_1, Fixed256<18>::from_raw(cancelling), mode);
        mixed_differential<Fixed256<18>>(f256_1, Fixed128<18>::from_raw(wide::int128{other_limbs[0], other_limbs[1]}),
                                         mode);
        same_as_kernel(fixed_cast<Fixed128<18>>(f256_1, mode), detail::mixed_cast_kernel(i256_1, 18, 18, mode, 128));
        same_as_kernel(fixed_cast<Fixed128<0>>(Fixed256<38>::from_raw(i256_1), mode),
                       detail::mixed_cast_kernel(i256_1, 38, 0, mode, 128));
    }

    return 0;
}
