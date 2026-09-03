cmake_minimum_required(VERSION 3.25)
project(fixedwide VERSION 0.5.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

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

target_compile_options(fixedwide PRIVATE -Wall -Wextra -Wconversion -Wshadow)

set_target_properties(fixedwide PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    POSITION_INDEPENDENT_CODE ON
)

if(FIXEDWIDE_FORCE_PORTABLE)
    target_compile_definitions(fixedwide PUBLIC FIXEDWIDE_FORCE_PORTABLE)
endif()

if(FIXEDWIDE_SANITIZE)
    target_compile_options(fixedwide PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
    target_link_options(fixedwide PUBLIC -fsanitize=address,undefined)
endif()

if(FIXEDWIDE_COVERAGE)
    target_compile_options(fixedwide PUBLIC -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(fixedwide PUBLIC -fprofile-instr-generate)
endif()

if(FIXEDWIDE_BUILD_TESTS OR FIXEDWIDE_BUILD_ORACLE_TESTS)
    enable_testing()
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
