#include <fixedwide/all.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string_view>

using namespace fixedwide;

template<typename T>
T load_val(const std::uint8_t* ptr) noexcept {
    T val;
    std::memcpy(&val, ptr, sizeof(T));
    return val;
}

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
    }

    return 0;
}
