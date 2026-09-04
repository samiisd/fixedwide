#pragma once

/// \file
/// `std::format` support: `std::format("{}", value)` prints all decimals with
/// nearest-even rounding, and every alignment, width and fill spec that
/// `std::formatter<std::string_view>` understands works too. A value that
/// cannot be formatted renders as empty rather than throwing.

#include <fixedwide/chars.hpp>
#include <cstddef>
#include <format>
#include <string_view>

/// Formatter for any `basic_fixed`.
///
/// Both template parameters of `std::formatter` are named on purpose. Writing
/// only the first relies on the default `charT`, and the `format` member must
/// be a template over the context rather than taking `std::format_context&`:
/// an implementation is free to instantiate the formatter with its own context
/// type, and libc++ does -- it uses a `back_insert_iterator` where
/// `std::format_context` has a `char*`, so a hard-coded context silently made
/// every `basic_fixed` non-formattable there. libstdc++ happened not to notice.
template<std::size_t Bits, unsigned Decimals>
struct std::formatter<fixedwide::basic_fixed<Bits, Decimals>, char>
    : std::formatter<std::string_view, char> {
    template<typename FormatContext>
    auto format(const fixedwide::basic_fixed<Bits, Decimals>& value, FormatContext& ctx) const {
        char buffer[fixedwide::text_capacity];
        const auto written = fixedwide::to_chars(buffer, sizeof(buffer), value);
        const std::string_view text = written ? std::string_view(buffer, *written)
                                              : std::string_view{};
        return std::formatter<std::string_view, char>::format(text, ctx);
    }
};
