# Third-party implementations used only by the optional competitor benchmark.
#
# Every source dependency is pinned. Header-only projects are populated without
# adding their CMake projects, so they cannot inject tests, install rules or
# cache options into fixedwide's build.
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(fpm
    GIT_REPOSITORY https://github.com/MikeLankamp/fpm.git
    GIT_TAG        v1.1.0
    GIT_SHALLOW    TRUE)
FetchContent_Declare(cnl
    GIT_REPOSITORY https://github.com/johnmcfarlane/cnl.git
    GIT_TAG        v1.1.7
    GIT_SHALLOW    TRUE)
FetchContent_Declare(decimal_for_cpp
    GIT_REPOSITORY https://github.com/vpiotr/decimal_for_cpp.git
    GIT_TAG        599372ee214ab37b5c0fc68148352321978f20ed)
FetchContent_Declare(boost_decimal
    GIT_REPOSITORY https://github.com/boostorg/decimal.git
    GIT_TAG        1297a5efcb2368969f322d0addb3149ed4cbdd50)

FetchContent_Populate(fpm)
FetchContent_Populate(cnl)
FetchContent_Populate(decimal_for_cpp)
FetchContent_Populate(boost_decimal)

add_library(fixedwide_competitor_deps INTERFACE)
target_include_directories(fixedwide_competitor_deps SYSTEM INTERFACE
    ${fpm_SOURCE_DIR}/include
    ${cnl_SOURCE_DIR}/include
    ${decimal_for_cpp_SOURCE_DIR}/include
    ${boost_decimal_SOURCE_DIR}/include
)

set(FIXEDWIDE_MPDECIMAL_ROOT "" CACHE PATH
    "Prefix containing decimal.hh, libmpdec++ and libmpdec")
set(FIXEDWIDE_MPDECIMAL_VERSION "system-or-local" CACHE STRING
    "Version label recorded for the mpdecimal benchmark dependency")
option(FIXEDWIDE_REQUIRE_MPDECIMAL
    "Fail configuration when the mpdecimal C++ library is unavailable" OFF)

set(_fixedwide_mpdecimal_hints)
if(FIXEDWIDE_MPDECIMAL_ROOT)
    list(APPEND _fixedwide_mpdecimal_hints
        ${FIXEDWIDE_MPDECIMAL_ROOT}
        ${FIXEDWIDE_MPDECIMAL_ROOT}/include
        ${FIXEDWIDE_MPDECIMAL_ROOT}/lib
        ${FIXEDWIDE_MPDECIMAL_ROOT}/lib64)
endif()

find_path(MPDECXX_INCLUDE_DIR NAMES decimal.hh
    HINTS ${_fixedwide_mpdecimal_hints}
    PATH_SUFFIXES include)
find_library(MPDECXX_LIBRARY NAMES mpdec++ libmpdec++
    HINTS ${_fixedwide_mpdecimal_hints}
    PATH_SUFFIXES lib lib64)
find_library(MPDEC_LIBRARY NAMES mpdec libmpdec
    HINTS ${_fixedwide_mpdecimal_hints}
    PATH_SUFFIXES lib lib64)

if(MPDECXX_INCLUDE_DIR AND MPDECXX_LIBRARY AND MPDEC_LIBRARY)
    find_package(Threads REQUIRED)
    message(STATUS "fixedwide: mpdecimal C++ library: ${MPDECXX_LIBRARY}")
    target_include_directories(fixedwide_competitor_deps SYSTEM INTERFACE
        ${MPDECXX_INCLUDE_DIR})
    target_link_libraries(fixedwide_competitor_deps INTERFACE
        ${MPDECXX_LIBRARY} ${MPDEC_LIBRARY} Threads::Threads)
    if(UNIX AND NOT APPLE)
        target_link_libraries(fixedwide_competitor_deps INTERFACE m)
    endif()
    target_compile_definitions(fixedwide_competitor_deps INTERFACE
        FIXEDWIDE_HAVE_MPDECIMAL=1)
    set(_fixedwide_mpdecimal_label "${FIXEDWIDE_MPDECIMAL_VERSION}")
else()
    set(_fixedwide_mpdecimal_label "not-found")
    if(FIXEDWIDE_REQUIRE_MPDECIMAL)
        message(FATAL_ERROR
            "FIXEDWIDE_REQUIRE_MPDECIMAL=ON, but decimal.hh/libmpdec++/libmpdec "
            "were not found. Set FIXEDWIDE_MPDECIMAL_ROOT to the install prefix.")
    endif()
    message(STATUS
        "fixedwide: mpdecimal not found; its rows will be omitted")
endif()

if(DEFINED Boost_VERSION_STRING)
    set(_fixedwide_boost_label "${Boost_VERSION_STRING}")
elseif(DEFINED Boost_VERSION)
    set(_fixedwide_boost_label "${Boost_VERSION}")
else()
    set(_fixedwide_boost_label "unknown")
endif()

string(CONCAT _fixedwide_dependency_text
    "fixedwide=${PROJECT_VERSION}; "
    "decimal_for_cpp=599372ee214ab37b5c0fc68148352321978f20ed; "
    "CNL=v1.1.7; fpm=v1.1.0; "
    "Boost.Decimal=1297a5efcb2368969f322d0addb3149ed4cbdd50; "
    "Boost.Multiprecision=${_fixedwide_boost_label}; "
    "mpdecimal=${_fixedwide_mpdecimal_label}")


file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/competitor_versions.hpp
"#pragma once\n"
"namespace fixedwide_bench {\n"
"inline constexpr const char competitor_dependencies[] = "
"\"${_fixedwide_dependency_text}\";\n"
"} // namespace fixedwide_bench\n")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/competitor_versions.txt
"${_fixedwide_dependency_text}\n")
