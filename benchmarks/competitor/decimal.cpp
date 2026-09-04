#include "competitor_common.hpp"

#include <boost/decimal.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#if defined(FIXEDWIDE_HAVE_MPDECIMAL)
#include <decimal.hh>
#endif

#include <array>
#include <ios>

namespace fixedwide_competitor {
namespace {

using BoostDecimal = boost::decimal::decimal64_t;

BoostDecimal parse_boost_decimal(std::string_view text) {
    BoostDecimal value{};
    const auto result =
        boost::decimal::from_chars(text.data(), text.data() + text.size(), value);
    if (!result || result.ptr != text.data() + text.size()) {
        fail("Boost.Decimal could not parse an exact fixture");
    }
    return value;
}

void benchmark_boost_decimal(const Fixtures& fixtures) {
    std::vector<BoostDecimal> add_lhs(data_size), add_rhs(data_size), add_expected(data_size);
    std::vector<BoostDecimal> mul_lhs(data_size), mul_rhs(data_size), mul_expected(data_size);
    std::vector<BoostDecimal> div_lhs(data_size), div_rhs(data_size), div_expected(data_size);
    std::vector<BoostDecimal> text_values(data_size);

    for (std::size_t i = 0; i < data_size; ++i) {
        add_lhs[i] = parse_boost_decimal(fixtures.add[i].lhs_text);
        add_rhs[i] = parse_boost_decimal(fixtures.add[i].rhs_text);
        add_expected[i] = parse_boost_decimal(fixtures.add[i].expected_text);
        mul_lhs[i] = parse_boost_decimal(fixtures.mul[i].lhs_text);
        mul_rhs[i] = parse_boost_decimal(fixtures.mul[i].rhs_text);
        mul_expected[i] = parse_boost_decimal(fixtures.mul[i].expected_text);
        div_lhs[i] = parse_boost_decimal(fixtures.div[i].lhs_text);
        div_rhs[i] = parse_boost_decimal(fixtures.div[i].rhs_text);
        div_expected[i] = parse_boost_decimal(fixtures.div[i].expected_text);
        text_values[i] = parse_boost_decimal(fixtures.text[i].text);

        expect(add_lhs[i] + add_rhs[i] == add_expected[i],
               "Boost.Decimal addition disagrees with the decimal oracle");
        expect(mul_lhs[i] * mul_rhs[i] == mul_expected[i],
               "Boost.Decimal multiplication disagrees with the decimal oracle");
        expect(div_lhs[i] / div_rhs[i] == div_expected[i],
               "Boost.Decimal division disagrees with the decimal oracle");

        std::array<char, 96> buffer{};
        const auto formatted = boost::decimal::to_chars(
            buffer.data(), buffer.data() + buffer.size(), text_values[i],
            boost::decimal::chars_format::fixed, 4);
        expect(formatted &&
                   std::string_view(
                       buffer.data(),
                       static_cast<std::size_t>(formatted.ptr - buffer.data())) ==
                       fixtures.text[i].text,
               "Boost.Decimal formatting disagrees with canonical text");
    }

    row("boost.decimal", "decimal64_t", "decimal_float_exact_4", "add", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = add_lhs[i & (data_size - 1)] + add_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("boost.decimal", "decimal64_t", "decimal_float_exact_4", "mul", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = mul_lhs[i & (data_size - 1)] * mul_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("boost.decimal", "decimal64_t", "decimal_float_exact_4", "div", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = div_lhs[i & (data_size - 1)] / div_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("boost.decimal", "decimal64_t", "decimal_float_exact_4", "parse", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            BoostDecimal value{};
            const auto& text = fixtures.text[i & (data_size - 1)].text;
            const auto result =
                boost::decimal::from_chars(text.data(), text.data() + text.size(), value);
            consume(value);
            consume(result);
        }
    });
    row("boost.decimal", "decimal64_t", "decimal_float_exact_4", "format_fixed", [&](std::size_t n) {
        std::array<char, 96> buffer{};
        for (std::size_t i = 0; i < n; ++i) {
            const auto result = boost::decimal::to_chars(
                buffer.data(), buffer.data() + buffer.size(),
                text_values[i & (data_size - 1)],
                boost::decimal::chars_format::fixed, 4);
            consume(result);
            consume_bytes(buffer.data(),
                          result ? static_cast<std::size_t>(result.ptr - buffer.data()) : 0);
        }
    });
}

#if defined(FIXEDWIDE_HAVE_MPDECIMAL)
void benchmark_mpdecimal(const Fixtures& fixtures) {
    using T = decimal::Decimal;
    std::vector<T> add_lhs, add_rhs, add_expected;
    std::vector<T> mul_lhs, mul_rhs, mul_expected;
    std::vector<T> div_lhs, div_rhs, div_expected;
    std::vector<T> text_values;
    for (auto* values : {&add_lhs, &add_rhs, &add_expected,
                         &mul_lhs, &mul_rhs, &mul_expected,
                         &div_lhs, &div_rhs, &div_expected, &text_values}) {
        values->reserve(data_size);
    }

    for (std::size_t i = 0; i < data_size; ++i) {
        add_lhs.emplace_back(fixtures.add[i].lhs_text.c_str());
        add_rhs.emplace_back(fixtures.add[i].rhs_text.c_str());
        add_expected.emplace_back(fixtures.add[i].expected_text.c_str());
        mul_lhs.emplace_back(fixtures.mul[i].lhs_text.c_str());
        mul_rhs.emplace_back(fixtures.mul[i].rhs_text.c_str());
        mul_expected.emplace_back(fixtures.mul[i].expected_text.c_str());
        div_lhs.emplace_back(fixtures.div[i].lhs_text.c_str());
        div_rhs.emplace_back(fixtures.div[i].rhs_text.c_str());
        div_expected.emplace_back(fixtures.div[i].expected_text.c_str());
        text_values.emplace_back(fixtures.text[i].text.c_str());

        expect(add_lhs.back() + add_rhs.back() == add_expected.back(),
               "mpdecimal addition disagrees with the decimal oracle");
        expect(mul_lhs.back() * mul_rhs.back() == mul_expected.back(),
               "mpdecimal multiplication disagrees with the decimal oracle");
        expect(div_lhs.back() / div_rhs.back() == div_expected.back(),
               "mpdecimal division disagrees with the decimal oracle");
        expect(text_values.back().format(".4f") == fixtures.text[i].text,
               "mpdecimal formatting disagrees with canonical text");
    }

    row("mpdecimal", "Decimal", "arbitrary_decimal_exact_4", "add", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result = add_lhs[i & (data_size - 1)] + add_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("mpdecimal", "Decimal", "arbitrary_decimal_exact_4", "mul", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result = mul_lhs[i & (data_size - 1)] * mul_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("mpdecimal", "Decimal", "arbitrary_decimal_exact_4", "div", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result = div_lhs[i & (data_size - 1)] / div_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("mpdecimal", "Decimal", "arbitrary_decimal_exact_4", "parse", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result(fixtures.text[i & (data_size - 1)].text.c_str());
            consume(result);
        }
    });
    row("mpdecimal", "Decimal", "arbitrary_decimal_exact_4", "format_fixed", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const std::string result = text_values[i & (data_size - 1)].format(".4f");
            consume(result);
            consume_bytes(result.data(), result.size());
        }
    });
}
#endif

void benchmark_cpp_dec_float(const Fixtures& fixtures) {
    using T = boost::multiprecision::cpp_dec_float_50;
    std::vector<T> add_lhs(data_size), add_rhs(data_size), add_expected(data_size);
    std::vector<T> mul_lhs(data_size), mul_rhs(data_size), mul_expected(data_size);
    std::vector<T> div_lhs(data_size), div_rhs(data_size), div_expected(data_size);
    std::vector<T> text_values(data_size);

    for (std::size_t i = 0; i < data_size; ++i) {
        add_lhs[i] = T(fixtures.add[i].lhs_text);
        add_rhs[i] = T(fixtures.add[i].rhs_text);
        add_expected[i] = T(fixtures.add[i].expected_text);
        mul_lhs[i] = T(fixtures.mul[i].lhs_text);
        mul_rhs[i] = T(fixtures.mul[i].rhs_text);
        mul_expected[i] = T(fixtures.mul[i].expected_text);
        div_lhs[i] = T(fixtures.div[i].lhs_text);
        div_rhs[i] = T(fixtures.div[i].rhs_text);
        div_expected[i] = T(fixtures.div[i].expected_text);
        text_values[i] = T(fixtures.text[i].text);

        expect(add_lhs[i] + add_rhs[i] == add_expected[i],
               "cpp_dec_float_50 addition disagrees with the decimal oracle");
        expect(mul_lhs[i] * mul_rhs[i] == mul_expected[i],
               "cpp_dec_float_50 multiplication disagrees with the decimal oracle");
        expect(div_lhs[i] / div_rhs[i] == div_expected[i],
               "cpp_dec_float_50 division disagrees with the decimal oracle");
        expect(text_values[i].str(4, std::ios_base::fixed) == fixtures.text[i].text,
               "cpp_dec_float_50 formatting disagrees with canonical text");
    }

    row("boost.multiprecision", "cpp_dec_float_50", "arbitrary_decimal_exact_4", "add", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result = add_lhs[i & (data_size - 1)] + add_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("boost.multiprecision", "cpp_dec_float_50", "arbitrary_decimal_exact_4", "mul", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result = mul_lhs[i & (data_size - 1)] * mul_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("boost.multiprecision", "cpp_dec_float_50", "arbitrary_decimal_exact_4", "div", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result = div_lhs[i & (data_size - 1)] / div_rhs[i & (data_size - 1)];
            consume(result);
        }
    });
    row("boost.multiprecision", "cpp_dec_float_50", "arbitrary_decimal_exact_4", "parse", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const T result(fixtures.text[i & (data_size - 1)].text);
            consume(result);
        }
    });
    row("boost.multiprecision", "cpp_dec_float_50", "arbitrary_decimal_exact_4", "format_fixed", [&](std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            const std::string result =
                text_values[i & (data_size - 1)].str(4, std::ios_base::fixed);
            consume(result);
            consume_bytes(result.data(), result.size());
        }
    });
}

} // namespace

void benchmark_decimal_float(const Fixtures& fixtures) {
    benchmark_boost_decimal(fixtures);
#if defined(FIXEDWIDE_HAVE_MPDECIMAL)
    benchmark_mpdecimal(fixtures);
#endif
    benchmark_cpp_dec_float(fixtures);
}

} // namespace fixedwide_competitor
