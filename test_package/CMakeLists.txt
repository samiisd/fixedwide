cmake_minimum_required(VERSION 3.25)
project(fixedwide_test_package LANGUAGES CXX)

find_package(fixedwide REQUIRED CONFIG)

add_executable(test_package test_package.cpp)
target_link_libraries(test_package PRIVATE fixedwide::fixedwide)
target_compile_features(test_package PRIVATE cxx_std_23)
