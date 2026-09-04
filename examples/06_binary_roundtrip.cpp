// 06 - Binary serialisation with the byte order written down.
//
// to_bytes/from_bytes never guess the endianness of the wire: you name it, so
// a file or a packet written on one machine reads the same on another. The
// conversion is zero-copy and allocation-free.
//
// Docs: ../docs/api_reference.md#binary-serialization

#include <fixedwide/all.hpp>
#include <cstdio>
#include <cstring>

int main() {
    using namespace fixedwide;
    using Money = Fixed128<6>;

    const auto value = parse<Money>("-1234567.891234").value();

    // Little-endian wire format.
    const auto le = to_bytes<endian::little>(value);
    static_assert(le.size() == 16, "Fixed128 is 16 bytes on the wire");
    const auto back_le = from_bytes<Money, endian::little>(le);
    if (!back_le || *back_le != value) return 1;

    // Big-endian wire format: the same 16 bytes, reversed.
    const auto be = to_bytes<endian::big>(value);
    const auto back_be = from_bytes<Money, endian::big>(be);
    if (!back_be || *back_be != value) return 1;

    for (std::size_t i = 0; i < le.size(); ++i) {
        if (le[i] != be[le.size() - 1 - i]) return 1;
    }

    std::printf("value      %s\n", to_string(value).value().c_str());
    std::printf("little     ");
    for (auto b : le) std::printf("%02x", b);
    std::printf("\nbig        ");
    for (auto b : be) std::printf("%02x", b);
    std::puts("");

    // A span of the wrong length is rejected rather than read past.
    std::array<std::uint8_t, 8> too_short{};
    const auto bad = from_bytes<Money, endian::little>(too_short);
    if (bad || bad.error() != BinaryError::wrong_size) return 1;

    // Unaligned access, for reading straight out of a packet buffer.
    alignas(1) unsigned char packet[32];
    std::memset(packet, 0, sizeof packet);
    store_unaligned<endian::big>(packet + 1, value);
    if (load_unaligned<Money, endian::big>(packet + 1) != value) return 1;

    std::puts("OK");
    return 0;
}
