#pragma once

/// \file
/// `std::format` support.
///
/// `std::format("{}", value)` prints every decimal the type carries.
/// `std::format("{:.2}", value)` prints two DECIMALS, rounded to nearest-even --
/// not the first two characters.
///
/// That distinction is the reason this formatter parses its own spec instead of
/// inheriting `std::formatter<std::string_view>`'s. Inheriting it made
/// precision mean "truncate the string", so `{:.2}` on 123.4567 produced "12".
/// For a decimal type that is a silently wrong number, which is the one thing
/// this library exists to prevent.

#include <fixedwide/chars.hpp>

#include <cstddef>
#include <format>
#include <string_view>

/// Formatter for any `basic_fixed`.
///
/// Grammar: `[[fill]align][width][.precision][f]`, a subset of the standard
/// floating-point spec with the same meaning.
///
///   `{}`        every decimal the type carries
///   `{:.2}`     two decimals, rounded nearest-even
///   `{:.2f}`    the same; `f` is accepted because callers reach for it
///   `{:>12}`    right-aligned in a field of twelve
///   `{:*^14.2}` fill, centre, width and decimals together
///
/// Both template parameters are named on purpose, and `format` is a template
/// over the context: an implementation may instantiate a formatter with its own
/// context type, and libc++ does, so a hard-coded `std::format_context&` makes
/// every `basic_fixed` non-formattable there.
template<std::size_t Bits, unsigned Decimals>
struct std::formatter<fixedwide::basic_fixed<Bits, Decimals>, char> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        const auto end = ctx.end();
        if (it == end || *it == '}') return it;

        // [[fill]align]. Fill is any character followed by an alignment one, so
        // the two-character form has to be tested first.
        const auto is_align = [](char c) { return c == '<' || c == '>' || c == '^'; };
        if (it + 1 != end && is_align(*(it + 1))) {
            m_fill = *it;
            m_align = *(it + 1);
            it += 2;
        } else if (is_align(*it)) {
            m_align = *it++;
        }

        // [width]
        while (it != end && *it >= '0' && *it <= '9') {
            m_width = m_width * 10 + static_cast<unsigned>(*it++ - '0');
        }

        // [.precision] -- decimals, not characters.
        if (it != end && *it == '.') {
            ++it;
            unsigned precision = 0;
            bool any = false;
            while (it != end && *it >= '0' && *it <= '9') {
                precision = precision * 10 + static_cast<unsigned>(*it++ - '0');
                any = true;
            }
            if (!any) throw std::format_error("fixedwide: '.' with no precision");
            if (precision > Decimals) {
                throw std::format_error("fixedwide: precision exceeds the type's decimals");
            }
            m_precision = precision;
            m_has_precision = true;
        }

        // An optional 'f', because it is what callers type for a fixed-point
        // value and rejecting it would be a surprise with no upside.
        if (it != end && *it == 'f') ++it;

        if (it != end && *it != '}') throw std::format_error("fixedwide: invalid format spec");
        return it;
    }

    template<typename FormatContext>
    auto format(const fixedwide::basic_fixed<Bits, Decimals>& value, FormatContext& ctx) const {
        char buffer[fixedwide::text_capacity];
        fixedwide::FormatOptions options{};
        if (m_has_precision) {
            options.digits = m_precision;
            options.explicit_digits = true;
        }
        const auto written = fixedwide::to_chars(buffer, sizeof(buffer), value, options);
        const std::string_view text = written ? std::string_view(buffer, *written) : std::string_view{};

        auto out = ctx.out();
        if (text.size() >= m_width) return std::copy(text.begin(), text.end(), out);

        const std::size_t padding = m_width - text.size();
        const std::size_t before = m_align == '>' ? padding : (m_align == '^' ? padding / 2 : 0);
        const std::size_t after = padding - before;
        for (std::size_t i = 0; i < before; ++i) *out++ = m_fill;
        out = std::copy(text.begin(), text.end(), out);
        for (std::size_t i = 0; i < after; ++i) *out++ = m_fill;
        return out;
    }

private:
    char m_fill = ' ';
    // Numbers right-align by default, as they do for every arithmetic type.
    char m_align = '>';
    std::size_t m_width = 0;
    unsigned m_precision = 0;
    bool m_has_precision = false;
};
