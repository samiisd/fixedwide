# The instruction-count gate must configure on a bare CI runner, so it comes
# first and depends on nothing but the library itself. Everything below it
# differential-tests against Boost.Multiprecision and needs Boost headers.
add_executable(fixedwide_icount icount.cpp)
target_link_libraries(fixedwide_icount PRIVATE fixedwide::fixedwide)
# Same vectorisation discipline as the timed benchmarks: a vectorised fixture
# loop would count the compiler's unrolling, not the operation under test.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(fixedwide_icount PRIVATE -fno-vectorize -fno-slp-vectorize)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(fixedwide_icount PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize)
endif()

find_package(Boost)
if(NOT Boost_FOUND)
    message(STATUS "fixedwide: Boost headers not found; building fixedwide_icount only")
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
