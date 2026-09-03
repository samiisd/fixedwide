#include <fixedwide/string.hpp>
namespace fixedwide {
inline namespace FIXEDWIDE_SCALE_NAMESPACE {
namespace {
template<class FP>
std::expected<std::string, FormatError> format_string(FP value, FormatOptions options) {
    char buffer[text_capacity];
    const auto size = to_chars(buffer, sizeof(buffer), value, options);
    if (!size) return std::unexpected(size.error());
    return std::string(buffer, *size);
}
}
std::expected<std::string, FormatError> to_string(FP64 value, FormatOptions options) { return format_string(value, options); }
std::expected<std::string, FormatError> to_string(FP128 value, FormatOptions options) { return format_string(value, options); }
} // inline namespace
} // namespace fixedwide
