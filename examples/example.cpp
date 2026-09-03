#include <fixedwide/all.hpp>
#include <cstdio>

int main() {
    using namespace fixedwide;

    // Define distinct domain types with compile-time scales
    using Price = Fixed64<4>;
    using Quantity = Fixed32<2>;
    using Notional = Fixed128<6>;

    // Parsing from ASCII decimal text
    auto price = parse<Price>("123.4567");
    auto qty = parse<Quantity>("10.50");

    if (!price || !qty) {
        std::fprintf(stderr, "Failed to parse inputs\n");
        return 1;
    }

    // Explicit cross-scale mixed multiplication into target Notional
    auto notional = mul_to<Notional>(*price, *qty, Rounding::nearest_even);
    if (!notional) {
        std::fprintf(stderr, "Calculation overflow\n");
        return 2;
    }

    // Format output with trailing zeros trimmed
    auto str = to_string(*notional, FormatOptions{.trim_trailing_zeros = true});

    std::printf("Price:    %s\n", to_string(*price).value().c_str());
    std::printf("Quantity: %s\n", to_string(*qty).value().c_str());
    std::printf("Notional: %s\n", str.value().c_str());

    // Binary serialization
    auto bytes = to_bytes<endian::little>(*notional);
    auto restored = from_bytes<Notional, endian::little>(bytes);
    if (restored == *notional) {
        std::printf("Binary serialization roundtrip successful (%zu bytes)\n", bytes.size());
    }

    return 0;
}
