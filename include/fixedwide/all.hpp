#pragma once

/// \file
/// Every public header at once. Convenient, and the most expensive way to use
/// the library: each header below stands alone, so include only what a
/// translation unit needs when build time matters.

#include <fixedwide/fixed.hpp>
#include <fixedwide/rounding.hpp>
#include <fixedwide/error.hpp>
#include <fixedwide/wide.hpp>
#include <fixedwide/arithmetic.hpp>
#include <fixedwide/mixed.hpp>
#include <fixedwide/chars.hpp>
#include <fixedwide/binary.hpp>
#include <fixedwide/floating.hpp>
#include <fixedwide/string.hpp>
#include <fixedwide/version.hpp>

// Deliberately NOT included here, because each one costs more to compile than
// the whole of the rest of this library put together:
//
//   <fixedwide/format.hpp>     pulls <format>      435 ms
//   <fixedwide/iostream.hpp>   pulls <iostream>    450 ms
//   <fixedwide/hash.hpp>       pulls <functional>  103 ms
//
// (clang 22, -O2, medians; this header without them is 199 ms and was 558 ms
// with them.) They are adapters to standard-library facilities, not part of the
// numeric API, and most translation units that want fixed-point arithmetic do
// not format with std::format, stream to std::ostream, or use the values as
// unordered-container keys. Include whichever you need, next to this one:
//
//   #include <fixedwide/all.hpp>
//   #include <fixedwide/iostream.hpp>   // only where you actually stream
//
// Everything that computes is here; only the three integrations moved out.
