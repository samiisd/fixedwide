#include "competitor_common.hpp"

#include <fixedwide/arithmetic.hpp>
#include <fixedwide/chars.hpp>

#include <cnl/scaled_integer.h>
#include <decimal.h>

#include <array>

namespace fixedwide_competitor {
namespace {

template<unsigned Decimals>
void benchmark_fixedwide(const Fixtures& fixtures, const char* type_name, const char* semantic_class) {
    using T = fixedwide::basic_fixed<64, Decimals>;
    std::vector<T> add_lhs(data_size), add_rhs(data_size);
    std::vector<T> mul_lhs(data_size), mul_rhs(data_size);
    std::vector<T> div_lhs(data_size), div_rhs(data_size);
    std::vector<T> text_values(data_size);

    for (std::size_t i = 0; i < data_size; ++i) {
        add_lhs[i] = T::from_raw(fixtures.add[i].lhs_raw);
        add_rhs[i] = T::from_raw(fixtures.add[i].rhs_raw);
        mul_lhs[i] = T::from_raw(fixtures.mul[i].lhs_raw);
        mul_rhs[i] = T::from_raw(fixtures.mul[i].rhs_raw);
        div_lhs[i] = T::from_raw(fixtures.div[i].lhs_raw);
        div_rhs[i] = T::from_raw(fixtures.div[i].rhs_raw);
        text_values[i] = T::from_raw(fixtures.text[i].raw);

        const auto add_result = fixedwide::add(add_lhs[i], add_rhs[i]);
        expect(add_result && add_result->raw() == fixtures.add[i].expected_raw,
               "fixedwide addition disagrees with the integer oracle");
        const auto mul_result = fixedwide::mul(mul_lhs[i], mul_rhs[i]);
        expect(mul_result && mul_result->raw() == fixtures.mul[i].expected_raw,
               "fixedwide multiplication disagrees with the integer oracle");
        const auto div_result = fixedwide::div(div_lhs[i], div_rhs[i]);
        expect(div_result && div_result->raw() == fixtures.div[i].expected_raw,
               "fixedwide division disagrees with the integer oracle");
        const auto parsed = fixedwide::parse<T>(fixtures.text[i].text);
        expect(parsed && parsed->raw() == fixtures.text[i].raw, "fixedwide parsing disagrees with the integer oracle");

        std::array<char, 96> buffer{};
        const auto formatted = fixedwide::to_chars(buffer.data(), buffer.size(), text_values[i]);
        expect(formatted && *formatted == fixtures.text[i].text.size() &&
                   std::string_view(buffer.data(), *formatted) == fixtures.text[i].text,
               "fixedwide formatting disagrees with canonical text");
    }

    row("fixedwide", type_name, semantic_class, "add", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = fixedwide::add(add_lhs[i & (data_size - 1)], add_rhs[i & (data_size - 1)]);
            consume(result);
        }
    });
    row("fixedwide", type_name, semantic_class, "mul", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = fixedwide::mul(mul_lhs[i & (data_size - 1)], mul_rhs[i & (data_size - 1)]);
            consume(result);
        }
    });
    row("fixedwide", type_name, semantic_class, "div", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = fixedwide::div(div_lhs[i & (data_size - 1)], div_rhs[i & (data_size - 1)]);
            consume(result);
        }
    });
    row("fixedwide", type_name, semantic_class, "parse", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = fixedwide::parse<T>(fixtures.text[i & (data_size - 1)].text);
            consume(result);
        }
    });
    row("fixedwide", type_name, semantic_class, "format_fixed", [&](std::size_t n) {
        std::array<char, 96> buffer{};
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = fixedwide::to_chars(buffer.data(), buffer.size(), text_values[i & (data_size - 1)]);
            consume(result);
            consume_bytes(buffer.data(), result ? *result : 0);
        }
    });
}

template<unsigned Decimals>
void benchmark_decimal_for_cpp(const Fixtures& fixtures, const char* type_name, const char* semantic_class) {
    using T = dec::decimal<static_cast<int>(Decimals), dec::half_even_round_policy>;
    std::vector<T> add_lhs(data_size), add_rhs(data_size);
    std::vector<T> mul_lhs(data_size), mul_rhs(data_size);
    std::vector<T> div_lhs(data_size), div_rhs(data_size);
    std::vector<T> text_values(data_size);

    for (std::size_t i = 0; i < data_size; ++i) {
        add_lhs[i].setUnbiased(fixtures.add[i].lhs_raw);
        add_rhs[i].setUnbiased(fixtures.add[i].rhs_raw);
        mul_lhs[i].setUnbiased(fixtures.mul[i].lhs_raw);
        mul_rhs[i].setUnbiased(fixtures.mul[i].rhs_raw);
        div_lhs[i].setUnbiased(fixtures.div[i].lhs_raw);
        div_rhs[i].setUnbiased(fixtures.div[i].rhs_raw);
        text_values[i].setUnbiased(fixtures.text[i].raw);

        expect((add_lhs[i] + add_rhs[i]).getUnbiased() == fixtures.add[i].expected_raw,
               "decimal_for_cpp addition disagrees with the integer oracle");
        expect((mul_lhs[i] * mul_rhs[i]).getUnbiased() == fixtures.mul[i].expected_raw,
               "decimal_for_cpp multiplication disagrees with the integer oracle");
        expect((div_lhs[i] / div_rhs[i]).getUnbiased() == fixtures.div[i].expected_raw,
               "decimal_for_cpp division disagrees with the integer oracle");
        const T parsed(fixtures.text[i].text);
        expect(parsed.getUnbiased() == fixtures.text[i].raw,
               "decimal_for_cpp parsing disagrees with the integer oracle");
        expect(dec::toString(text_values[i]) == fixtures.text[i].text,
               "decimal_for_cpp formatting disagrees with canonical text");
    }

    row("decimal_for_cpp", type_name, semantic_class, "add", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result = add_lhs[i & (data_size - 1)] + add_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("decimal_for_cpp", type_name, semantic_class, "mul", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result = mul_lhs[i & (data_size - 1)] * mul_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("decimal_for_cpp", type_name, semantic_class, "div", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result = div_lhs[i & (data_size - 1)] / div_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("decimal_for_cpp", type_name, semantic_class, "parse", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result(fixtures.text[i & (data_size - 1)].text);
            consume(result);
        }
    });
    row("decimal_for_cpp", type_name, semantic_class, "format_fixed", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const std::string result = dec::toString(text_values[i & (data_size - 1)]);
            consume(result);
            consume_bytes(result.data(), result.size());
        }
    });
}

