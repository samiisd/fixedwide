#include <fixedwide/all.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string_view>

using namespace fixedwide;

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size == 0) return 0;
    const std::string_view text(reinterpret_cast<const char*>(data), size);
    const auto mode = static_cast<Rounding>(data[0] % 6);

    // Fuzz parsing and roundtripping across various scales and bits
    auto f64 = parse<Fixed64<4>>(text, mode);
    char buf[128];
    if (f64) {
        auto formatted = to_chars(buf, sizeof(buf), *f64, FormatOptions{.rounding = mode});
        if (formatted) {
            auto reparsed = parse<Fixed64<4>>(std::string_view(buf, *formatted), mode);
            (void)reparsed;
        }
    }

    auto f128 = parse<Fixed128<12>>(text, mode);
    if (f128) {
        auto formatted = to_chars(buf, sizeof(buf), *f128, FormatOptions{.rounding = mode});
        if (formatted) {
            auto reparsed = parse<Fixed128<12>>(std::string_view(buf, *formatted), mode);
            (void)reparsed;
        }
    }

    auto f256 = parse<Fixed256<18>>(text, mode);
    if (f256) {
        auto formatted = to_chars(buf, sizeof(buf), *f256, FormatOptions{.rounding = mode});
        if (formatted) {
            auto reparsed = parse<Fixed256<18>>(std::string_view(buf, *formatted), mode);
            (void)reparsed;
        }
    }

    // Fuzz binary operations
    if (size >= 16) {
        auto f1 = Fixed64<4>::from_raw(*reinterpret_cast<const std::int64_t*>(data));
        auto f2 = Fixed64<4>::from_raw(*reinterpret_cast<const std::int64_t*>(data + 8));
        auto s = add(f1, f2);
        auto d = sub(f1, f2);
        auto p = mul(f1, f2, mode);
        auto q = div(f1, f2, mode);
        (void)s; (void)d; (void)p; (void)q;

        // Binary serialization
        auto b_le = to_bytes<endian::little>(f1);
        auto restored = from_bytes<Fixed64<4>, endian::little>(b_le);
        if (restored != f1) std::abort();
    }

    if (size >= 32) {
        wide::int128 i1(*reinterpret_cast<const std::uint64_t*>(data), *reinterpret_cast<const std::uint64_t*>(data + 8));
        wide::int128 i2(*reinterpret_cast<const std::uint64_t*>(data + 16), *reinterpret_cast<const std::uint64_t*>(data + 24));
        auto f128_1 = Fixed128<12>::from_raw(i1);
        auto f128_2 = Fixed128<12>::from_raw(i2);
        auto p128 = mul(f128_1, f128_2, mode);
        auto q128 = div(f128_1, f128_2, mode);
        (void)p128; (void)q128;

        // Mixed arithmetic
        auto mixed_res = mul_to<Fixed128<12>>(Fixed64<4>::from_raw(static_cast<std::int64_t>(i1.low)),
                                               Fixed64<8>::from_raw(static_cast<std::int64_t>(i2.low)), mode);
        (void)mixed_res;
    }

    return 0;
}
