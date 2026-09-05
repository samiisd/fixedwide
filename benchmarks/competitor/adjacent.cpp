#include "competitor_common.hpp"

#include <fixedwide/binary.hpp>

#include <cnl/scaled_integer.h>
#include <fpm/fixed.hpp>

#include <array>
#include <charconv>
#include <cmath>
#include <cstring>
#include <limits>

namespace fixedwide_competitor {

void benchmark_adjacent_types(const Fixtures& fixtures) {
    using CnlBinary = cnl::scaled_integer<std::int64_t, cnl::power<-32>>;
    using FpmBinary = fpm::fixed<std::int64_t, __int128, 32>;
    std::vector<double> lhs(data_size), rhs(data_size);
    std::vector<CnlBinary> cnl_lhs(data_size), cnl_rhs(data_size);
    std::vector<FpmBinary> fpm_lhs(data_size), fpm_rhs(data_size);

    // Both binary libraries receive the SAME bounded inputs. Dividing the
    // scale-4 fixture by 32 puts both magnitudes below 0.5, keeping CNL's
    // un-widened raw product inside int64_t. Prove that bound per input below;
    // never execute an overflowing operation and then inspect its result.
    const double scale = static_cast<double>(fixtures.scale) * 32.0;
    constexpr __int128 binary_scale = static_cast<__int128>(1) << 32;
    for (std::size_t i = 0; i < data_size; ++i) {
        lhs[i] = static_cast<double>(fixtures.mul[i].lhs_raw) / scale;
        rhs[i] = static_cast<double>(fixtures.mul[i].rhs_raw) / scale;
        cnl_lhs[i] = CnlBinary{lhs[i]};
        cnl_rhs[i] = CnlBinary{rhs[i]};
        fpm_lhs[i] = FpmBinary{lhs[i]};
        fpm_rhs[i] = FpmBinary{rhs[i]};

        const auto lhs_raw = cnl::_impl::to_rep(cnl_lhs[i]);
        const auto rhs_raw = cnl::_impl::to_rep(cnl_rhs[i]);
        const __int128 product = static_cast<__int128>(lhs_raw) * rhs_raw;
        expect(rhs_raw != 0, "CNL binary divisor is zero");
        expect(product >= std::numeric_limits<std::int64_t>::min() &&
                   product <= std::numeric_limits<std::int64_t>::max(),
               "CNL binary raw product would overflow; fixture must be bounded before execution");

        const CnlBinary cnl_add = cnl_lhs[i] + cnl_rhs[i];
        expect(std::abs(static_cast<double>(cnl_add) - (lhs[i] + rhs[i])) < 1e-4,
               "CNL binary addition exceeded its declared tolerance");
        const CnlBinary cnl_mul = cnl_lhs[i] * cnl_rhs[i];
        expect(std::abs(static_cast<double>(cnl_mul) - (lhs[i] * rhs[i])) < 1e-4,
               "CNL binary multiplication exceeded its declared tolerance");

        // Same-type division loses fractional quotient bits at exponent zero.
        // Label it separately rather than presenting it as full-precision div.
        const __int128 expected_div_raw = (static_cast<__int128>(lhs_raw) / rhs_raw) * binary_scale;
        expect(expected_div_raw >= std::numeric_limits<std::int64_t>::min() &&
                   expected_div_raw <= std::numeric_limits<std::int64_t>::max(),
               "CNL binary quotient conversion would overflow");
        const CnlBinary cnl_div = cnl_lhs[i] / cnl_rhs[i];
        expect(cnl::_impl::to_rep(cnl_div) == static_cast<std::int64_t>(expected_div_raw),
               "CNL binary division disagrees with its same-type quotient model");

        expect(std::abs(static_cast<double>(fpm_lhs[i] + fpm_rhs[i]) - (lhs[i] + rhs[i])) < 1e-4,
               "fpm addition exceeded its declared tolerance");
        expect(std::abs(static_cast<double>(fpm_lhs[i] * fpm_rhs[i]) - (lhs[i] * rhs[i])) < 1e-4,
               "fpm multiplication exceeded its declared tolerance");
        expect(std::abs(static_cast<double>(fpm_lhs[i] / fpm_rhs[i]) - (lhs[i] / rhs[i])) < 1e-4,
               "fpm division exceeded its declared tolerance");
    }

    row("cnl", "scaled_integer<int64,power<-32>>", "binary_fixed_approx", "add", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const CnlBinary result = cnl_lhs[i & (data_size - 1)] + cnl_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("cnl", "scaled_integer<int64,power<-32>>", "binary_fixed_approx", "mul", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const CnlBinary result = cnl_lhs[i & (data_size - 1)] * cnl_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("cnl", "scaled_integer<int64,power<-32>>", "binary_fixed_approx", "div_same_type", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const CnlBinary result = cnl_lhs[i & (data_size - 1)] / cnl_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("fpm", "fixed<int64,int128,32>", "binary_fixed_approx", "add", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const FpmBinary result = fpm_lhs[i & (data_size - 1)] + fpm_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("fpm", "fixed<int64,int128,32>", "binary_fixed_approx", "mul", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const FpmBinary result = fpm_lhs[i & (data_size - 1)] * fpm_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("fpm", "fixed<int64,int128,32>", "binary_fixed_approx", "div", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const FpmBinary result = fpm_lhs[i & (data_size - 1)] / fpm_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
}

void benchmark_hardware_floors(const Fixtures& fixtures) {
    std::vector<double> add_lhs(data_size), add_rhs(data_size);
    std::vector<double> mul_lhs(data_size), mul_rhs(data_size);
    std::vector<double> div_lhs(data_size), div_rhs(data_size);
    std::vector<double> text_values(data_size);
    std::vector<std::int64_t> raw_lhs(data_size), raw_rhs(data_size);

    const double scale = static_cast<double>(fixtures.scale);
    for (std::size_t i = 0; i < data_size; ++i) {
        add_lhs[i] = static_cast<double>(fixtures.add[i].lhs_raw) / scale;
        add_rhs[i] = static_cast<double>(fixtures.add[i].rhs_raw) / scale;
        mul_lhs[i] = static_cast<double>(fixtures.mul[i].lhs_raw) / scale;
        mul_rhs[i] = static_cast<double>(fixtures.mul[i].rhs_raw) / scale;
        div_lhs[i] = static_cast<double>(fixtures.div[i].lhs_raw) / scale;
        div_rhs[i] = static_cast<double>(fixtures.div[i].rhs_raw) / scale;
        text_values[i] = static_cast<double>(fixtures.text[i].raw) / scale;
        raw_lhs[i] = fixtures.mul[i].lhs_raw;
        raw_rhs[i] = fixtures.mul[i].rhs_raw;
        expect(raw_rhs[i] != 0, "raw integer divisor is zero");
        const __int128 raw_product = static_cast<__int128>(raw_lhs[i]) * raw_rhs[i];
        expect(raw_product >= std::numeric_limits<std::int64_t>::min() &&
                   raw_product <= std::numeric_limits<std::int64_t>::max(),
               "raw integer multiplication fixture would overflow");
        expect(std::abs((add_lhs[i] + add_rhs[i]) - static_cast<double>(fixtures.add[i].expected_raw) / scale) < 1e-4,
               "hardware double add exceeds expected tolerance");
        expect(std::abs((mul_lhs[i] * mul_rhs[i]) - static_cast<double>(fixtures.mul[i].expected_raw) / scale) < 1e-4,
               "hardware double mul exceeds expected tolerance");
        expect(std::abs((div_lhs[i] / div_rhs[i]) - static_cast<double>(fixtures.div[i].expected_raw) / scale) < 1e-4,
               "hardware double div exceeds expected tolerance");

        double parsed = 0;
        const auto& text = fixtures.text[i].text;
        const auto parsed_result = std::from_chars(text.data(), text.data() + text.size(), parsed);
        expect(parsed_result.ec == std::errc{} && parsed_result.ptr == text.data() + text.size() &&
                   std::abs(parsed - text_values[i]) < 1e-12,
               "hardware double parse exceeds expected tolerance");
        std::array<char, 96> buffer{};
        const auto formatted =
            std::to_chars(buffer.data(), buffer.data() + buffer.size(), text_values[i], std::chars_format::fixed, 4);
        expect(formatted.ec == std::errc{} &&
                   std::string_view(buffer.data(), static_cast<std::size_t>(formatted.ptr - buffer.data())) == text,
               "hardware double formatting disagrees with canonical text");
    }

    row("std", "double", "hardware_baseline", "add", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const double result = add_lhs[i & (data_size - 1)] + add_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("std", "double", "hardware_baseline", "mul", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const double result = mul_lhs[i & (data_size - 1)] * mul_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("std", "double", "hardware_baseline", "div", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const double result = div_lhs[i & (data_size - 1)] / div_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("std", "double", "hardware_baseline", "parse", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto& text = fixtures.text[i & (data_size - 1)].text;
            double value = 0;
            const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
            consume(value);
            consume(result);
        }
    });
    row("std", "double", "hardware_baseline", "format_fixed", [&](std::size_t n) {
        std::array<char, 96> buffer{};
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(),
                                              text_values[i & (data_size - 1)], std::chars_format::fixed, 4);
            consume(result);
            consume_bytes(buffer.data(),
                          result.ec == std::errc{} ? static_cast<std::size_t>(result.ptr - buffer.data()) : 0);
        }
    });
    row("std", "int64_t", "hardware_baseline", "add_unchecked", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = raw_lhs[i & (data_size - 1)] + raw_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("std", "int64_t", "hardware_baseline", "mul_unchecked", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = raw_lhs[i & (data_size - 1)] * raw_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("std", "int64_t", "hardware_baseline", "div_unchecked", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = raw_lhs[i & (data_size - 1)] / raw_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
}

void benchmark_serialization(const Fixtures& fixtures) {
    using T = fixedwide::Fixed64<4>;
    using Bytes = std::array<std::uint8_t, sizeof(std::int64_t)>;
    std::vector<T> values(data_size);
    std::vector<Bytes> native_bytes(data_size), little_bytes(data_size);
    for (std::size_t i = 0; i < data_size; ++i) {
        values[i] = T::from_raw(fixtures.text[i].raw);
        little_bytes[i] = fixedwide::to_bytes<fixedwide::endian::little>(values[i]);
        const auto restored = fixedwide::from_bytes<T, fixedwide::endian::little>(little_bytes[i]);
        expect(restored.has_value() && *restored == values[i], "serialization roundtrip disagrees with oracle");
        const auto raw = fixtures.text[i].raw;
        std::memcpy(native_bytes[i].data(), &raw, sizeof raw);
    }

    row("std", "int64_t", "hardware_baseline", "memcpy_store", [&](std::size_t n) {
        Bytes buffer{};
        for (std::size_t i = 0; i < n; ++i) {
            const auto raw = fixtures.text[i & (data_size - 1)].raw;
            std::memcpy(buffer.data(), &raw, sizeof raw);
            consume(buffer);
        }
    });
    row("std", "int64_t", "hardware_baseline", "memcpy_load", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            std::int64_t result = 0;
            std::memcpy(&result, native_bytes[i & (data_size - 1)].data(), sizeof result);
            consume(result);
        }
    });
    row("fixedwide", "Fixed64<4>", "serialization", "to_bytes_little", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = fixedwide::to_bytes<fixedwide::endian::little>(values[i & (data_size - 1)]);
            consume(result);
        }
    });
    row("fixedwide", "Fixed64<4>", "serialization", "from_bytes_little", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = fixedwide::from_bytes<T, fixedwide::endian::little>(little_bytes[i & (data_size - 1)]);
            consume(result);
        }
    });
}

} // namespace fixedwide_competitor
