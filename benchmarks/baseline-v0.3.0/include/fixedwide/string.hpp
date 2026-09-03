#pragma once
#include <fixedwide/chars.hpp>
#include <string>

namespace fixedwide {
inline namespace FIXEDWIDE_SCALE_NAMESPACE {
// Optional allocating convenience layer. Allocation failures propagate normally.
[[nodiscard]] std::expected<std::string, FormatError> to_string(FP64, FormatOptions = {});
[[nodiscard]] std::expected<std::string, FormatError> to_string(FP128, FormatOptions = {});
} // inline namespace
} // namespace fixedwide
