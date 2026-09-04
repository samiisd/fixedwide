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
#include <fixedwide/hash.hpp>
#include <fixedwide/string.hpp>
#include <fixedwide/format.hpp>
#include <fixedwide/iostream.hpp>
#include <fixedwide/version.hpp>
