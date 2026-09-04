cmake_minimum_required(VERSION 3.25)
project(fixedwide VERSION 0.5.0 LANGUAGES CXX)

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
    target_compile_options(fixedwide PRIVATE -Wall -Wextra -Wconversion -Wshadow)
endif()

set_target_properties(fixedwide PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    POSITION_INDEPENDENT_CODE ON
)

set(FIXEDWIDE_AUTO_FORCE_PORTABLE OFF)
if(NOT FIXEDWIDE_FORCE_PORTABLE)
    check_cxx_source_compiles("
        int main() {
        #if defined(__SIZEOF_INT128__)
            volatile __int128 a = static_cast<__int128>(123456789ULL) << 64;
            volatile __int128 b = static_cast<__int128>(97ULL);
            volatile __int128 q = a / b;
            volatile __int128 r = a % b;
            volatile unsigned __int128 ua = static_cast<unsigned __int128>(a);
            volatile unsigned __int128 ub = static_cast<unsigned __int128>(b);
            volatile unsigned __int128 uq = ua / ub;
            volatile unsigned __int128 ur = ua % ub;
            (void)q; (void)r; (void)uq; (void)ur;
        #endif
            return 0;
        }
    " FIXEDWIDE_INT128_DIVISION_LINKS)
    if(NOT FIXEDWIDE_INT128_DIVISION_LINKS)
        set(FIXEDWIDE_AUTO_FORCE_PORTABLE ON)
        message(STATUS "fixedwide: native __int128 division does not link; forcing portable backend")
    endif()
endif()

if(FIXEDWIDE_FORCE_PORTABLE OR FIXEDWIDE_AUTO_FORCE_PORTABLE)
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
