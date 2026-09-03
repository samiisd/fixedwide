#include "check.hpp"
#include <fixedwide/all.hpp>

using namespace fixedwide;

void test_binary_roundtrip() {
    auto f8 = Fixed8<1>::from_raw(42);
    auto b8_le = to_bytes<endian::little>(f8);
    auto b8_be = to_bytes<endian::big>(f8);
    CHECK(from_bytes<Fixed8<1>, endian::little>(b8_le) == f8);
    CHECK(from_bytes<Fixed8<1>, endian::big>(b8_be) == f8);

    auto f64 = Fixed64<12>::from_raw(0x0102030405060708LL);
    auto b64_le = to_bytes<endian::little>(f64);
    auto b64_be = to_bytes<endian::big>(f64);
    CHECK(b64_le[0] == 0x08 && b64_le[7] == 0x01);
    CHECK(b64_be[0] == 0x01 && b64_be[7] == 0x08);
    CHECK(from_bytes<Fixed64<12>, endian::little>(b64_le) == f64);
    CHECK(from_bytes<Fixed64<12>, endian::big>(b64_be) == f64);

    auto f128 = Fixed128<12>::from_raw(wide::int128(0x0102030405060708ULL, 0x090a0b0c0d0e0f10ULL));
    auto b128_le = to_bytes<endian::little>(f128);
    CHECK(from_bytes<Fixed128<12>, endian::little>(b128_le) == f128);

    auto f256 = Fixed256<18>::from_raw(wide::int256(1, 2, 3, 4));
    auto b256_le = to_bytes<endian::little>(f256);
    CHECK(from_bytes<Fixed256<18>, endian::little>(b256_le) == f256);

    // Wrong size error
    std::uint8_t short_b[3] = {1, 2, 3};
    auto err = from_bytes<Fixed64<12>>(short_b);
    CHECK(!err.has_value() && err.error() == BinaryError::wrong_size);

    // Unaligned load/store
    std::uint8_t unaligned[40]{};
    store_unaligned<endian::little>(unaligned + 3, f64);
    CHECK(load_unaligned<Fixed64<12>, endian::little>(unaligned + 3) == f64);
}

int main() {
    test_binary_roundtrip();
    std::printf("test_binary passed (%lu checks)\n", checks);
    return 0;
}
