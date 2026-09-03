find_package(Boost REQUIRED)

add_executable(fixedwide_rounding_bench rounding_bench.cpp)
target_link_libraries(fixedwide_rounding_bench PRIVATE fixedwide::fixedwide)
target_include_directories(fixedwide_rounding_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(fixedwide_rounding_bench PRIVATE -fno-vectorize -fno-slp-vectorize -ffp-contract=off)
else()
    target_compile_options(fixedwide_rounding_bench PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize -ffp-contract=off)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/competitors/fpm/include AND EXISTS ${CMAKE_SOURCE_DIR}/competitors/cnl/include)
    add_executable(fixedwide_competitor_bench competitor_bench.cpp)
    target_link_libraries(fixedwide_competitor_bench PRIVATE fixedwide::fixedwide)
    target_include_directories(fixedwide_competitor_bench PRIVATE
        ${CMAKE_SOURCE_DIR}/competitors/fpm/include
        ${CMAKE_SOURCE_DIR}/competitors/cnl/include
        ${Boost_INCLUDE_DIRS}
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(fixedwide_competitor_bench PRIVATE -fno-vectorize -fno-slp-vectorize -ffp-contract=off)
    else()
        target_compile_options(fixedwide_competitor_bench PRIVATE -fno-tree-vectorize -fno-tree-slp-vectorize -ffp-contract=off)
    endif()
endif()