void benchmark_cnl_decimal(const Fixtures& fixtures) {
    using T = cnl::scaled_integer<std::int64_t, cnl::power<-4, 10>>;
    std::vector<T> add_lhs(data_size), add_rhs(data_size);
    std::vector<T> mul_lhs(data_size), mul_rhs(data_size);
    std::vector<T> div_lhs(data_size), div_rhs(data_size);
    bool saw_fractional_quotient_loss = false;

    for (std::size_t i = 0; i < data_size; ++i) {
        add_lhs[i] = cnl::_impl::from_rep<T>(fixtures.add[i].lhs_raw);
        add_rhs[i] = cnl::_impl::from_rep<T>(fixtures.add[i].rhs_raw);
        mul_lhs[i] = cnl::_impl::from_rep<T>(fixtures.mul[i].lhs_raw);
        mul_rhs[i] = cnl::_impl::from_rep<T>(fixtures.mul[i].rhs_raw);
        div_lhs[i] = cnl::_impl::from_rep<T>(fixtures.div[i].lhs_raw);
        div_rhs[i] = cnl::_impl::from_rep<T>(fixtures.div[i].rhs_raw);

        const T add_result = add_lhs[i] + add_rhs[i];
        const T mul_result = mul_lhs[i] * mul_rhs[i];
        const T div_result = div_lhs[i] / div_rhs[i];
        expect(cnl::_impl::to_rep(add_result) == fixtures.add[i].expected_raw,
               "CNL radix-10 addition disagrees with the integer oracle");
        expect(cnl::_impl::to_rep(mul_result) == fixtures.mul[i].expected_raw,
               "CNL radix-10 multiplication disagrees with the integer oracle");

        // CNL's same-type division first divides the raw representations at
        // exponent zero, then converts that integral quotient back to scale 4.
        // Fractional quotient digits are therefore gone before the conversion.
        const __int128 whole = static_cast<__int128>(fixtures.div[i].lhs_raw) / fixtures.div[i].rhs_raw;
        const __int128 expected_raw = whole * fixtures.scale;
        expect(expected_raw >= std::numeric_limits<std::int64_t>::min() &&
                   expected_raw <= std::numeric_limits<std::int64_t>::max() &&
                   cnl::_impl::to_rep(div_result) == static_cast<std::int64_t>(expected_raw),
               "CNL radix-10 division disagrees with its same-type quotient model");
        saw_fractional_quotient_loss = saw_fractional_quotient_loss || expected_raw != fixtures.div[i].expected_raw;
    }
    expect(saw_fractional_quotient_loss, "CNL adjacent division fixture did not expose fractional quotient loss");

    constexpr const char* type_name = "scaled_integer<int64,power<-4,10>>";
    row("cnl", type_name, "decimal_fixed_exact_4", "add", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result = add_lhs[i & (data_size - 1)] + add_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("cnl", type_name, "decimal_fixed_exact_4", "mul", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result = mul_lhs[i & (data_size - 1)] * mul_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("cnl", type_name, "decimal_fixed_adjacent", "div_same_type", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result = div_lhs[i & (data_size - 1)] / div_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
}

} // namespace

void benchmark_fixed_scale4(const Fixtures& fixtures) {
    benchmark_fixedwide<4>(fixtures, "Fixed64<4>", "decimal_fixed_exact_4");
    benchmark_decimal_for_cpp<4>(fixtures, "decimal<4,half_even>", "decimal_fixed_exact_4");
    benchmark_cnl_decimal(fixtures);
}

void benchmark_fixed_scale12(const Fixtures& fixtures) {
    benchmark_fixedwide<12>(fixtures, "Fixed64<12>", "decimal_fixed_exact_12");
    benchmark_decimal_for_cpp<12>(fixtures, "decimal<12,half_even>", "decimal_fixed_exact_12");
}

} // namespace fixedwide_competitor
