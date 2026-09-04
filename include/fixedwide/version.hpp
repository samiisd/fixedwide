#pragma once

/// \file
/// The library's version, as preprocessor macros for `#if` and as constants for
/// ordinary code. Both are generated from the same release and always agree.


#define FIXEDWIDE_VERSION_MAJOR 0
#define FIXEDWIDE_VERSION_MINOR 5
#define FIXEDWIDE_VERSION_PATCH 0
#define FIXEDWIDE_VERSION_PRERELEASE ""
#define FIXEDWIDE_VERSION_STRING "0.5.0"

namespace fixedwide {
/// Major version. 0 while the API is still allowed to change.
inline constexpr unsigned version_major = 0;
/// Minor version.
inline constexpr unsigned version_minor = 5;
/// Patch version.
inline constexpr unsigned version_patch = 0;
/// Pre-release tag, empty for a final release.
inline constexpr const char* version_prerelease = "";
/// The full version, as in "0.5.0".
inline constexpr const char* version_string = "0.5.0";
}
