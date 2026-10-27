# The instruction-count gate must configure on a bare CI runner, so it comes
# first and depends on nothing but the library itself. External benchmark
# dependencies are optional and must not affect this target's configuration.
add_executable(fixedwide_icount icount.cpp)
target_link_libraries(fixedwide_icount PRIVATE fixedwide::fixedwide)
# Same vectorisation discipline as the timed benchmarks: a vectorised fixture
# loop would count the compiler's unrolling, not the operation under test.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(fixedwide_icount PRIVATE -fno-vectorize -fno-slp-vectorize)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(fixedwide_icount PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize)
endif()

# Focused microbenchmarks have no Boost dependency.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT MSVC)
    add_executable(fixedwide_checked_chain_bench checked_chain_bench.cpp)
    target_link_libraries(fixedwide_checked_chain_bench PRIVATE fixedwide::fixedwide)
    add_executable(fixedwide_log10_bench log10_bench.cpp)
    target_link_libraries(fixedwide_log10_bench PRIVATE fixedwide::fixedwide)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(fixedwide_checked_chain_bench PRIVATE -fno-vectorize -fno-slp-vectorize -ffp-contract=off)
        target_compile_options(fixedwide_log10_bench PRIVATE -fno-vectorize -fno-slp-vectorize -ffp-contract=off -fno-fast-math)
    else()
        target_compile_options(fixedwide_checked_chain_bench PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize -ffp-contract=off)
        target_compile_options(fixedwide_log10_bench PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize -ffp-contract=off -fno-fast-math)
    endif()
endif()

option(FIXEDWIDE_BUILD_RUNTIME_NARROW_BENCHMARK
    "Build the Google Benchmark runtime-narrow comparison" OFF)
if(FIXEDWIDE_BUILD_RUNTIME_NARROW_BENCHMARK)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" OR MSVC)
        message(FATAL_ERROR "The runtime-narrow benchmark currently requires GCC or Clang")
    endif()
    find_package(benchmark 1.9.4 CONFIG QUIET)
    if(NOT benchmark_FOUND)
        include(FetchContent)
        set(BENCHMARK_ENABLE_TESTING OFF)
        set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
        set(BENCHMARK_ENABLE_INSTALL OFF)
        set(BENCHMARK_ENABLE_WERROR OFF)
        # v1.9.4. An immutable commit also supports reproducible offline builds
        # through FETCHCONTENT_SOURCE_DIR_GOOGLEBENCHMARK.
        FetchContent_Declare(googlebenchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG eddb0241389718a23a42db6af5f0164b6e0139af)
        FetchContent_MakeAvailable(googlebenchmark)
        set(_fixedwide_google_benchmark_version "1.9.4 (eddb0241389718a23a42db6af5f0164b6e0139af)")
    else()
        set(_fixedwide_google_benchmark_version "${benchmark_VERSION}")
    endif()
    add_executable(fixedwide_runtime_narrow_bench runtime_narrow_bench.cpp)
    target_link_libraries(fixedwide_runtime_narrow_bench PRIVATE fixedwide::fixedwide benchmark::benchmark)
    target_compile_definitions(fixedwide_runtime_narrow_bench PRIVATE
        FIXEDWIDE_GOOGLE_BENCHMARK_VERSION="${_fixedwide_google_benchmark_version}")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(fixedwide_runtime_narrow_bench PRIVATE -fno-vectorize -fno-slp-vectorize -ffp-contract=off -fno-fast-math)
    else()
        target_compile_options(fixedwide_runtime_narrow_bench PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize -ffp-contract=off -fno-fast-math)
    endif()
endif()

find_package(Boost)
if(NOT Boost_FOUND)
    message(STATUS "fixedwide: Boost headers not found; skipping Boost-dependent benchmarks")
    return()
endif()

add_executable(fixedwide_bench benchmark.cpp baselines.cpp)
target_link_libraries(fixedwide_bench PRIVATE fixedwide::fixedwide)
target_include_directories(fixedwide_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(fixedwide_bench PRIVATE -fno-vectorize -fno-slp-vectorize -ffp-contract=off)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(fixedwide_bench PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize -ffp-contract=off)
endif()

add_executable(fixedwide_midpoint_bench midpoint_bench.cpp)
target_link_libraries(fixedwide_midpoint_bench PRIVATE fixedwide::fixedwide)
target_include_directories(fixedwide_midpoint_bench PRIVATE ${Boost_INCLUDE_DIRS})
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(fixedwide_midpoint_bench PRIVATE -fno-vectorize -fno-slp-vectorize)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(fixedwide_midpoint_bench PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize)
endif()

add_executable(fixedwide_numeric_bench numeric_bench.cpp)
target_link_libraries(fixedwide_numeric_bench PRIVATE fixedwide::fixedwide)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(fixedwide_numeric_bench PRIVATE -fno-vectorize -fno-slp-vectorize -ffp-contract=off)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(fixedwide_numeric_bench PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize -ffp-contract=off)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(fixedwide_memory_bench memory_bench.cpp)
    target_link_libraries(fixedwide_memory_bench PRIVATE fixedwide::fixedwide)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(fixedwide_memory_bench PRIVATE -fno-vectorize -fno-slp-vectorize)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(fixedwide_memory_bench PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize)
    endif()
endif()

add_executable(fixedwide_mixed_bench mixed_bench.cpp)
target_link_libraries(fixedwide_mixed_bench PRIVATE fixedwide::fixedwide)
target_include_directories(fixedwide_mixed_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(fixedwide_mixed_bench PRIVATE -fno-vectorize -fno-slp-vectorize)
else()
    target_compile_options(fixedwide_mixed_bench PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize)
endif()

add_executable(fixedwide_rounding_bench rounding_bench.cpp)
target_link_libraries(fixedwide_rounding_bench PRIVATE fixedwide::fixedwide)
target_include_directories(fixedwide_rounding_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(fixedwide_rounding_bench PRIVATE -fno-vectorize -fno-slp-vectorize -ffp-contract=off)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(fixedwide_rounding_bench PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize -ffp-contract=off)
endif()

option(FIXEDWIDE_BUILD_COMPETITOR_BENCH
    "Build the pinned decimal and fixed-point competitor benchmark" OFF)
if(FIXEDWIDE_BUILD_COMPETITOR_BENCH)
    include(${CMAKE_CURRENT_SOURCE_DIR}/competitors.cmake)
    add_executable(fixedwide_competitor_bench
        competitor/main.cpp
        competitor/common.cpp
        competitor/fixed.cpp
        competitor/decimal.cpp
        competitor/adjacent.cpp
    )
    target_link_libraries(fixedwide_competitor_bench PRIVATE
        fixedwide::fixedwide fixedwide_competitor_deps)
    target_include_directories(fixedwide_competitor_bench PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${Boost_INCLUDE_DIRS})
    # fpm's 64-bit representation uses __int128 as its intermediate and checks
    # it with std::is_signed. libstdc++ exposes that extension trait only in GNU
    # language mode. Keep fixedwide itself strict C++23; only this optional
    # third-party comparison target needs gnu++23.
    set_target_properties(fixedwide_competitor_bench PROPERTIES CXX_EXTENSIONS ON)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(fixedwide_competitor_bench PRIVATE
            -fno-vectorize -fno-slp-vectorize -ffp-contract=off)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(fixedwide_competitor_bench PRIVATE
            -fno-tree-vectorize -fno-tree-slp-vectorize -ffp-contract=off)
    endif()
endif()
