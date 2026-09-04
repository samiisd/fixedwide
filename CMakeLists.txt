cmake_minimum_required(VERSION 3.25)
project(fixedwide VERSION 0.6.0 LANGUAGES CXX)

# clangd -- and every editor built on it -- needs a compile database to know the
# include paths and the standard, without which hover, go-to-definition and the
# doc comments in the public headers all degrade. Only when this is the top-level
# project, so a consumer's build is not changed by depending on us.
if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CheckCXXSourceCompiles)

option(FIXEDWIDE_BUILD_TESTS "Build deterministic unit tests" ${PROJECT_IS_TOP_LEVEL})
option(FIXEDWIDE_BUILD_ORACLE_TESTS "Build Boost.Multiprecision differential oracle tests" OFF)
option(FIXEDWIDE_BUILD_BENCHMARKS "Build benchmark harnesses" OFF)
option(FIXEDWIDE_FORCE_PORTABLE "Force portable backend without compiler __int128 / inline asm" OFF)
option(FIXEDWIDE_SANITIZE "Enable AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
option(FIXEDWIDE_COVERAGE "Enable Clang source coverage" OFF)
option(FIXEDWIDE_BUILD_FUZZER "Build Clang libFuzzer target" OFF)
option(FIXEDWIDE_BUILD_EXAMPLES "Build example applications" ${PROJECT_IS_TOP_LEVEL})
option(BUILD_SHARED_LIBS "Build shared library" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(fixedwide
    src/arithmetic.cpp
    src/mixed.cpp
    src/chars.cpp
    src/integer_chars.cpp
    src/floating.cpp
    src/bigint.cpp
)
add_library(fixedwide::fixedwide ALIAS fixedwide)

target_compile_features(fixedwide PUBLIC cxx_std_23)

target_include_directories(fixedwide PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(fixedwide PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(MSVC)
    # /utf-8 and /Zc:preprocessor are needed by the sources; /Zc:__cplusplus is
    # PUBLIC because a consumer that checks the macro should see the truth when
    # it includes these headers, and MSVC reports C++98 without it.
    target_compile_options(fixedwide PRIVATE /W4 /permissive- /utf-8 /Zc:preprocessor)
    target_compile_options(fixedwide PUBLIC /Zc:__cplusplus)
else()
    # -Wsign-conversion is on because this library is nothing but integer
    # index and width arithmetic: a signed/unsigned slip here is a wrong answer,
    # not a style question. The `warnings` CI job compiles the same set with
    # -Werror over every backend.
    target_compile_options(fixedwide PRIVATE
        -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -Wold-style-cast)
endif()

set_target_properties(fixedwide PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    POSITION_INDEPENDENT_CODE ON
)

# 128-bit division through __int128 is not an instruction: the compiler emits a
# call to __divti3 / __udivti3 / __umodti3 in its runtime library. clang-cl
# defines __SIZEOF_INT128__ but targets the MSVC CRT, which has no such symbols
# and does not link compiler-rt's builtins by default -- so the native path
# compiles there and then fails at link time with three undefined symbols.
#
# Test for it rather than naming the compiler: any toolchain that cannot divide
# a 128-bit value without a runtime library it does not have gets the portable
# multi-limb backend, which is differentially tested against the native one on
# every CI run.
if(NOT FIXEDWIDE_FORCE_PORTABLE)
    set(CMAKE_REQUIRED_QUIET ON)
    check_cxx_source_compiles("
        #if !defined(__SIZEOF_INT128__)
        #error no __int128
        #endif
        __int128 d(__int128 a, __int128 b) { return a / b; }
        unsigned __int128 u(unsigned __int128 a, unsigned __int128 b) { return a / b + a % b; }
        int main() { return (int)(d(3, 2) + u(3, 2)); }
    " FIXEDWIDE_INT128_DIVISION_LINKS)
    unset(CMAKE_REQUIRED_QUIET)
    if(NOT FIXEDWIDE_INT128_DIVISION_LINKS)
        message(STATUS "fixedwide: 128-bit division does not link here; using the portable backend")
        set(FIXEDWIDE_FORCE_PORTABLE ON)
    endif()
endif()

if(FIXEDWIDE_FORCE_PORTABLE)
    target_compile_definitions(fixedwide PUBLIC FIXEDWIDE_FORCE_PORTABLE)
endif()

if(FIXEDWIDE_SANITIZE)
    if(MSVC)
        target_compile_options(fixedwide PUBLIC /fsanitize=address)
    else()
        target_compile_options(fixedwide PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
        target_link_options(fixedwide PUBLIC -fsanitize=address,undefined)
    endif()
endif()

if(FIXEDWIDE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(fixedwide PUBLIC -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(fixedwide PUBLIC -fprofile-instr-generate)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(fixedwide PUBLIC --coverage)
        target_link_options(fixedwide PUBLIC --coverage)
    endif()
endif()

# Examples register ctest tests too, so testing has to be enabled before either
# subdirectory is added -- not inside the tests block.
if(FIXEDWIDE_BUILD_TESTS OR FIXEDWIDE_BUILD_ORACLE_TESTS OR FIXEDWIDE_BUILD_EXAMPLES)
    enable_testing()
endif()

if(FIXEDWIDE_BUILD_TESTS OR FIXEDWIDE_BUILD_ORACLE_TESTS)
    add_subdirectory(tests)
endif()

if(FIXEDWIDE_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(FIXEDWIDE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(FIXEDWIDE_BUILD_FUZZER)
    if(NOT FIXEDWIDE_SANITIZE)
        message(FATAL_ERROR "Fuzzing requires FIXEDWIDE_SANITIZE=ON")
    endif()
    add_subdirectory(fuzz)
endif()

install(TARGETS fixedwide
    EXPORT fixedwideTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT fixedwideTargets
    NAMESPACE fixedwide::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fixedwide
)

configure_package_config_file(
    cmake/fixedwideConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/fixedwideConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fixedwide
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/fixedwideConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/fixedwideConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/fixedwideConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fixedwide
)
